add_executable(memorysvr memory_server.cc)

target_link_libraries(memorysvr
    ibverbs 
    numa
    infinity
    glog::glog
)

target_include_directories(memorysvr PUBLIC ${CMAKE_SOURCE_DIR}/RDMA/release/include)