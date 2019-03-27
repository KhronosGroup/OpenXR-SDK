# Copyright (c) 2017 The Khronos Group Inc.
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

# Entire project uses C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# All options defined here
option(BUILD_LOADER "Build loader" ON)
option(DYNAMIC_LOADER "Build the loader as a .dll library" OFF)
option(BUILD_API_LAYERS "Build API layers" ON)
option(BUILD_TESTS "Build tests" ON)

# Several files use these compile-time OS switches
if(WIN32)
    add_definitions(-DXR_OS_WINDOWS)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_definitions(-DXR_OS_LINUX)
endif()

# Determine the presentation backend for Linux systems.
# Use an include because the code is pretty big.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    include(presentation)
endif()

# Several files use these compile-time platform switches
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_definitions( -DXR_USE_PLATFORM_WIN32 )
elseif( PRESENTATION_BACKEND MATCHES "xlib" )
    add_definitions( -DXR_USE_PLATFORM_XLIB )
elseif( PRESENTATION_BACKEND MATCHES "xcb" )
    add_definitions( -DXR_USE_PLATFORM_XCB )
elseif( PRESENTATION_BACKEND MATCHES "wayland" )
    add_definitions( -DXR_USE_PLATFORM_WAYLAND )
endif()

# Enable graphics API available to the build.
if (NOT CMAKE_VERSION VERSION_LESS 3.7.0)
    # Find the Vulkan headers
    find_package(VulkanHeaders)
    if (VulkanHeaders_FOUND)
        add_definitions(-DXR_USE_GRAPHICS_API_VULKAN)
    endif()
    # Find the Vulkan loader.
    find_package(Vulkan)
    # To use simply include ${Vulkan_LIBRARY} in your target_link_library or
    # wherever you normally link your library files to your target.
endif()

find_package(OpenGL)
if (OPENGL_FOUND)
    add_definitions(-DXR_USE_GRAPHICS_API_OPENGL)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_definitions(-DXR_USE_GRAPHICS_API_D3D)
    add_definitions(-DXR_USE_GRAPHICS_API_D3D10)
    add_definitions(-DXR_USE_GRAPHICS_API_D3D11)
    add_definitions(-DXR_USE_GRAPHICS_API_D3D12)
endif()

# Check for the existence of the secure_getenv or __secure_getenv commands
include(CheckFunctionExists)
CHECK_FUNCTION_EXISTS(secure_getenv HAVE_SECURE_GETENV)
CHECK_FUNCTION_EXISTS(__secure_getenv HAVE___SECURE_GETENV)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/common_cmake_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/common_cmake_config.h)

# Set up the OpenXR version variables, used by several targets in this project.
set(MAJOR "0")
set(MINOR "0")
set(PATCH "0")

# Fetch the Major/Minor version from registry
file(STRINGS ${CMAKE_SOURCE_DIR}/specification/registry/xr.xml lines REGEX "#define <name>XR_CURRENT_API_VERSION")
list(LENGTH lines len)
if(${len} EQUAL 1)
    list(GET lines 0 cur_line)
    # Erase everything up to the open parentheses
    string(REGEX REPLACE "^[^\(]+" "" VERSION_BEFORE_ERASED ${cur_line})
    # Erase everything after the close parentheses
    string(REGEX REPLACE "[^\)]+$" "" VERSION_AFTER_ERASED ${VERSION_BEFORE_ERASED})
    # Erase the parentheses
    string(REPLACE "(" "" VERSION_AFTER_ERASED2 ${VERSION_AFTER_ERASED})
    string(REPLACE ")" "" VERSION_AFTER_ERASED3 ${VERSION_AFTER_ERASED2})
    string(REPLACE " " "" VERSION_AFTER_ERASED4 ${VERSION_AFTER_ERASED3})
    string(REGEX REPLACE "^([0-9]+)\\,[0-9]+\\,[0-9]+" "\\1" MAJOR "${VERSION_AFTER_ERASED4}")
    string(REGEX REPLACE "^[0-9]+\\,([0-9]+)\\,[0-9]+" "\\1" MINOR "${VERSION_AFTER_ERASED4}")
    string(REGEX REPLACE "^[0-9]+\\,[0-9]+\\,([0-9]+)" "\\1" PATCH "${VERSION_AFTER_ERASED4}")
