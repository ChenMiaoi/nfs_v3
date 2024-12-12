#ifndef RPC_V2_H
#define RPC_V2_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <net/if.h>
#include <sys/socket.h>

#include <auth.h>

#define DEFAULT_HASHES      4
#define NFS_RA_TIMEOUT      5
#define NFS_MIN_XFER_SIZE   NFSMAXDATA2
#define NFS_MAX_XFER_SIZE   ( 4 * 1024 * 1024 )
#define NFS_DEF_XFER_SIZE   ( 1 * 1024 * 1024 )
#define RPC_CONTEXT_MAGIC   0xc6e46435
#define RPC_PARAM_UNDEFINED -1

struct rpc_data {
  int32_t size;
  char*   data;
};

struct rpc_pdu {
  struct rpc_pdu* next;
  uint32_t        xid;
  struct rpc_data outdata;
};

struct rpc_context;
typedef void ( *rpc_cb )(
  struct rpc_context* rpc,
  int                 status,
  void*               data,
  void*               private_data );

struct rpc_queue {
  struct rpc_pdu *head, *tail;
};

enum input_state {
  READ_RM       = 0,
  READ_PAYLOAD  = 1,
  READ_FRAGMENT = 2,
  READ_IOVEC    = 3,
  READ_PADDING  = 4,
  READ_UNKNOWN  = 5,
};

struct rpc_fragment {
  struct rpc_fragment* next;
  uint32_t             size;
  char*                data;
};

struct rpc_stats {
  /*
   * RPC requests sent out.
   * Retransmitted requests are counted multiple times in this.
   */
  uint64_t num_req_sent;

  /*
   * RPC responses received.
   * (num_req_sent - num_resp_rcvd) could be one of the following:
   * - requests in flight, whose responses are awaited.
   * - requests timed out, whose responses were never received.
   *   If retransmits are enabled, we would have retransmited these.
   */
  uint64_t num_resp_rcvd;

  /*
   * RPC requests which didn't get a response for timeo period.
   * See mount option 'timeo'.
   * These indicate some issue with the server and/or connection.
   */
  uint64_t num_timedout;

  /*
   * RPC requests that timed out while sitting in outqueue.
   * Unlike num_timedout, these are requests which were not sent to
   * server. If this number is high it indicates a slow or unresponsive
   * server and/or slow connection. Application should slow down issuing
   * new RPC requests.
   */
  uint64_t num_timedout_in_outqueue;

  /*
   * RPC requests which didn't get a response even after retrans
   * retries. These are counted in num_timedout as well.
   * See mount option 'retrans'.
   */
  uint64_t num_major_timedout;

  /*
   * RPC requests retransmited due to reconnect or timeout.
   */
  uint64_t num_retransmitted;

  /*
   * Number of times we had to reconnect, for one of the following
   * reasons:
   * - Peer closed connection.
   * - Major timeout was observed.
   */
  uint64_t num_reconnects;
};

struct rpc_context {
  uint32_t magic;
  int      fd;
  int      socket_disabled;
  int      old_fd;
  int      is_connected;
  int      is_nonblocking;

  char* error_string;

  uint32_t program;
  uint32_t version;

  rpc_cb connect_cb;
  void*  connect_data;

  struct auth* auth;
  uint32_t     xid;

  struct rpc_queue        outqueue;
  struct sockaddr_storage udp_src;
  uint32_t                num_hashes;

  struct rpc_queue* waitpdu;
  uint32_t          waitpdu_len;
  uint32_t          max_waitpdu_len;

  uint32_t         inpos;
  uint32_t         inbuf_size;
  char*            inbuf;
  enum input_state state;
  uint32_t         rm_xid[ 2 ]; /* array holding the record marker and the next 4 bytes */
  uint32_t         pdu_size;    /* used in rpc_read_from_socket() */
  char*            buf;         /* used in rpc_read_from_socket() */
  struct rpc_pdu*  pdu;

  int                     is_udp;
  struct sockaddr_storage udp_dest;
  int                     is_broadcast;

  struct sockaddr_storage s;
  int                     auto_reconnect;
  int                     num_retries;
  char*                   server;
  struct rpc_fragment*    fragments;

  int      tcp_syncnt;
  int      uid;
  int      gid;
  int      debug;
  uint64_t last_timeout_scan;
  uint64_t last_successful_rpc_response;
  int      timeout;
  int      retrans;

  char ifname[ IFNAMSIZ ];
  int  poll_timeout;

  /* Is a server context ? */
  int                  is_server_context;
  struct rpc_endpoint* endpoints;

  /* Per-transport RPC stats */
  struct rpc_stats stats;
};

extern struct rpc_context* rpc_init_context( void );
extern bool                rpc_set_hash_size( struct rpc_context* rpc, int hashes );
extern uint64_t            rpc_current_time( void );
extern void                rpc_reset_queue( struct rpc_queue* q );
extern bool                rpc_set_username( struct rpc_context* rpc, const std::string& username );
extern void                rpc_set_tcp_syncnt( struct rpc_context* rpc, int v );
extern void                rpc_set_uid( struct rpc_context* rpc, int uid );
extern void                rpc_set_gid( struct rpc_context* rpc, int gid );
extern void                rpc_set_debug( struct rpc_context* rpc, int level );

#endif//! RPC_V2_H
