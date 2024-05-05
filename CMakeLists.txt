cmake_minimum_required(VERSION 3.18)
project(sbjson)

add_library(sbjson sbjson.c)

option(BUILD_UTILS "Enable building the sbjson_utils library." OFF)
if(BUILD_UTILS)
    add_library(sbjson_utils sbjson_utils.c)
endif()

option(BUILD_TESTS "Enable building the tests" ON)

# To run the tests simply run 'ctest' in the build directory.
if(BUILD_TESTS)
    enable_testing()

    # This is just an example program.
    add_executable(example example.c)
    target_link_libraries(example sbjson)

    add_subdirectory(tests)
endif()

install(TARGETS sbjson DESTINATION lib)
