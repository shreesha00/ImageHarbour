set(RPC_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/transport.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/erpc_transport.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/rpc_factory.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/rpc_token.cc)

target_sources(imagesvr PRIVATE ${RPC_SRC})
target_sources(imageharbourcli PRIVATE ${RPC_SRC})