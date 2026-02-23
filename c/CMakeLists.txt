set(INSPECTOR_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(src)

set(exec_name c_inspector_sample)

add_executable(${exec_name}
    main.c
)

target_include_directories(${exec_name}
    PRIVATE
        ${INSPECTOR_BASE_DIR}/include
)

target_link_libraries(${exec_name}
    PRIVATE
        c_inspector
)