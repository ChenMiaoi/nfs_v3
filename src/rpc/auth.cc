#include <arpa/inet.h>
#include <auth.h>
#include <cstdint>
#include <cstring>
#include <rpc/rpc.h>
#include <sys/types.h>

struct auth* authunix_create( const char* host,
                              uint32_t    uid,
                              uint32_t    gid,
                              uint32_t    len,
                              uint32_t*   groups ) {
  struct auth* auth;
  int          size, idx;
  uint32_t*    buf;

  size = 4                                // current_time
         + 4                              // host len
         + ( ( strlen( host ) + 3 ) & ~3 )// host
         + 4                              // uid
         + 4                              // gid
         + 4                              // size of group
         + len * 4;                       // groups
  auth                    = new struct auth();
  auth->ah_cred.oa_flavor = AUTH_UNIX;
  auth->ah_cred.oa_length = size;
  auth->ah_cred.oa_base   = new char[ size ];

  buf          = static_cast< uint32_t* >( (void*) auth->ah_cred.oa_base );
  idx          = 0;
  buf[ idx++ ] = htonl( (uint32_t) rpc_current_time() );
  buf[ idx++ ] = htonl( strlen( host ) );
  memcpy( &buf[ 2 ], host, strlen( host ) );

  idx += ( strlen( host ) + 3 ) >> 2;
  buf[ idx++ ] = htonl( uid );
  buf[ idx++ ] = htonl( gid );
  buf[ idx++ ] = htonl( len );
  while ( len-- > 0 ) {
    buf[ idx++ ] = htonl( *groups++ );
  }

  auth->ah_verf.oa_flavor = AUTH_NONE;
  auth->ah_verf.oa_length = 0;
  auth->ah_verf.oa_base   = NULL;
  auth->ah_private        = NULL;

  return auth;
}

struct auth* authunix_create_default( void ) {
  return authunix_create(
    "nfs_v3", getuid(), getgid(), 0, nullptr );
}
