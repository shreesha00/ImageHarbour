cmake_minimum_required(VERSION 3.14)

project(ImageHarbour)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

include(FetchContent)

if(NOT ERPC_DIR)
    set(ERPC_DIR ${CMAKE_SOURCE_DIR}/eRPC)
endif()
message("ERPC dir is ${ERPC_DIR}")

find_package(Threads REQUIRED)

include_directories(
    ${ERPC_DIR}/src
    ${ERPC_DIR}/third_party/asio/include)
    
if(NOT RDMA_DIR)
    set(RDMA_DIR ${CMAKE_SOURCE_DIR}/RDMA)
endif()
message("RDMA dir is ${RDMA_DIR}")

include_directories(
    ${RDMA_DIR}/src/
    ${RDMA_DIR}/)

add_subdirectory(HdrHistogram_c)
add_subdirectory(RDMA)
add_subdirectory(src)

