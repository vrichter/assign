#**********************************************************************
#* The MIT License (MIT)                                              *
#*                                                                    *
#* Copyright (c) 2015 Viktor Richter                                  *
#*                                                                    *
#* Permission is hereby granted, free of charge, to any person        *
#* obtaining a copy of this software and associated documentation     *
#* files (the "Software"), to deal in the Software without            *
#* restriction, including without limitation the rights               *
#* to use, copy, modify, merge, publish, distribute, sublicense,      *
#* and/or sell copies of the Software, and to permit persons to whom  *
#* the Software is furnished to do so, subject to the following       *
#* conditions:                                                        *
#*                                                                    *
#* The above copyright notice and this permission notice shall be     *
#* included in all copies or substantial portions of the Software.    *
#*                                                                    *
#* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,    *
#* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF *
#* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND              *
#* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT        *
#* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,       *
#* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, *
#* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER      *
#* DEALINGS IN THE SOFTWARE.                                          *
#*********************************************************************/

cmake_minimum_required(VERSION 2.8.2)

# this sets the projects name
project(assign)
set(PROJECT_BRIEF "Assignment of entities to a set of groups using preference costs.")
# the projects version number
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSIOM_PATCH 0)
set(PROJECT_NUMBER "v${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSIOM_PATCH}")

# some output
message(STATUS "")
message(STATUS "==============================================================")
message(STATUS "Configuring ${PROJECT_NAME}:")

# add c++11 compiler flag
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# checking build type, setting to release when not set
IF(NOT CMAKE_BUILD_TYPE)
  MESSAGE(STATUS "No Specific build type specified: using Release")
  SET(CMAKE_BUILD_TYPE "Release")
ENDIF()

add_subdirectory(munkres-cpp)

include_directories(${PROJECT_SOURCE_DIR}/munkres-cpp/src/)


add_executable("assign"
  "${PROJECT_SOURCE_DIR}/assign.cpp"
  "${PROJECT_SOURCE_DIR}/common.cpp"
  )

target_link_libraries("assign"
  munkres)

install(TARGETS "assign"
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE
 )

add_executable("assign_multiple"
  "${PROJECT_SOURCE_DIR}/assign_multiple.cpp"
  "${PROJECT_SOURCE_DIR}/common.cpp"
  )

target_link_libraries("assign_multiple"
  munkres)

install(TARGETS "assign_multiple"
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE
 )
