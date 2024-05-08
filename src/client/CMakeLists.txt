target_sources(imageharbourcli
    PRIVATE image_harbour_cli.cc)

add_executable(basic_cli basic_client.cc)
add_executable(client_daemon client_daemon.cc)

target_link_libraries(basic_cli
    PRIVATE imageharbourcli
)

target_link_libraries(client_daemon
    PRIVATE imageharbourcli
)

target_link_libraries(imageharbourcli
    PRIVATE ibverbs
    PRIVATE infinity
    PRIVATE OpenSSL::SSL
    PRIVATE OpenSSL::Crypto
) 

target_include_directories(imageharbourcli PUBLIC ${CMAKE_SOURCE_DIR}/RDMA/release/include)

target_compile_definitions(imageharbourcli PRIVATE ERPC_INFINIBAND=true)
target_compile_definitions(basic_cli PRIVATE ERPC_INFINIBAND=true)
target_compile_definitions(client_daemon PRIVATE ERPC_INFINIBAND=true)
