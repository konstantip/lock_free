macro(SUBDIRSLIST result curdir)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child} AND NOT child IN_LIST SUBDIR_TO_EXCLUDE)
      list(APPEND dirlist ${child})
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()

macro(invoke func_name params)
  set(temp_file_name "__${func_name}_temp_invoking__")

  file(WRITE ${temp_file_name} "${${func_name}}(${params})")

  include(${temp_file_name})

  file(REMOVE ${temp_file_name})
endmacro()
