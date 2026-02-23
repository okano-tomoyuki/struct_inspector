set(lib_name c_inspector)

add_library(${lib_name}
    inspector.c
    inspector_dsl.c
    inspector_context.c
)

target_include_directories(${lib_name}
    PRIVATE
        ${INSPECTOR_BUILD_BASE_DIR}/include
)