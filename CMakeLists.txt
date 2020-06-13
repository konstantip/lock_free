cmake_minimum_required(VERSION 3.12)
project(lock_free)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-O3 -W -Wall -pedantic)

enable_testing()

include(utils.cmake)

set(HAZARD_POINTERS HAZARD_POINTERS)
set(HAZARD_POINTERS_LIBS_LIST "")
add_subdirectory(hazard_pointers)
message(STATUS "Hazard pointers libs: ${HAZARD_POINTERS_LIBS_LIST}")


list(APPEND SUBDIR_TO_EXCLUDE .git build tests hazard_pointers)


set(TESTS_DIR ${CMAKE_CURRENT_LIST_DIR}/tests)


set(TEST_NAME one_pop_one_push)
set(${TEST_NAME} ${TESTS_DIR}/only_pop_thread_only_push_thread.cpp)
set(${TEST_NAME}_link pthread)
list(APPEND TEST_LIST ${TEST_NAME})

set(TEST_NAME is_lock_free_on_current_platform)
set(${TEST_NAME} ${TESTS_DIR}/is_lock_free_on_current_platform.cpp)
set(${TEST_NAME}_skip_test 1)
set(${TEST_NAME}_handler HANDLE_IS_LOCK_FREE)
list(APPEND TEST_LIST ${TEST_NAME})


set(IS_LOCK_FREE_LIST "")

macro(HANDLE_IS_LOCK_FREE target_name)
  add_custom_target(check_${target_name} COMMAND ${target_name} --verbose >> ${CMAKE_BINARY_DIR}/is_log_free.log)
  list(APPEND LOCAL_LOCKFREE_CHECKERS_LIST check_${target_name})
endmacro()


macro(CONFIGURATIONS_LIST result use_hp)
  if(${use_hp})
    set(${result} ${HAZARD_POINTERS_LIBS_LIST})
  else()
    set(${result} " ")
  endif()
endmacro()


set(TEST_EXECUTABLES_DEPS_LIST "")

macro(CREATE_TESTS_TO_SUBDIRS subdir_with_tests)
  message(STATUS "Start to create tests for ${subdir_with_tests}.")

  set(LOCAL_TEST_EXECUTABLES_DEPS_LIST ${TEST_EXECUTABLES_DEPS_LIST})
  set(LOCAL_LOCKFREE_CHECKERS_LIST ${IS_LOCK_FREE_LIST})

  SUBDIRSLIST(SUBDIRS ${subdir_with_tests})

  message(STATUS "Subdir list: ${SUBDIRS}.")
  foreach(subdir ${SUBDIRS})
    message(STATUS "Subdirectory: ${subdir}")
    set(LIBS_TO_LINK "")

    set(SKIP_TESTS 0)
    if(EXISTS "${subdir_with_tests}/${subdir}/CMakeLists.txt")
      message(STATUS "Cmake exists")
      add_subdirectory(${subdir})
    endif()

    set(USE_HP 0)
    if(LIBS_TO_LINK)
      list(FIND LIBS_TO_LINK ${HAZARD_POINTERS} HP_INDEX)

      if(NOT HP_INDEX EQUAL -1)
        set(USE_HP 1)
        list(REMOVE_AT LIBS_TO_LINK ${HP_INDEX})
      endif()
    endif()

    CONFIGURATIONS_LIST(conf_list ${USE_HP})
    list(GET conf_list 0 first_element)
    if (${first_element} STREQUAL " ")
      message(STATUS "Only one configuration enabled")
    else()
      message(STATUS "Conf list: ${conf_list}")
    endif()
    foreach(CONF_LIBS ${conf_list})
      foreach(T ${TEST_LIST})

        if(${CONF_LIBS} STREQUAL " ")
          set(NAME ${subdir}_${T})
        else()
          message(STATUS "Conf libs are: ${CONF_LIBS}")
          prefix_from_list(pre ${CONF_LIBS})
          set(NAME ${subdir}${pre}${T})
        endif()

        message(STATUS "Name is: ${NAME}")

        add_executable(${NAME} ${${T}})
	target_link_libraries(${NAME} PRIVATE ${${T}_link} ${LIBS_TO_LINK})
        if (NOT ${CONF_LIBS} STREQUAL " ")
          message(STATUS "Link ${CONF_LIBS} to ${NAME}")
          target_link_libraries(${NAME} PRIVATE ${CONF_LIBS})
        endif()
        target_include_directories(${NAME} PRIVATE ${subdir})

        if(NOT DEFINED ${T}_skip_test AND NOT ${SKIP_TESTS})
          add_test(NAME test_${NAME} COMMAND ./${NAME})
          list(APPEND LOCAL_TEST_EXECUTABLES_DEPS_LIST ${NAME})
        endif()

        if(DEFINED ${T}_handler)
          invoke(${T}_handler ${NAME})
        endif()
      endforeach()
    endforeach()

  endforeach()


  set(IS_LOCK_FREE_LIST ${LOCAL_LOCKFREE_CHECKERS_LIST} PARENT_SCOPE)
  set(TEST_EXECUTABLES_DEPS_LIST ${LOCAL_TEST_EXECUTABLES_DEPS_LIST} PARENT_SCOPE)
endmacro()

macro(CREATE_TESTS_TO_CURRENT_SUBDIRS)
  CREATE_TESTS_TO_SUBDIRS(${CMAKE_CURRENT_LIST_DIR})
endmacro()

SUBDIRSLIST(SUBDIRS ${CMAKE_CURRENT_LIST_DIR})
foreach(subdir ${SUBDIRS})
  if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/${subdir}/CMakeLists.txt")
    message(STATUS "Cmake exists in ${subdir}. It is being processed.")
    add_subdirectory(${subdir})
  endif()
endforeach()

add_custom_target(run_tests COMMAND ${CMAKE_CTEST_COMMAND})
add_dependencies(run_tests ${TEST_EXECUTABLES_DEPS_LIST})


add_custom_target(run_lockfree_checkers)
add_dependencies(run_lockfree_checkers ${IS_LOCK_FREE_LIST})
