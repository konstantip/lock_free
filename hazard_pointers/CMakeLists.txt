cmake_minimum_required(VERSION 3.12)


macro(ADD_HP_LIB)
  get_filename_component(lib_name ${CMAKE_CURRENT_LIST_DIR} NAME)

  file(GLOB HP_SRC "*.cpp")
  add_library(${lib_name} STATIC ${HP_SRC})

  target_include_directories(${lib_name} PUBLIC .)
endmacro()


SUBDIRSLIST(HAZARD_SUBDIRS ${CMAKE_CURRENT_LIST_DIR})

set(LOCAL_HAZARD_POINTERS_LIBS_LIST ${HAZARD_POINTERS_LIBS_LIST})
foreach(subdir ${HAZARD_SUBDIRS})
  if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/${subdir}/CMakeLists.txt")
    message(STATUS "Add ${subdir} library.")
    add_subdirectory(${subdir})
    list(APPEND LOCAL_HAZARD_POINTERS_LIBS_LIST ${subdir})
  endif()
endforeach()

set(HAZARD_POINTERS_LIBS_LIST ${LOCAL_HAZARD_POINTERS_LIBS_LIST} PARENT_SCOPE)
