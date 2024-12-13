#ifndef NFS_V3_H
#define NFS_V3_H

#include <rpc/rpc.h>
#include <rpcgen_mount.h>
#include <rpcgen_nfs_v3.h>

#include <memory>
#include <string>
#include <utility>

/**
 * @brief 
 *
 * @ref [rfc1813 page20](https://www.rfc-editor.org/rfc/rfc1813)
 * 
 * @code{.cc}
 * struct specdata3 {  
 *   uint32 specdata1;  
 *   uint32 specdata2;  
 * };  
 * @endcode
 */
struct nfs_specdata {
  uint32_t specdata1;
  uint32_t specdata2;
};

/**
 * @brief 
 * 
 * @ref [rfc1813 page21](https://www.rfc-editor.org/rfc/rfc1813)
 * 
 * @code{.cc}
 * struct nfstime3 {
 *   uint32   seconds;
 *   uint32   nseconds;
 * };
 * @endcode
 */
struct nfs_time {
  uint32_t seconds;
  uint32_t nseconds;
};

/**
 * @brief 
 *
 * @ref [rfc1813 page22](https://www.rfc-editor.org/rfc/rfc1813)
 * 
 */
struct nfs_attr {
  uint32_t            type;
  uint32_t            mode;
  uint32_t            uid;
  uint32_t            gid;
  uint32_t            nlink;
  uint64_t            size;
  uint64_t            used;
  uint64_t            fsid;
  struct nfs_specdata rdev;
  struct nfs_time     atime;
  struct nfs_time     mtime;
  struct nfs_time     ctime;
};

/**
 * @brief 
 * 
 * @ref [rfc1813 page21](https://www.rfc-editor.org/rfc/rfc1813)
 *
 * @code {.cc}
 * struct nfs_fh3 {
 *   opaque  data<NFS3_FHSIZE>;
 * };
 * @endcode
 */
struct nfs_fh {
  int   len;
  char* val;
};

struct nfsdir {
  struct nfs_fh   fh;
  struct nfs_attr attr;
  struct nfsdir*  next;

  struct nfsdirent* entries;
  struct nfsdirent* current;
};

struct nested_mounts {
  struct nested_mounts* next;
  char*                 path;
  struct nfs_fh         fh;
  struct nfs_attr       attr;
};

struct nfs_context_internal {
  char*         server;
  char*         ex_port;
  char*         cwd;
  struct nfs_fh rootfh;
  size_t        readmax;
  size_t        writemax;

  int auto_reconnect;
  int timeout;
  int retrans;

  int                   dircache_enabled;
  struct nfsdir*        dircache;
  uint16_t              mask;
  int                   auto_traverse_mounts;
  struct nested_mounts* nested_mounts;
  int                   default_version;

  int      version;
  int      nfsport;
  int      mountport;
  uint32_t readdir_dircount;
  uint32_t readdir_maxcount;
};

struct nfs_url {
  std::string server;
  std::string path;
  std::string file;
};

struct nfs_context {
  struct rpc_context*          rpc;
  struct nfs_context_internal* nfsi;
  char*                        error_string;
};

extern struct nfs_context* nfs_init_context( void );

extern struct nfs_url* nfs_parse_url_full( struct nfs_context* nfs,
                                           const std::string&  url );

extern struct nfs_url* nfs_parse_url_dir( struct nfs_context* nfs,
                                          const std::string&  url );

extern struct nfs_url*     nfs_parse_url_incomplete( struct nfs_context* nfs,
                                                     const std::string&  url );
extern struct rpc_context* nfs_get_rpc_context( struct nfs_context* nfs );
extern void                nfs_destroy_url( struct nfs_url* url );

extern void nfs_set_timeout( struct nfs_context* nfs, int timeout_msecs );
extern void nfs_set_retrans( struct nfs_context* nfs, int retrans );
extern void nfs_set_auto_traverse_mounts( struct nfs_context* nfs, int enabled );
extern void nfs_set_dircache( struct nfs_context* nfs, int enabled );
extern void nfs_set_autoreconnect( struct nfs_context* nfs, int num_retries );
extern int  nfs_set_version( struct nfs_context* nfs, int version );
extern void nfs_set_nfsport( struct nfs_context* nfs, int port );
extern void nfs_set_mountport( struct nfs_context* nfs, int port );
extern void nfs_set_readmax( struct nfs_context* nfs, size_t readmax );
extern void nfs_set_writemax( struct nfs_context* nfs, size_t writemax );
extern void nfs_set_readdir_max_buffer_size( struct nfs_context* nfs,
                                             uint32_t            dircount,
                                             uint32_t            maxcount );
#endif//! NFS_V3_H
