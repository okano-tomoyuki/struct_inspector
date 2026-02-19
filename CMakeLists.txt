cmake_minimum_required(VERSION 3.10)
project(struct_inspector VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

add_executable(${PROJECT_NAME}
    main.cpp
    struct_inspector.cpp
)
