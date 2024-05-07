add_executable(tbbtest test_tbb_lru.cc)
add_executable(shatest sha256_test.cc)
find_package(TBB REQUIRED)

target_link_libraries(tbbtest TBB::tbb)
target_link_libraries(shatest OpenSSL::Crypto OpenSSL::SSL)
