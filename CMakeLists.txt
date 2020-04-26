cmake_minimum_required(VERSION 3.12)
project(lock_free)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-O3 -W -Wall -pedantic)

set(ONE_POP_ONE_PUSH tests/only_pop_thread_only_push_thread.cpp)
set(TMP_NAME "ONE_POP_ONE_PUSH")
set(${TMP_NAME}_LINK pthread)
list(APPEND TEST_LIST TMP_NAME) 

set(NAME_PREFIX HP)
set(HP_SUBDIR hp_stack)
add_subdirectory(${HP_SUBDIR})
foreach(TEST IN LISTS ${TEST_LIST})
  set(NAME ${NAME_PREFIX}_${TEST})

  message(STATUS "name is ${NAME}")

  message(STATUS "sources is ${${TEST}}")

  add_executable(${NAME} ${${TEST}})
  target_link_libraries(${NAME} PRIVATE hazard_pointers ${${TEST}_LINK})
endforeach()

set(NAME_PREFIX SIMPLE_STACK)
set(HP_SUBDIR simple_stack)
foreach(TEST IN LISTS ${TEST_LIST})
  set(NAME ${NAME_PREFIX}_${TEST})

  add_executable(${NAME} ${${TEST}})
  target_include_directories(${NAME} PRIVATE ${HP_SUBDIR})
  target_link_libraries(${NAME} PRIVATE ${${TEST}_LINK})
endforeach()
