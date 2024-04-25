cmake_minimum_required(VERSION 3.18)
project(SbJSON)

add_library(sbJSON sbJSON.c)

option(ENABLE_SBJSON_UTILS "Enable building the sbJSON_Utils library." OFF)
if(ENABLE_SBJSON_UTILS)
    add_library(sbjson_utils sbJSON_Utils.c)
endif()

option(ENABLE_SBJSON_TEST "Enable building sbJSON test" ON)
if(ENABLE_SBJSON_TEST)
    add_executable(sbJSON_test test.c)
    target_link_libraries(sbJSON_test sbJSON)
endif()

install(TARGETS sbJSON DESTINATION lib)
