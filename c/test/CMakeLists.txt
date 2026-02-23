enable_testing()

# Create test executable for inspector core
add_executable(test_inspector
    test_inspector.c
)

target_include_directories(test_inspector
    PRIVATE
        ${INSPECTOR_BUILD_BASE_DIR}/include
)

target_link_libraries(test_inspector
    PRIVATE
        c_inspector
)

add_test(
    NAME InspectorCore
    COMMAND test_inspector
)

# Create test executable for context wrapper
add_executable(test_context
    test_context.c
)

target_include_directories(test_context
    PRIVATE
        ${INSPECTOR_BUILD_BASE_DIR}/include
)

target_link_libraries(test_context
    PRIVATE
        c_inspector
)

add_test(
    NAME ContextWrapper
    COMMAND test_context
)

# Create test executable for DSL
add_executable(test_dsl
    test_dsl.c
)

target_include_directories(test_dsl
    PRIVATE
        ${INSPECTOR_BUILD_BASE_DIR}/include
)

target_link_libraries(test_dsl
    PRIVATE
        c_inspector
)

add_test(
    NAME DSLMetadata
    COMMAND test_dsl
)
