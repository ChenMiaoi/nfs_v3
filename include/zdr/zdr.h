#ifndef NFS_ZDR_H
#define NFS_ZDR_H

#include <cstdint>
#include <sys/types.h>

struct zdr_t {
};

typedef uint32_t ( *zdrproc_t )( zdr_t*, void*, ... );

#endif//! NFS_ZDR_H
