cmake_minimum_required(VERSION 3.10)

# Basic examples demonstrating fundamental inspector_context usage

set(EXAMPLES
    basic_struct
    arrays_struct
    nested_struct
    context_memory
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
