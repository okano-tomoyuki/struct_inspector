cmake_minimum_required(VERSION 3.10)
project(c_inspector VERSION 0.1)

set(INSPECTOR_BUILD_BASE_DIR ${PROJECT_SOURCE_DIR})
add_subdirectory(src)

set(exec_name c_inspector_sample)

add_executable(${exec_name}
    main.c
)

target_include_directories(${exec_name}
    PRIVATE
        ${INSPECTOR_BUILD_BASE_DIR}/include
)

target_link_libraries(${exec_name}
    PRIVATE
        c_inspector
)

# Add test directory for CTest
add_subdirectory(test)

# Add examples
add_subdirectory(example)