cmake_minimum_required(VERSION 3.10)

# Feature examples combining inspector_context APIs for practical use cases

set(EXAMPLES
    serialization_example
    string_query_example
    config_persistence_example
)

foreach(example ${EXAMPLES})
    add_executable(${example}
        ${example}.c
    )

    target_include_directories(${example}
        PRIVATE
            ${INSPECTOR_BUILD_BASE_DIR}/include
    )

    target_link_libraries(${example}
        PRIVATE
            c_inspector
    )

    # Set output name
    set_target_properties(${example} PROPERTIES
        OUTPUT_NAME ${example}
    )
endforeach()
