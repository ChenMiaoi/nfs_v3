#include <cstdlib>
#include <cstring>
#include <memory>
#include <nfs/v3/nfs_v3.h>
#include <optional>
#include <rpc/rpc.h>
#include <string>

struct nfs_context* nfs_init_context( void ) {
  struct nfs_context*          nfs;
  struct nfs_context_internal* nfsi;

  nfs  = new nfs_context;
  nfsi = new nfs_context_internal;
  if ( !nfs || !nfsi ) {
    return nullptr;
  }

  nfs->nfsi = nfsi;
  nfs->rpc  = rpc_init_context();
  if ( !nfs->rpc ) {
    delete nfs->nfsi;
    delete nfs;
    return nullptr;
  }

  nfs->nfsi->cwd                  = strdup( "/" );
  nfs->nfsi->mask                 = 022;
  nfs->nfsi->auto_traverse_mounts = 1;
  nfs->nfsi->dircache_enabled     = 1;

  nfs->nfsi->auto_reconnect = -1;
  nfs->nfsi->timeout        = 60 * 1000;
  nfs->nfsi->retrans        = 2;

  nfs->nfsi->default_version  = NFS_V3;
  nfs->nfsi->version          = NFS_V3;
  nfs->nfsi->readmax          = NFS_DEF_XFER_SIZE;
  nfs->nfsi->writemax         = NFS_DEF_XFER_SIZE;
  nfs->nfsi->readdir_dircount = 8192;
  nfs->nfsi->readdir_maxcount = 8192;

  return nfs;
}

static int tohex( char ch ) {
  if ( ch >= '0' && ch <= '9' ) {
    return ch - '0';
  }
  ch &= 0xDF;
  if ( ch >= 'A' && ch <= 'F' ) {
    return ch - 'A' + 10;
  }
  return -1;
}

static int nfs_set_context_args( struct nfs_context* nfs,
                                 const char*         arg,
                                 const char*         val ) {
  if ( !strcmp( arg, "tcp-syncnt" ) ) {
    rpc_set_tcp_syncnt( nfs_get_rpc_context( nfs ), atoi( val ) );
  } else if ( !strcmp( arg, "uid" ) ) {
    rpc_set_uid( nfs_get_rpc_context( nfs ), atoi( val ) );
  } else if ( !strcmp( arg, "gid" ) ) {
    rpc_set_gid( nfs_get_rpc_context( nfs ), atoi( val ) );
  } else if ( !strcmp( arg, "timeo" ) ) {
    /* val is in deci-seconds */
    const int timeout_msecs = atoi( val ) * 100;
    if ( timeout_msecs < ( 10 * 1000 ) ) {
      // nfs_set_error( nfs, "timeo cannot be less than 100: %s", val );
      return -1;
    }
    nfs_set_timeout( nfs, timeout_msecs );
  } else if ( !strcmp( arg, "retrans" ) ) {
    const int retrans = atoi( val );
    if ( retrans < 0 ) {
      // nfs_set_error( nfs, "retrans cannot be less than 0: %s", val );
      return -1;
    }
    nfs_set_retrans( nfs, retrans );
  } else if ( !strcmp( arg, "debug" ) ) {
    rpc_set_debug( nfs_get_rpc_context( nfs ), atoi( val ) );
  } else if ( !strcmp( arg, "auto-traverse-mounts" ) ) {
    nfs_set_auto_traverse_mounts( nfs, atoi( val ) );
  } else if ( !strcmp( arg, "dircache" ) ) {
    nfs_set_dircache( nfs, atoi( val ) );
  } else if ( !strcmp( arg, "autoreconnect" ) ) {
    nfs_set_autoreconnect( nfs, atoi( val ) );
  } else if ( !strcmp( arg, "version" ) ) {
    if ( nfs_set_version( nfs, atoi( val ) ) < 0 ) {
      // nfs_set_error( nfs, "NFS version %d is not supported", atoi( val ) );
      return -1;
    }
  } else if ( !strcmp( arg, "nfsport" ) ) {
    nfs_set_nfsport( nfs, atoi( val ) );
  } else if ( !strcmp( arg, "mountport" ) ) {
    nfs_set_mountport( nfs, atoi( val ) );
  } else if ( !strcmp( arg, "rsize" ) ) {
    nfs_set_readmax( nfs, atoi( val ) );
  } else if ( !strcmp( arg, "wsize" ) ) {
    nfs_set_writemax( nfs, atoi( val ) );
  } else if ( !strcmp( arg, "readdir-buffer" ) ) {
    char* strp = (char*) strchr( val, ',' );
    if ( strp ) {
      *strp = 0;
      strp++;
      nfs_set_readdir_max_buffer_size( nfs, atoi( val ), atoi( strp ) );
    } else {
      nfs_set_readdir_max_buffer_size( nfs, atoi( val ), atoi( val ) );
    }
  } else {
    // nfs_set_error( nfs, "Unknown url argument : %s", arg );
    return -1;
  }
  return 0;
}

static std::optional< nfs_url* > nfs_parse_url( struct nfs_context* nfs,
                                                const std::string&  url,
                                                bool                dir,
                                                bool                incomplete ) {
  // "nfs://<username>@server_ip:<port>/<path>/file?x=x&y=y...."
  if ( url.compare( 0, 6, "nfs://" ) ) {
    return std::nullopt;
  }

  auto                   urls   = std::make_unique< nfs_url >();
  std::string::size_type at_pos = 0,// '@'
    slash_pos                   = 0,// '/'
    ques_pos                    = 0;// '?'

  if ( at_pos = url.find( '@' ); at_pos != std::string::npos ) {
    std::string user_name = url.substr( 6, at_pos );
    rpc_set_username( nfs_get_rpc_context( nfs ), user_name );
  }

  if ( slash_pos = url.find_first_of( '/' ); slash_pos != std::string::npos ) {
    std::string sp = url.substr(
      at_pos != std::string::npos ? at_pos : 6,
      slash_pos - at_pos );

    if ( auto colon_pos = sp.find( ':' ); colon_pos != std::string::npos ) {
      urls->server = sp.substr( 0, colon_pos );
      nfs_set_nfsport( nfs, std::stoi( sp.substr( colon_pos + 1 ) ) );
    } else {
      urls->server = sp;
    }
  } else {
    return std::nullopt;
  }

  std::string file_path;
  if ( ques_pos = url.find( '?' ); ques_pos != std::string::npos ) {
    std::string args = url.substr( ques_pos );
    file_path        = url.substr( slash_pos, ques_pos - slash_pos );
  } else {
    file_path = url.substr( slash_pos );
  }

  if ( !file_path.empty() ) {

  } else {
    return std::nullopt;
  }

  return urls.get();
}

struct nfs_url* nfs_parse_url_full( struct nfs_context* nfs,
                                    const std::string&  url ) {
  return nfs_parse_url( nfs, url, false, false ).value_or( nullptr );
}

extern struct nfs_url* nfs_parse_url_dir( struct nfs_context* nfs,
                                          const std::string&  url ) {
  return nfs_parse_url( nfs, url, true, false ).value_or( nullptr );
}

extern struct nfs_url* nfs_parse_url_incomplete( struct nfs_context* nfs,
                                                 const std::string&  url ) {
  return nfs_parse_url( nfs, url, false, true ).value_or( nullptr );
}
