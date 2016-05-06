cmake_minimum_required(VERSION 3.0)
project(gpu)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB GPU_SRCS
        "${PROJECT_SOURCE_DIR}/*.cpp"
        )

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_INCLUDE_DIR}")
include_directories("/usr/local/cuda-7.5/targets/x86_64-linux/include")
add_library(gpu STATIC ${GPU_SRCS})

TARGET_INCLUDE_DIRECTORIES(gpu PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        )
TARGET_LINK_LIBRARIES(gpu
        LINK_PUBLIC OpenCL
        )

#target_compile_features(gpu PRIVATE cxx_range_for)
