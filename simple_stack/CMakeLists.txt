cmake_minimum_required(VERSION 3.12)
project(simple_lock_free_stack_sample)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-O3 -W -Wall -pedantic)

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE .)
