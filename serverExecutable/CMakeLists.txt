cmake_minimum_required(VERSION 3.0)
project(serverExecutable)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB SERVER_EXECUTABLE_SRCS
        "${PROJECT_SOURCE_DIR}/*.cpp"
        )

add_executable(serverExecutable ${SERVER_EXECUTABLE_SRCS})

#TARGET_INCLUDE_DIRECTORIES(server PUBLIC
#        ${CMAKE_CURRENT_SOURCE_DIR}/include
#        )

target_compile_definitions(serverExecutable PRIVATE SPDLOG_TRACE_ON SPDLOG_DEBUG_ON)
#target_compile_features(server PRIVATE cxx_range_for)
#target_compile_features(serverTest PRIVATE cxx_range_for)
target_link_libraries(serverExecutable
        gpu
        server
        ldns
        pthread
        )
