add_executable(memorysvr memory_server.cc)

target_link_libraries(memorysvr
    PRIVATE ibverbs 
    PRIVATE numa
    PRIVATE infinity
    PRIVATE glog::glog
)

target_include_directories(memorysvr PUBLIC ${CMAKE_SOURCE_DIR}/RDMA/release/include)