cmake_minimum_required(VERSION 3.0)

project(score_addons LANGUAGES CXX)

macro(SUBDIRLIST result curdir)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
        list(APPEND dirlist ${child})
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()


subdirlist(SUBDIRS "${CMAKE_CURRENT_SOURCE_DIR}")
set(SCORE_ADDON True)
foreach(subdir ${SUBDIRS})
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/CMakeLists.txt")
    add_subdirectory("${subdir}")
  endif()
endforeach()
