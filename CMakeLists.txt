cmake_minimum_required(VERSION 3.12)
project(lock_free)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-O3 -W -Wall -pedantic)

set(TEST_NAME one_pop_one_push)
set(${TEST_NAME} tests/only_pop_thread_only_push_thread.cpp)
set(${TEST_NAME}_link pthread)
list(APPEND TEST_LIST ${TEST_NAME}) 

list(APPEND SUBDIR_TO_EXCLUDE .git build tests)

macro(SUBDIRLIST result curdir)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child} AND NOT child IN_LIST SUBDIR_TO_EXCLUDE)
      list(APPEND dirlist ${child})
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()

SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_LIST_DIR})
foreach(subdir ${SUBDIRS})
  message(STATUS "Subdirectory: ${subdir}")
  set(LIBS_TO_LINK "")
  if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/${subdir}/CMakeLists.txt")
    message(STATUS "Cmake exists")
    add_subdirectory(${subdir})
  endif()

  if(LIBS_TO_LINK)
    message(STATUS "Target ${subdir} will be linked with: ${LIBS_TO_LINK}")
  endif()

  foreach(T ${TEST_LIST})
    set(NAME ${subdir}_${T})

    add_executable(${NAME} ${${T}})
    target_link_libraries(${NAME} PRIVATE ${${T}_link} ${LIBS_TO_LINK})
    target_include_directories(${NAME} PRIVATE ${subdir})
  endforeach()
endforeach()

