cmake_minimum_required(VERSION 3.12)

set(LIBS_TO_LINK hazard_pointers PARENT_SCOPE)

add_library(hazard_pointers STATIC hp.cpp)

target_include_directories(hazard_pointers PUBLIC .)
