add_library(imageharbourcli STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/client/image_harbour_cli.cc)
target_link_libraries(imageharbourcli
    PRIVATE ${ERPC_DIR}/build/liberpc.a
    PRIVATE ibverbs
    PRIVATE numa
    PRIVATE infinity
    PUBLIC glog::glog
)
target_compile_definitions(imageharbourcli PRIVATE ERPC_INFINIBAND=true)
target_include_directories(imageharbourcli PRIVATE ${CMAKE_SOURCE_DIR}/RDMA/release/include)

add_subdirectory(client)
add_subdirectory(image_server)
add_subdirectory(rpc)
add_subdirectory(utils)

add_custom_target(gen_net_config ALL
    DEPENDS ${CMAKE_SOURCE_DIR}/cfg/rdma.prop)
add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/cfg/rdma.prop
    COMMAND python ${CMAKE_SOURCE_DIR}/gen_rdma_config.py ${CMAKE_SOURCE_DIR}/cfg/rdma.prop
    DEPENDS ${CMAKE_SOURCE_DIR}/gen_rdma_config.py)