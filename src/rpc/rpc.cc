#include <cstdint>
#include <memory>
#include <rpc.h>
#include <rpc/auth.h>
#include <string>

extern struct rpc_context* rpc_init_context( void ) {
  struct rpc_context* rpc;
  static uint32_t     salt = 0;

  rpc = new rpc_context();
  if ( !rpc ) {
    return nullptr;
  }

  if ( !rpc_set_hash_size( rpc, DEFAULT_HASHES ) ) {
    return nullptr;
  }

  rpc->magic = RPC_CONTEXT_MAGIC;
  rpc->inpos = 0;
  rpc->state = READ_RM;
  rpc->auth  = authunix_create_default();
  if ( rpc->auth == nullptr ) {
    delete rpc->waitpdu;
    delete rpc;
    return nullptr;
  }

  rpc->xid =
    salt + (uint32_t) rpc_current_time() + ( (uint32_t) getpid() << 16 );
  salt            = salt + 0x01000000;
  rpc->fd         = -1;
  rpc->tcp_syncnt = RPC_PARAM_UNDEFINED;
  rpc->uid        = getuid();
  rpc->gid        = getgid();

  rpc_reset_queue( &rpc->outqueue );
  rpc->max_waitpdu_len = 0;
  rpc->timeout         = 60 * 1000;
  rpc->retrans         = 0;
  rpc->poll_timeout    = 100;

  return rpc;
}

bool rpc_set_hash_size( struct rpc_context* rpc, int hashes ) {
  rpc->num_hashes = hashes;

  delete rpc->waitpdu;
  rpc->waitpdu = new rpc_queue[ rpc->num_hashes ]();
  if ( !rpc->waitpdu ) {
    return false;
  }

  for ( uint32_t i = 0; i < rpc->num_hashes; i++ ) {
    rpc_reset_queue( &rpc->waitpdu[ i ] );
  }

  return true;
}

uint64_t rpc_current_time( void ) {
  struct timespec tp;
  clock_gettime( CLOCK_MONOTONIC_COARSE, &tp );
  return (uint64_t) tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
}

void rpc_reset_queue( struct rpc_queue* q ) {
  q->head = q->tail = nullptr;
}

bool rpc_set_username( struct rpc_context* rpc, const std::string& username ) {
  //? HAS LIBKRB5
  return true;
}
