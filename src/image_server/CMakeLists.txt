add_executable(imagesvr
    image_server.cc
    image_svr.cc)

target_link_libraries(imagesvr
    ${ERPC_DIR}/build/liberpc.a
    ibverbs
    numa
    glog::glog)

target_compile_definitions(imagesvr PRIVATE ERPC_INFINIBAND=true)