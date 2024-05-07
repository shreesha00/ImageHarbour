add_executable(imagesvr
    image_server.cc
    image_svr.cc)

target_include_directories(imagesvr PRIVATE ${parallel_hashmap_SOURCE_DIR})

target_link_libraries(imagesvr
    PRIVATE ${ERPC_DIR}/build/liberpc.a
    PRIVATE ibverbs
    PRIVATE numa
    PRIVATE infinity
    PRIVATE glog::glog
    PRIVATE OpenSSL::SSL
    PRIVATE OpenSSL::Crypto
)
    
target_compile_definitions(imagesvr PRIVATE ERPC_INFINIBAND=true)
target_include_directories(imagesvr PUBLIC ${CMAKE_SOURCE_DIR}/RDMA/release/include)