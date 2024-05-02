cmake_minimum_required(VERSION 3.14)

project(ImageHarbour)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

include(FetchContent)

FetchContent_Declare(
    glog
    GIT_REPOSITORY https://github.com/google/glog.git
    GIT_TAG v0.6.0
)
FetchContent_MakeAvailable(glog)

FetchContent_Declare(
    parallel_hashmap 
    GIT_REPOSITORY https://github.com/greg7mdp/parallel-hashmap.git 
    GIT_TAG master
) 
FetchContent_MakeAvailable(parallel_hashmap)

if(NOT ERPC_DIR)
    set(ERPC_DIR ${CMAKE_SOURCE_DIR}/../eRPC)
endif()
message("ERPC dir is ${ERPC_DIR}")

find_package(Threads REQUIRED)

include_directories(
    ${ERPC_DIR}/src
    ${ERPC_DIR}/third_party/asio/include)

add_subdirectory(HdrHistogram_c)
add_subdirectory(RDMA)
add_subdirectory(src)

