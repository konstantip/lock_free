cmake_minimum_required(VERSION 3.12)
project(hazard_pointers_lock_free_stack_sample)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-O3 -W -Wall -pedantic)

add_library(hazard_pointers STATIC hp.cpp)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE hazard_pointers)

target_include_directories(hazard_pointers PUBLIC .)
