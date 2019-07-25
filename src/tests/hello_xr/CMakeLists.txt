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

file(GLOB LOCAL_HEADERS "*.h")
file(GLOB LOCAL_SOURCE "*.cpp")
file(GLOB VULKAN_SHADERS "vulkan_shaders/*.glsl")

# For including compiled shaders
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(hello_xr
    ${LOCAL_SOURCE}
    ${LOCAL_HEADERS}
    ${VULKAN_SHADERS})
set_target_properties(hello_xr PROPERTIES FOLDER ${SAMPLES_FOLDER})

source_group("Headers" FILES ${LOCAL_HEADERS})
source_group("Shaders" FILES ${VULKAN_SHADERS})

# Find glslc shader compiler.
# On Android, the NDK includes the binary, so no external dependency.
if(ANDROID)
    file(GLOB glslc-folders ${ANDROID_NDK}/shader-tools/*)
else()
    file(GLOB glslc-folders $ENV{VULKAN_SDK}/*)
endif()
find_program(CMAKE_GLSL_COMPILER glslc PATHS ${glslc-folders} NO_DEFAULT_PATH)
if(CMAKE_GLSL_COMPILER)
    message(STATUS "Found glslc: ${CMAKE_GLSL_COMPILER}")
else()
    message(STATUS "Could NOT find glslc, using precompiled .spv files")
endif()

function(compile_glsl)
    set(glsl_output_files "")
    foreach(in_file IN LISTS ARGN)
        get_filename_component(glsl_stage ${in_file} NAME_WE)
        set(out_file ${CMAKE_CURRENT_BINARY_DIR}/${glsl_stage}.spv)
        if(CMAKE_GLSL_COMPILER)
            # Run glslc if we can find it
            add_custom_command(
                OUTPUT ${out_file}
                COMMAND ${CMAKE_GLSL_COMPILER} -mfmt=c -fshader-stage=${glsl_stage} ${in_file} -o ${out_file}
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
    add_custom_target(
        run_glsl_compiles ALL
        DEPENDS ${glsl_output_files}
    )
	set_target_properties(run_glsl_compiles PROPERTIES FOLDER ${HELPER_FOLDER})

endfunction()

compile_glsl(${VULKAN_SHADERS})

add_dependencies(hello_xr
    generate_openxr_header
    run_glsl_compiles
)

target_include_directories(hello_xr
    PRIVATE ${CMAKE_SOURCE_DIR}/src
    PRIVATE ${CMAKE_SOURCE_DIR}/src/common
    PRIVATE ${CMAKE_BINARY_DIR}/include
    PRIVATE ${CMAKE_SOURCE_DIR}/external/include
)

if(VulkanHeaders_INCLUDE_DIRS)
    target_include_directories(hello_xr
        PRIVATE
        ${VulkanHeaders_INCLUDE_DIRS}
    )
endif()
if(OPENGL_FOUND)
    target_sources(hello_xr
        PRIVATE
        ${PROJECT_SOURCE_DIR}/src/common/gfxwrapper_opengl.c
    )
endif()

if(WIN32)
    set(OPENXR_LIB openxr_loader-${MAJOR}_${MINOR})
else()
    set(OPENXR_LIB openxr_loader)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(MSVC)
        target_compile_definitions(hello_xr PRIVATE _CRT_SECURE_NO_WARNINGS)
        target_compile_options(hello_xr PRIVATE /Zc:wchar_t /Zc:forScope /W4 /WX)
    endif()
    target_link_libraries(hello_xr ${OPENXR_LIB} d3d11 d3dcompiler dxgi ${OPENGL_gl_LIBRARY})
    if(Vulkan_LIBRARY)
        target_link_libraries(hello_xr ${Vulkan_LIBRARY})
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_options(hello_xr PRIVATE -Wall)
    target_link_libraries(hello_xr ${OPENXR_LIB} m pthread GL ${XLIB_LIBRARIES} ${XCB_LIBRARIES} ${WAYLAND_LIBRARIES})
    if(Vulkan_LIBRARY)
        target_link_libraries(hello_xr ${Vulkan_LIBRARY})
    endif()
    target_sources(hello_xr PRIVATE ${WAYLAND_PROTOCOL_SRC})
endif()
