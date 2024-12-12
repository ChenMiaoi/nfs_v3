#ifndef RPC_V2_AUTH_H
#define RPC_V2_AUTH_H

#include <cstdint>
#include <sys/types.h>
#include <unistd.h>

#define AUTH_NONE 0
#define AUTH_NULL 0
#define AUTH_SYS  1
#define AUTH_UNIX 1
#define AUTH_GSS  6

struct opaque_cred {
  uint32_t oa_flavor;
  caddr_t  oa_base;
  uint32_t oa_length;
};

struct opaque_verf {
  uint32_t oa_flavor;
  caddr_t  oa_base;
  uint32_t oa_length;

  /* GSS */
  // struct gss_ctx_id_struct* gss_context;
};

struct auth {
  struct opaque_cred ah_cred;
  struct opaque_verf ah_verf;
  caddr_t            ah_private;
};

struct auth* authunix_create( const char* host,
                              uint32_t    uid,
                              uint32_t    gid,
                              uint32_t    len,
                              uint32_t*   groups );
struct auth* authunix_create_default( void );

#endif//! RPC_V2_AUTH_H
