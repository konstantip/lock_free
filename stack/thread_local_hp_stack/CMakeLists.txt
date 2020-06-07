cmake_minimum_required(VERSION 3.12)

set(lib_name thread_local_hazard_pointers)

set(LIBS_TO_LINK ${lib_name} PARENT_SCOPE)

add_library(${lib_name} STATIC hp.cpp)

target_include_directories(${lib_name} PUBLIC .)
