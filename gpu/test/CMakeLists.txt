cmake_minimum_required(VERSION 3.0)
project(gpuTest)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories("${GTEST_INCLUDE_DIR}")
include_directories("${GLOBAL_LIB_DIR}")
include_directories("${PROJECT_INCLUDE_DIR}")

file(GLOB GPUTEST_SRCS
        "${PROJECT_SOURCE_DIR}/*.cpp"
        )


add_executable(gpuTest ${GPUTEST_SRCS})

TARGET_LINK_LIBRARIES(gpuTest
        gpu
        gtest
        ldns
        )

add_test(
        NAME gpuTest
        COMMAND gpuTest "${CMAKE_CURRENT_LIST_DIR}/../main/kernels"
)

#target_compile_features(gpuTest PRIVATE cxx_range_for)