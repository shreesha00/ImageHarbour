target_sources(imageharbourcli
    PRIVATE image_harbour_cli.cc)

add_executable(basic_cli basic_client.cc)

target_link_libraries(basic_cli
    imageharbourcli
    ibverbs)

target_compile_definitions(imageharbourcli PRIVATE ERPC_INFINIBAND=true)
target_compile_definitions(basic_cli PRIVATE ERPC_INFINIBAND=true)
