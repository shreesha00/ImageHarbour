add_executable(imagesvr
    image_server.cc
    image_svr.cc)

target_include_directories(imagesvr PRIVATE ${parallel_hashmap_SOURCE_DIR})

target_link_libraries(imagesvr
    ${ERPC_DIR}/build/liberpc.a
    ibverbs
    numa
    infinity
    glog::glog
)
    
target_compile_definitions(imagesvr PRIVATE ERPC_INFINIBAND=true)
target_include_directories(imagesvr PUBLIC ${CMAKE_SOURCE_DIR}/RDMA/release/include)