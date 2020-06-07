cmake_minimum_required(VERSION 3.12)
project(lock_free)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-O3 -W -Wall -pedantic)

enable_testing()


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


set(TEST_NAME one_pop_one_push)
set(${TEST_NAME} tests/only_pop_thread_only_push_thread.cpp)
set(${TEST_NAME}_link pthread)
list(APPEND TEST_LIST ${TEST_NAME})

set(TEST_NAME is_lock_free_on_current_platform)
set(${TEST_NAME} tests/is_lock_free_on_current_platform.cpp)
set(${TEST_NAME}_skip_test 1)
list(APPEND TEST_LIST ${TEST_NAME})


SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_LIST_DIR})
foreach(subdir ${SUBDIRS})
  message(STATUS "Subdirectory: ${subdir}")
  set(LIBS_TO_LINK "")

  set(SKIP_TESTS 0)
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

    if (NOT DEFINED ${T}_skip_test AND NOT ${SKIP_TESTS})
      add_test(NAME ${NAME} COMMAND ./${NAME})
    endif()
  endforeach()
endforeach()

