cmake_minimum_required(VERSION 3.12)
project(lock_free)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-O3 -W -Wall -pedantic)

set(TEST_NAME one_pop_one_push)
set(${TEST_NAME} tests/only_pop_thread_only_push_thread.cpp)
set(${TEST_NAME}_link pthread)
list(APPEND TEST_LIST ${TEST_NAME}) 

set(NAME_PREFIX hp)
set(SUBDIR hp_stack)
add_subdirectory(${SUBDIR})
foreach(T ${TEST_LIST})
  set(NAME ${NAME_PREFIX}_${T})

  message(STATUS "name is ${NAME}")

  message(STATUS "sources is ${${T}}")

  add_executable(${NAME} ${${T}})
  target_link_libraries(${NAME} PRIVATE hazard_pointers ${${T}_link})
endforeach()

set(NAME_PREFIX simple_stack)
set(SUBDIR simple_stack)
foreach(T ${TEST_LIST})
  set(NAME ${NAME_PREFIX}_${T})

  add_executable(${NAME} ${${T}})
  target_include_directories(${NAME} PRIVATE ${SUBDIR})
  target_link_libraries(${NAME} PRIVATE ${${T}_link})
endforeach()

set(NAME_PREFIX shared_ptr_stack)
set(SUBDIR stack_on_shared_ptr)
foreach(T ${TEST_LIST})
  set(NAME ${NAME_PREFIX}_${T})

  add_executable(${NAME} ${${T}})
  target_include_directories(${NAME} PRIVATE ${SUBDIR})
  target_link_libraries(${NAME} PRIVATE ${${T}_link})
endforeach()

set(NAME_PREFIX counted_node_stack)
set(SUBDIR counted_node_stack)
foreach(T ${TEST_LIST})
  set(NAME ${NAME_PREFIX}_${T})

  add_executable(${NAME} ${${T}})
  target_include_directories(${NAME} PRIVATE ${SUBDIR})
  target_link_libraries(${NAME} PRIVATE ${${T}_link} atomic)
endforeach()
