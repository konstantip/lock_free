cmake_minimum_required(VERSION 3.12)

add_library(hazard_pointers STATIC hp.cpp)

target_include_directories(hazard_pointers PUBLIC .)
