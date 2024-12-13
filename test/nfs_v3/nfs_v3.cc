#include <gtest/gtest.h>

#include <memory>
#include <nfs/v3/nfs_v3.h>

TEST( nfs_v3_url, parse_full ) {
  std::unique_ptr< nfs_context > nfs = std::make_unique< nfs_context >();
  nfs->nfsi                          = new nfs_context_internal;

  auto url = nfs_parse_url_full(
    nfs.get(),
    "nfs://localhost/test/test_path/t.txt" );
  ASSERT_NE( url, nullptr );
  EXPECT_EQ( url->server, "localhost" );
  EXPECT_EQ( nfs->nfsi->nfsport, 7890 );
  EXPECT_EQ( url->path, "/test/test_path" );
  EXPECT_EQ( url->file, "/t.txt" );

  auto url1 = nfs_parse_url_full(
    nfs.get(),
    "nfs://test@localhost/test/test_path/t.txt" );
  ASSERT_NE( url1, nullptr );
  EXPECT_EQ( url1->server, "localhost" );
  EXPECT_EQ( nfs->nfsi->nfsport, 7890 );
  EXPECT_EQ( url1->path, "/test/test_path" );
  EXPECT_EQ( url1->file, "/t.txt" );

  auto url2 = nfs_parse_url_full(
    nfs.get(),
    "nfs://test@localhost:7890/test/test_path/t.txt" );
  ASSERT_NE( url2, nullptr );
  EXPECT_EQ( url2->server, "localhost" );
  EXPECT_EQ( nfs->nfsi->nfsport, 7890 );
  EXPECT_EQ( url2->path, "/test/test_path" );
  EXPECT_EQ( url2->file, "/t.txt" );

  auto url3 = nfs_parse_url_full(
    nfs.get(),
    "nfs://test@localhost:7890/test/test_path/t.txt?x=1" );
  ASSERT_NE( url3, nullptr );
  EXPECT_EQ( url3->server, "localhost" );
  EXPECT_EQ( nfs->nfsi->nfsport, 7890 );
  EXPECT_EQ( url3->path, "/test/test_path" );
  EXPECT_EQ( url3->file, "/t.txt" );

  auto url4 = nfs_parse_url_full(
    nfs.get(),
    "nfs://test@localhost:7890/test/test_path/t.txt?x=1&y=2" );
  ASSERT_NE( url4, nullptr );
  EXPECT_EQ( url4->server, "localhost" );
  EXPECT_EQ( nfs->nfsi->nfsport, 7890 );
  EXPECT_EQ( url4->path, "/test/test_path" );
  EXPECT_EQ( url4->file, "/t.txt" );
}

int main( int argc, char* argv[] ) {
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}
