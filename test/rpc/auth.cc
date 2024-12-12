#include <cstdint>
#include <gtest/gtest.h>

#include <rpc/auth.h>
#include <unistd.h>

TEST( rpc_auth, default_create ) {
  const char* host   = "auth_test";
  uint32_t    uid    = getuid();
  uint32_t    gid    = getgid();
  uint32_t    len    = 0;
  uint32_t*   groups = nullptr;

  auto a = authunix_create( host, uid, gid, len, groups );
  ASSERT_NE( a, nullptr );
}

int main( int argc, char* argv[] ) {
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}
