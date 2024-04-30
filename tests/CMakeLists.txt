add_library(unity STATIC 3rdparty/unity.c)
target_compile_definitions(unity PUBLIC -DUNITY_INCLUDE_DOUBLE)

#copy test files
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/inputs")
file(GLOB test_files "inputs/*")
file(COPY ${test_files} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/inputs/")

set(unity_tests
    parse_examples
    parse_number
    parse_hex4
    parse_string
    parse_array
    parse_double
    parse_object
    parse_value
    print_string
    print_number
    print_array
    print_object
    print_value
    misc_tests
    parse_with_opts
    compare_tests
    sbjson_add
    readme_examples
    minify_tests
)

foreach(unity_test ${unity_tests})
    add_executable("${unity_test}" "${unity_test}.c")
    target_link_libraries("${unity_test}" unity)
    add_test(NAME "${unity_test}"
        COMMAND "./${unity_test}")
endforeach()

if (BUILD_UTILS)
    #copy test files
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/json-patch-tests")
    file(GLOB test_files "json-patch-tests/*")
    file(COPY ${test_files} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/json-patch-tests/")

    set(utils_tests
        json_patch_tests
        old_utils_tests
        misc_utils_tests
    )

    foreach(utils_test ${utils_tests})
        add_executable("${utils_test}" "${utils_test}.c")
        target_link_libraries("${utils_test}" sbjson_utils unity)
        add_test(NAME "${utils_test}"
            COMMAND "./${utils_test}")
    endforeach()
endif()
