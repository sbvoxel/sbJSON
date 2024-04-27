add_library(unity STATIC 3rdparty/unity.c)

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
    cjson_add
    readme_examples
    minify_tests
)

foreach(unity_test ${unity_tests})
    add_executable("${unity_test}" "${unity_test}.c")
    target_link_libraries("${unity_test}" "${CJSON_LIB}" unity)
    add_test(NAME "${unity_test}"
        COMMAND "./${unity_test}")
endforeach()
