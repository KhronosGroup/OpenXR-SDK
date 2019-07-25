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

# Force all compilers to output to binary folder without additional output (like Windows adds "Debug" and "Release" folders)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_CURRENT_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_CURRENT_BINARY_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_CURRENT_BINARY_DIR})
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

add_library(XrApiLayer_test SHARED
    layer_test.cpp
)
set_target_properties(XrApiLayer_test PROPERTIES FOLDER ${TESTS_FOLDER})

add_dependencies(XrApiLayer_test
    xr_global_generated_files
    generate_openxr_header
    generated_layer_json_files
)
target_include_directories(XrApiLayer_test
    PRIVATE ${CMAKE_SOURCE_DIR}/src
    PRIVATE ${CMAKE_SOURCE_DIR}/src/common
    PRIVATE ${CMAKE_BINARY_DIR}/include
)
if(VulkanHeaders_FOUND)
    target_include_directories(XrApiLayer_test
        PRIVATE ${Vulkan_INCLUDE_DIRS}
    )
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(XrApiLayer_test PRIVATE _CRT_SECURE_NO_WARNINGS)
    # Turn off transitional "changed behavior" warning message for Visual Studio versions prior to 2015.
    # The changed behavior is that constructor initializers are now fixed to clear the struct members.
    target_compile_options(XrApiLayer_test PRIVATE "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,19>>:/wd4351>")
    gen_xr_layer_json(
        ${CMAKE_BINARY_DIR}/src/tests/loader_test/resources/layers/XrApiLayer_test.json
        test
        ${CMAKE_CURRENT_BINARY_DIR}/XrApiLayer_test.dll
        1
        Test_description
        -b
    )
    FILE(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/XrApiLayer_test.def DEF_FILE)
    add_custom_target(copy-test-def-file ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DEF_FILE} ${CMAKE_CURRENT_BINARY_DIR}/XrApiLayer_test.def
        VERBATIM
    )
	set_target_properties(copy-test-def-file PROPERTIES FOLDER ${HELPER_FOLDER})
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_options(XrApiLayer_test PRIVATE -Wpointer-arith -Wno-unused-function -Wno-sign-compare)
    set_target_properties(XrApiLayer_test PROPERTIES LINK_FLAGS "-Wl,-Bsymbolic,--exclude-libs,ALL")
    gen_xr_layer_json(
        ${CMAKE_BINARY_DIR}/src/tests/loader_test/resources/layers/XrApiLayer_test.json
        test
        ${CMAKE_CURRENT_BINARY_DIR}/libXrApiLayer_test.so
        1
        Test_description
        -b
    )
endif()

add_custom_target(generated_layer_json_files DEPENDS
    ${CMAKE_BINARY_DIR}/src/tests/loader_test/resources/layers/XrApiLayer_test.json
)
set_target_properties(generated_layer_json_files PROPERTIES FOLDER ${CODEGEN_FOLDER})

