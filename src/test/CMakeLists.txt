add_executable(tbbtest test_tbb_lru.cc)
find_package(TBB REQUIRED)

target_link_libraries(tbbtest TBB::tbb)