else()
    message(FATAL_ERROR "Unable to fetch major/minor version from registry")
endif()

# TODO Not sure BASE_DIR is used?
if(WIN32)
    string(REGEX REPLACE "/" "\\\\" BASE_DIR "${CMAKE_SOURCE_DIR}")
else()
    set(BASE_DIR "${CMAKE_SOURCE_DIR}")
endif()

# Path separators ( : or ; ) are not handled well in CMake.
# This seems like a reasonable approach.
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CODEGEN_PYTHON_PATH "${CMAKE_SOURCE_DIR}/specification/scripts;${CMAKE_SOURCE_DIR}/src/scripts;$ENV{PYTHONPATH}")
else()
    set(CODEGEN_PYTHON_PATH "${CMAKE_SOURCE_DIR}/specification/scripts:${CMAKE_SOURCE_DIR}/src/scripts:$ENV{PYTHONPATH}")
endif()

# General code generation macro used by several targets.
macro(run_xr_xml_generate dependency output)
    add_custom_command(OUTPUT ${output}
        COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=${CODEGEN_PYTHON_PATH}"
            ${PYTHON_EXECUTABLE}
                ${CMAKE_SOURCE_DIR}/src/scripts/src_genxr.py
                -registry ${CMAKE_SOURCE_DIR}/specification/registry/xr.xml
                ${output}
        DEPENDS 
            ${CMAKE_SOURCE_DIR}/specification/registry/xr.xml
            ${CMAKE_SOURCE_DIR}/specification/scripts/generator.py
            ${CMAKE_SOURCE_DIR}/specification/scripts/reg.py
            ${CMAKE_SOURCE_DIR}/src/scripts/${dependency}
            ${CMAKE_SOURCE_DIR}/src/scripts/src_genxr.py
        COMMENT "Generating ${output} using ${PYTHON_EXECUTABLE} on ${dependency}"
    )
    set_source_files_properties(${output} PROPERTIES GENERATED TRUE)
endmacro()

# Layer JSON generation macro used by several targets.
macro(gen_xr_layer_json filename layername libfile version desc genbad)
    add_custom_command(OUTPUT ${filename}
        COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=${CODEGEN_PYTHON_PATH}"
            ${PYTHON_EXECUTABLE}
                ${CMAKE_SOURCE_DIR}/src/scripts/generate_api_layer_manifest.py
                    -f ${filename} -n ${layername} -l ${libfile} -a ${MAJOR}.${MINOR} -v ${version} ${genbad} -d ${desc}
        DEPENDS ${CMAKE_SOURCE_DIR}/src/scripts/generate_api_layer_manifest.py
        COMMENT "Generating API Layer JSON ${filename} using -f ${filename} -n ${layername} -l ${libfile} -a ${MAJOR}.${MINOR} -v ${version} ${genbad} -d ${desc}"
    )
endmacro()

# Custom target for generated dispatch table sources, used by several targets.
add_custom_target(xr_global_generated_files DEPENDS
    xr_generated_dispatch_table.h
    xr_generated_dispatch_table.c
    xr_generated_utilities.h
    xr_generated_utilities.c
)
run_xr_xml_generate(utility_source_generator.py xr_generated_dispatch_table.h)
run_xr_xml_generate(utility_source_generator.py xr_generated_dispatch_table.c)
run_xr_xml_generate(utility_source_generator.py xr_generated_utilities.h)
run_xr_xml_generate(utility_source_generator.py xr_generated_utilities.c)

add_subdirectory(external)

if(BUILD_LOADER)
    add_subdirectory(loader)
endif()

if(BUILD_API_LAYERS)
    add_subdirectory(api_layers)
endif()

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
