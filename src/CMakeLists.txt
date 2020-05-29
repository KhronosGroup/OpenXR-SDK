# Copyright (c) 2017 The Khronos Group Inc.
#
# SPDX-License-Identifier: Apache-2.0
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

if(POLICY CMP0075)
    cmake_policy(SET CMP0075 NEW)
endif()

# Entire project uses C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(GNUInstallDirs)
include(StdFilesystemFlags)

### Dependencies
set(OpenGL_GL_PREFERENCE GLVND)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
    # OpenGL package will be found for UWP apps but gl.h excludes UWP apps so it isn't actually usable.
    find_package(OpenGL)
endif()

if(OPENGL_FOUND)
    add_definitions(-DXR_USE_GRAPHICS_API_OPENGL)
    message(STATUS "Enabling OpenGL support")
elseif(BUILD_ALL_EXTENSIONS AND NOT CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
    message(FATAL_ERROR "OpenGL not found")
endif()

if(NOT CMAKE_VERSION VERSION_LESS 3.7.0)
    # Find the Vulkan headers
    find_package(VulkanHeaders)
    find_package(Vulkan)
endif()
if(VulkanHeaders_FOUND)
    add_definitions(-DXR_USE_GRAPHICS_API_VULKAN)
    message(STATUS "Enabling Vulkan support")
elseif(BUILD_ALL_EXTENSIONS)
    message(FATAL_ERROR "Vulkan headers not found")
endif()

find_package(Threads REQUIRED)
find_package(JsonCpp)

### All options defined here
option(BUILD_LOADER "Build loader" ON)
option(BUILD_ALL_EXTENSIONS "Build loader and layers with all extensions" OFF)
option(
    BUILD_LOADER_WITH_EXCEPTION_HANDLING
    "Enable exception handling in the loader. Leave this on unless your standard library is built to not throw."
    ON
)

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
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/conformance")
    option(BUILD_CONFORMANCE_TESTS "Build conformance tests" ON)
endif()
include(CMakeDependentOption)

cmake_dependent_option(
    BUILD_WITH_SYSTEM_JSONCPP "Use system jsoncpp instead of vendored source" ON "JSONCPP_FOUND" OFF
)
cmake_dependent_option(BUILD_WITH_STD_FILESYSTEM "Use std::[experimental::]filesystem." ON
       "HAVE_FILESYSTEM_WITHOUT_LIB OR HAVE_FILESYSTEM_NEEDING_LIB" OFF)

# Several files use these compile-time OS switches
if(WIN32)
    add_definitions(-DXR_OS_WINDOWS)
    add_definitions(-DNOMINMAX)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_definitions(-DXR_OS_LINUX)
endif()

# /EHsc (support for C++ exceptions) is default in most configurations but seems missing when building arm/arm64.
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
endif()

# This is a little helper library for setting up OpenGL
if(OPENGL_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/common/gfxwrapper_opengl.c")
    add_library(openxr-gfxwrapper STATIC common/gfxwrapper_opengl.c common/gfxwrapper_opengl.h)
    target_include_directories(openxr-gfxwrapper PUBLIC ${PROJECT_SOURCE_DIR}/external/include)
    if(TARGET OpenGL::OpenGL)
        target_link_libraries(openxr-gfxwrapper PUBLIC OpenGL::OpenGL)
    elseif(TARGET OpenGL::GL)
        target_link_libraries(openxr-gfxwrapper PUBLIC OpenGL::GL)
    else()
        target_link_libraries(openxr-gfxwrapper PUBLIC ${OPENGL_LIBRARIES})
    endif()
    message(STATUS "Enabling OpenGL support in hello_xr, loader_test, and conformance, if configured")
endif()

# Determine the presentation backend for Linux systems.
# Use an include because the code is pretty big.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    include(presentation)
endif()

# Several files use these compile-time platform switches
if(WIN32)
    add_definitions(-DXR_USE_PLATFORM_WIN32)
    # TODO remove once work is done to get more stuff building for UWP.
    if (CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
        set(BUILD_TESTS OFF)
        set(BUILD_CONFORMANCE_TESTS OFF)
    endif()
elseif(PRESENTATION_BACKEND MATCHES "xlib")
    add_definitions(-DXR_USE_PLATFORM_XLIB)
elseif(PRESENTATION_BACKEND MATCHES "xcb")
    add_definitions(-DXR_USE_PLATFORM_XCB)

    # TODO remove once conformance supports XCB
    set(BUILD_CONFORMANCE_TESTS OFF)
elseif(PRESENTATION_BACKEND MATCHES "wayland")
    add_definitions(-DXR_USE_PLATFORM_WAYLAND)

    # TODO remove once conformance supports Wayland
    set(BUILD_CONFORMANCE_TESTS OFF)
endif()

add_library(openxr-all-supported INTERFACE)
if(BUILD_WITH_XLIB_HEADERS)
    target_compile_definitions(openxr-all-supported INTERFACE XR_USE_PLATFORM_XLIB)
endif()

if(BUILD_WITH_XCB_HEADERS)
    target_compile_definitions(openxr-all-supported INTERFACE XR_USE_PLATFORM_XCB)
endif()

if(BUILD_WITH_WAYLAND_HEADERS)
    target_compile_definitions(openxr-all-supported INTERFACE XR_USE_PLATFORM_WAYLAND)
endif()

# Find glslc shader compiler.
# On Android, the NDK includes the binary, so no external dependency.
if(ANDROID)
    file(GLOB glslc_folders ${ANDROID_NDK}/shader-tools/*)
    find_program(
        GLSL_COMPILER glslc
        PATHS ${glslc_folders}
        NO_DEFAULT_PATH
    )
else()
    file(GLOB glslc_folders $ENV{VULKAN_SDK}/*)
    find_program(GLSL_COMPILER glslc PATHS ${glslc_folders})
endif()
if(GLSL_COMPILER)
    message(STATUS "Found glslc: ${GLSL_COMPILER}")
else()
    message(STATUS "Could NOT find glslc, using precompiled .spv files")
endif()

function(compile_glsl run_target_name)
    set(glsl_output_files "")
    foreach(in_file IN LISTS ARGN)
        get_filename_component(glsl_stage ${in_file} NAME_WE)
        set(out_file ${CMAKE_CURRENT_BINARY_DIR}/${glsl_stage}.spv)
        if(GLSL_COMPILER)
            # Run glslc if we can find it
            add_custom_command(
                OUTPUT ${out_file}
                COMMAND ${GLSL_COMPILER} -mfmt=c -fshader-stage=${glsl_stage} ${in_file} -o ${out_file}
                DEPENDS ${in_file}
            )
        else()
            # Use the precompiled .spv files
            get_filename_component(glsl_src_dir ${in_file} DIRECTORY)
            set(precompiled_file ${glsl_src_dir}/${glsl_stage}.spv)
            configure_file(${precompiled_file} ${out_file} COPYONLY)
        endif()
        list(APPEND glsl_output_files ${out_file})
    endforeach()
    add_custom_target(${run_target_name} ALL DEPENDS ${glsl_output_files})
    set_target_properties(${run_target_name} PROPERTIES FOLDER ${HELPER_FOLDER})

endfunction()

if(WIN32)
    # This is too simple of detection to be worth separating into a find module
    find_library(D3D_DXGI_LIBRARY dxgi)
    find_library(D3D_COMPILER_LIBRARY d3dcompiler)
    find_path(D3D_D3D11_INCLUDE_DIR d3d11.h)
    find_library(D3D_D3D11_LIBRARY d3d11)
    if(D3D_D3D11_INCLUDE_DIR AND D3D_D3D11_LIBRARY AND D3D_DXGI_LIBRARY AND D3D_COMPILER_LIBRARY)
        set(D3D_D3D11_FOUND ON)
        set(D3D_D3D11_LIBRARIES ${D3D_D3D11_LIBRARY} ${D3D_DXGI_LIBRARY} ${D3D_COMPILER_LIBRARY})
        add_definitions(-DXR_USE_GRAPHICS_API_D3D11)
    endif()

    find_path(D3D_D3D12_INCLUDE_DIR d3d12.h)
    find_library(D3D_D3D12_LIBRARY d3d12)
    if(D3D_D3D12_INCLUDE_DIR AND D3D_D3D12_LIBRARY AND D3D_DXGI_LIBRARY)
        set(D3D_D3D12_FOUND ON)
        set(D3D_D3D12_LIBRARIES ${D3D_D3D12_LIBRARY} ${D3D_DXGI_LIBRARY} ${D3D_COMPILER_LIBRARY})
        add_definitions(-DXR_USE_GRAPHICS_API_D3D12)
    endif()

    find_path(D3D_DIRECTXCOLORS_INCLUDE_DIR DirectXColors.h)
endif()

# Check for the existence of the secure_getenv or __secure_getenv commands
include(CheckFunctionExists)

check_function_exists(secure_getenv HAVE_SECURE_GETENV)
check_function_exists(__secure_getenv HAVE___SECURE_GETENV)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/common_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/common_config.h)
add_definitions(-DOPENXR_HAVE_COMMON_CONFIG)

# Be able to find pre-generated files, if used.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include ${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DXR_USE_TIMESPEC)

# Set up the OpenXR version variables, used by several targets in this project.
include(${CMAKE_CURRENT_SOURCE_DIR}/version.cmake)

# Path separators ( : or ; ) are not handled well in CMake.
# This seems like a reasonable approach.
if(CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
    set(CODEGEN_PYTHON_PATH
        "${PROJECT_SOURCE_DIR}/specification/scripts;${PROJECT_SOURCE_DIR}/src/scripts;$ENV{PYTHONPATH}"
    )
else()
    set(CODEGEN_PYTHON_PATH
        "${PROJECT_SOURCE_DIR}/specification/scripts:${PROJECT_SOURCE_DIR}/src/scripts:$ENV{PYTHONPATH}"
    )
endif()

# General code generation macro used by several targets.
macro(run_xr_xml_generate dependency output)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${output}" AND NOT BUILD_FORCE_GENERATION)
        # pre-generated found
        message(STATUS "Found and will use pre-generated ${output} in source tree")
        list(APPEND GENERATED_OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/${output}")
    else()
        if(NOT PYTHON_EXECUTABLE)
            message(
                FATAL_ERROR
                    "Python 3 not found, but pre-generated ${CMAKE_CURRENT_SOURCE_DIR}/${output} not found"
            )
        endif()
        add_custom_command(
            OUTPUT ${output}
            COMMAND
                ${CMAKE_COMMAND} -E env "PYTHONPATH=${CODEGEN_PYTHON_PATH}"
                ${PYTHON_EXECUTABLE}
                ${PROJECT_SOURCE_DIR}/src/scripts/src_genxr.py
                -registry ${PROJECT_SOURCE_DIR}/specification/registry/xr.xml
                ${output}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS "${PROJECT_SOURCE_DIR}/specification/registry/xr.xml"
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
    add_custom_command(
        OUTPUT ${filename}
        COMMAND
            ${CMAKE_COMMAND} -E env "PYTHONPATH=${CODEGEN_PYTHON_PATH}"
            ${PYTHON_EXECUTABLE}
            ${PROJECT_SOURCE_DIR}/src/scripts/generate_api_layer_manifest.py
            -f ${filename}
            -n ${layername}
            -l ${libfile}
            -a ${MAJOR}.${MINOR}
            -v ${version}
            ${genbad}
            -d ${desc}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${PROJECT_SOURCE_DIR}/src/scripts/generate_api_layer_manifest.py
        COMMENT
            "Generating API Layer JSON ${filename} using -f ${filename} -n ${layername} -l ${libfile} -a ${MAJOR}.${MINOR} -v ${version} ${genbad} -d ${desc}"
    )
endmacro()

# Custom target for generated dispatch table sources, used by several targets.
set(GENERATED_OUTPUT)
set(GENERATED_DEPENDS)
run_xr_xml_generate(utility_source_generator.py xr_generated_dispatch_table.h)
run_xr_xml_generate(utility_source_generator.py xr_generated_dispatch_table.c)
add_custom_target(xr_global_generated_files DEPENDS ${GENERATED_DEPENDS})
set_target_properties(xr_global_generated_files PROPERTIES FOLDER ${CODEGEN_FOLDER})

set(COMMON_GENERATED_OUTPUT ${GENERATED_OUTPUT})
if(NOT MSVC)
    include(CheckCXXCompilerFlag)
    include(CheckCCompilerFlag)
    foreach(FLAG -Wall -Werror=unused-parameter -Werror=unused-argument -Wpointer-arith)
        string(REGEX REPLACE "[^A-Za-z0-9]" "" _flagvar "${FLAG}")
        check_cxx_compiler_flag(${FLAG} SUPPORTS_${_flagvar})
        if(SUPPORTS_${_flagvar})
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")
        endif()
        check_c_compiler_flag(${FLAG} SUPPORTS_C_${_flagvar})
        if(SUPPORTS_C_${_flagvar})
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAG}")
        endif()
    endforeach()
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
endif()

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
