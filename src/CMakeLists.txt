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
option(BUILD_ALL_EXTENSIONS "Build loader and layers with all extensions" OFF)
option(BUILD_LOADER_WITH_EXCEPTION_HANDLING "Enable exception handling in the loader. Leave this on unless your standard library is built to not throw." ON)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    option(DYNAMIC_LOADER "Build the loader as a .dll library" OFF)
else()
    option(DYNAMIC_LOADER "Build the loader as a .dll library" ON)
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/api_layers")
    option(BUILD_API_LAYERS "Build API layers" ON)
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests" AND NOT CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
    option(BUILD_TESTS "Build tests" ON)
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/conformance" AND NOT CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
    option(BUILD_CONFORMANCE_TESTS "Build conformance tests" ON)
endif()


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
if(WIN32)
    add_definitions( -DXR_USE_PLATFORM_WIN32 )
elseif( PRESENTATION_BACKEND MATCHES "xlib" )
    add_definitions( -DXR_USE_PLATFORM_XLIB )
elseif( PRESENTATION_BACKEND MATCHES "xcb" )
    add_definitions( -DXR_USE_PLATFORM_XCB )

    # TODO remove once conformance supports XCB
    set(BUILD_CONFORMANCE_TESTS OFF)
elseif( PRESENTATION_BACKEND MATCHES "wayland" )
    add_definitions( -DXR_USE_PLATFORM_WAYLAND )

    # TODO remove once conformance supports Wayland
    set(BUILD_CONFORMANCE_TESTS OFF)
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

if (BUILD_ALL_EXTENSIONS AND NOT VulkanHeaders_FOUND)
    message(FATAL_ERROR "Vulkan headers not found")
endif()

find_package(OpenGL)
if (OPENGL_FOUND)
    add_definitions(-DXR_USE_GRAPHICS_API_OPENGL)
elseif(BUILD_ALL_EXTENSIONS)
    message(FATAL_ERROR "OpenGL not found")
endif()

if(WIN32)
    add_definitions(-DXR_USE_GRAPHICS_API_D3D11)
    add_definitions(-DXR_USE_GRAPHICS_API_D3D12)
endif()

# Check for the existence of the secure_getenv or __secure_getenv commands
include(CheckFunctionExists)

check_function_exists(secure_getenv HAVE_SECURE_GETENV)
check_function_exists(__secure_getenv HAVE___SECURE_GETENV)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/common_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/common_config.h)
add_definitions(-DOPENXR_HAVE_COMMON_CONFIG)

# Be able to find pre-generated files, if used.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DXR_USE_TIMESPEC)

# Set up the OpenXR version variables, used by several targets in this project.
include(${CMAKE_CURRENT_SOURCE_DIR}/version.cmake)

# Path separators ( : or ; ) are not handled well in CMake.
# This seems like a reasonable approach.
if(WIN32)
    set(CODEGEN_PYTHON_PATH "${PROJECT_SOURCE_DIR}/specification/scripts;${PROJECT_SOURCE_DIR}/src/scripts;$ENV{PYTHONPATH}")
else()
    set(CODEGEN_PYTHON_PATH "${PROJECT_SOURCE_DIR}/specification/scripts:${PROJECT_SOURCE_DIR}/src/scripts:$ENV{PYTHONPATH}")
endif()

# General code generation macro used by several targets.
macro(run_xr_xml_generate dependency output)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${output}" AND NOT BUILD_FORCE_GENERATION)
        # pre-generated found
        message(STATUS "Found and will use pre-generated ${output} in source tree")
        list(APPEND GENERATED_OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/${output}")
    else()
        if(NOT PYTHON_EXECUTABLE)
            message(FATAL_ERROR "Python 3 not found, but pre-generated ${CMAKE_CURRENT_SOURCE_DIR}/${output} not found")
        endif()
        add_custom_command(OUTPUT ${output}
            COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=${CODEGEN_PYTHON_PATH}"
                ${PYTHON_EXECUTABLE}
                    ${PROJECT_SOURCE_DIR}/src/scripts/src_genxr.py
                    -registry ${PROJECT_SOURCE_DIR}/specification/registry/xr.xml
                    ${output}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS
                    "${PROJECT_SOURCE_DIR}/specification/registry/xr.xml"
                    "${PROJECT_SOURCE_DIR}/specification/scripts/generator.py"
                    "${PROJECT_SOURCE_DIR}/specification/scripts/reg.py"
                    "${PROJECT_SOURCE_DIR}/src/scripts/${dependency}"
                    "${PROJECT_SOURCE_DIR}/src/scripts/src_genxr.py"
                ${ARGN}
            COMMENT "Generating ${output} using ${PYTHON_EXECUTABLE} on ${dependency}"
        )
        set_source_files_properties(${output} PROPERTIES GENERATED TRUE)
        list(APPEND GENERATED_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${output}")
        list(APPEND GENERATED_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${output}")
    endif()
endmacro()

# Layer JSON generation macro used by several targets.
macro(gen_xr_layer_json filename layername libfile version desc genbad)
    add_custom_command(OUTPUT ${filename}
        COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=${CODEGEN_PYTHON_PATH}"
            ${PYTHON_EXECUTABLE}
                ${PROJECT_SOURCE_DIR}/src/scripts/generate_api_layer_manifest.py
                    -f ${filename} -n ${layername} -l ${libfile} -a ${MAJOR}.${MINOR} -v ${version} ${genbad} -d ${desc}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${PROJECT_SOURCE_DIR}/src/scripts/generate_api_layer_manifest.py
        COMMENT "Generating API Layer JSON ${filename} using -f ${filename} -n ${layername} -l ${libfile} -a ${MAJOR}.${MINOR} -v ${version} ${genbad} -d ${desc}"
    )
endmacro()

# Custom target for generated dispatch table sources, used by several targets.
set(GENERATED_OUTPUT)
set(GENERATED_DEPENDS)
run_xr_xml_generate(utility_source_generator.py xr_generated_dispatch_table.h)
run_xr_xml_generate(utility_source_generator.py xr_generated_dispatch_table.c)
add_custom_target(xr_global_generated_files DEPENDS
    ${GENERATED_DEPENDS}
)
set_target_properties(xr_global_generated_files PROPERTIES FOLDER ${CODEGEN_FOLDER})

set(COMMON_GENERATED_OUTPUT ${GENERATED_OUTPUT})

if(BUILD_LOADER)
    add_subdirectory(loader)
endif()

if(BUILD_API_LAYERS)
    add_subdirectory(api_layers)
endif()

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(BUILD_CONFORMANCE_TESTS)
    add_subdirectory(conformance)
endif()



