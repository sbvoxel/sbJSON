cmake_minimum_required(VERSION 3.18)
project(SbJSON)

add_library(sbJSON sbJSON.c)

option(BUILD_UTILS "Enable building the sbJSON_Utils library." OFF)
if(BUILD_UTILS)
    add_library(sbjson_utils sbJSON_Utils.c)
endif()

option(BUILD_TESTS "Enable building the tests" ON)
if(BUILD_TESTS)
    enable_testing()

    # This is not the testing executable; it's just an example program.
    # To run the tests simply run 'ctest' in the build directory.
    add_executable(example example.c)
    target_link_libraries(example sbJSON)

    add_subdirectory(tests)
endif()

install(TARGETS sbJSON DESTINATION lib)
