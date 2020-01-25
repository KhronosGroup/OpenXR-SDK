# Copyright (c) 2017-2020 The Khronos Group Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Author:
#

# Note: This is the top-level CMake file for the OpenXR project.
# It should contain only definitions that are applicable to the
# entire project and includes for the sub-directories.

cmake_minimum_required(VERSION 3.0)
project(OPENXR)

find_package(PythonInterp 3)

# Enable IDE GUI folders.  "Helper targets" that don't have interesting source code should set their FOLDER property to this
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake predefined targets")
set(LOADER_FOLDER "Loader")
set(HELPER_FOLDER "Helpers")
set(CODEGEN_FOLDER "Generated")
set(TESTS_FOLDER "Tests")
set(API_LAYERS_FOLDER "Layers")
set(SAMPLES_FOLDER "Samples")

option(BUILD_FORCE_GENERATION "Force re-generation of files even in the presence of pre-generated copies, replacing those copies." OFF)

if(BUILD_FORCE_GENERATION AND NOT PYTHON_EXECUTABLE)
    message(FATAL_ERROR "BUILD_FORCE_GENERATION requires Python")
endif()

add_subdirectory(include)
add_subdirectory(src)

# uninstall target
if(NOT TARGET uninstall)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/cmake/cmake_uninstall.cmake.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
                   IMMEDIATE
                   @ONLY)
    add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
    set_target_properties(uninstall PROPERTIES FOLDER ${HELPER_FOLDER})
endif()

find_program(BASH_COMMAND NAMES bash)
if(BASH_COMMAND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/specification/Makefile")
    option(BUILD_SPECIFICATION "Run './makeAllExts all' in the specification directory as part of the build - intended for one-step checking of spec changes" OFF)
    if(BUILD_SPECIFICATION)
        add_custom_target(spec-all ALL
            ${BASH_COMMAND} ./makeAllExts all
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/specification"
            VERBATIM
            COMMENT "Running './makeAllExts all' in the specification directory"
            USES_TERMINAL)
    endif()
endif()

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE" DESTINATION share/doc/openxr)
