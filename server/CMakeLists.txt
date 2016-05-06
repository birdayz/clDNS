cmake_minimum_required(VERSION 3.0)
project(server)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories("${GTEST_INCLUDE_DIR}")

file(GLOB SERVER_SRCS
        "${PROJECT_SOURCE_DIR}/*.cpp"
        )

file(GLOB SERVER_TEST_SRCS
        "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp"
        )

add_library(server STATIC ${SERVER_SRCS})

add_executable(serverTest ${SERVER_TEST_SRCS})



TARGET_INCLUDE_DIRECTORIES(server PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

add_test(
        NAME serverTest
        COMMAND serverTest
)

#target_compile_features(server PRIVATE cxx_range_for)
#target_compile_features(serverTest PRIVATE cxx_range_for)
target_link_libraries(serverTest
        gpu
        gtest_main
        server
        ldns
        pthread
        )

target_link_libraries(server
        gpu
        ldns
        pthread
        )