# Copyright (c) 2017-2019 The Khronos Group Inc.
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

# Copy the api_layer_platform_defines.h file and place it in the binary (build) directory.
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/api_layer_platform_defines.h
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/api_layer_platform_defines.h ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/api_layer_platform_defines.h
    COMMENT "Copying ${CMAKE_CURRENT_SOURCE_DIR}/api_layer_platform_defines.h to ${CMAKE_CURRENT_BINARY_DIR}"
)

# Basics for api_dump API Layer
add_library(XrApiLayer_api_dump SHARED
    api_dump.cpp
    ${CMAKE_BINARY_DIR}/src/xr_generated_dispatch_table.c
    ${CMAKE_BINARY_DIR}/src/api_layers/xr_generated_api_dump.cpp
)
add_dependencies(XrApiLayer_api_dump
    generate_openxr_header
    xr_global_generated_files
    api_dump_gen_files
    api_dump_json_file
)

target_include_directories(XrApiLayer_api_dump
    PRIVATE ${CMAKE_SOURCE_DIR}/src/common
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_BINARY_DIR}/include
    PRIVATE ${CMAKE_BINARY_DIR}/src
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)
if(VulkanHeaders_FOUND)
    target_include_directories(XrApiLayer_api_dump
        PRIVATE ${Vulkan_INCLUDE_DIRS}
    )
endif()

# Basics for core_validation API Layer
add_library(XrApiLayer_core_validation SHARED
    core_validation.cpp
    ${CMAKE_BINARY_DIR}/src/xr_generated_dispatch_table.c 
    ${CMAKE_BINARY_DIR}/src/api_layers/xr_generated_core_validation.cpp
)
add_dependencies(XrApiLayer_core_validation
    generate_openxr_header
    xr_global_generated_files
    core_validation_gen_files
    core_validation_json_file
)
target_include_directories(XrApiLayer_core_validation
    PRIVATE ${CMAKE_SOURCE_DIR}/src/common
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_BINARY_DIR}/include
    PRIVATE ${CMAKE_BINARY_DIR}/src
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)
if(VulkanHeaders_FOUND)
    target_include_directories(XrApiLayer_core_validation
        PRIVATE ${Vulkan_INCLUDE_DIRS}
    )
endif()

# Flag generated files
set_source_files_properties(
    ${CMAKE_BINARY_DIR}/src/xr_generated_dispatch_table.c
    ${CMAKE_BINARY_DIR}/src/api_layers/api_layer_platform_defines.h
    ${CMAKE_BINARY_DIR}/src/api_layers/xr_generated_api_dump.cpp
    ${CMAKE_BINARY_DIR}/src/api_layers/xr_generated_core_validation.cpp
    PROPERTIES GENERATED TRUE
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Windows api_dump-specific information
    target_compile_definitions(XrApiLayer_api_dump PRIVATE _CRT_SECURE_NO_WARNINGS)
    # Turn off transitional "changed behavior" warning message for Visual Studio versions prior to 2015.
    # The changed behavior is that constructor initializers are now fixed to clear the struct members.
    target_compile_options(XrApiLayer_api_dump PRIVATE "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,19>>:/wd4351>")

    FILE(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/XrApiLayer_api_dump.def DEF_FILE)
    add_custom_target(copy-api_dump-def-file ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DEF_FILE} ${CMAKE_CURRENT_BINARY_DIR}/XrApiLayer_api_dump.def
        VERBATIM
    )
    gen_xr_layer_json(
        ${CMAKE_CURRENT_BINARY_DIR}/XrApiLayer_api_dump.json
        api_dump
        ${CMAKE_CURRENT_BINARY_DIR}/libXrApiLayer_api_dump.dll
        1
        "API Layer to record api calls as they occur"
        ""
    )

    # Windows core_validation-specific information
    target_compile_definitions(XrApiLayer_core_validation PRIVATE _CRT_SECURE_NO_WARNINGS)
    # Turn off transitional "changed behavior" warning message for Visual Studio versions prior to 2015.
    # The changed behavior is that constructor initializers are now fixed to clear the struct members.
    target_compile_options(XrApiLayer_core_validation PRIVATE "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,19>>:/wd4351>")

    FILE(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/XrApiLayer_core_validation.def DEF_FILE)
    add_custom_target(copy-core_validation-def-file ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DEF_FILE} ${CMAKE_CURRENT_BINARY_DIR}/XrApiLayer_core_validation.def
        VERBATIM
    )
    gen_xr_layer_json(
        ${CMAKE_CURRENT_BINARY_DIR}/XrApiLayer_core_validation.json
        core_validation
        ${CMAKE_CURRENT_BINARY_DIR}/libXrApiLayer_core_validation.dll
        1
        "API Layer to record api calls as they occur"
        ""
    )
elseif(APPLE)
    # Apple api_dump-specific information
    target_compile_options(XrApiLayer_api_dump PRIVATE -Wpointer-arith -Wno-unused-function -Wno-sign-compare)
    set_target_properties(XrApiLayer_api_dump PROPERTIES LINK_FLAGS "-Wl")
    gen_xr_layer_json(
        ${CMAKE_CURRENT_BINARY_DIR}/XrApiLayer_api_dump.json
        api_dump
        ${CMAKE_CURRENT_BINARY_DIR}/libXrApiLayer_api_dump.dylib
        1
        "API Layer to record api calls as they occur"
        ""
    )

    # Apple core_validation-specific information
    target_compile_options(XrApiLayer_core_validation PRIVATE -Wpointer-arith -Wno-unused-function -Wno-sign-compare)
    set_target_properties(XrApiLayer_core_validation PROPERTIES LINK_FLAGS "-Wl")
    gen_xr_layer_json(
        ${CMAKE_CURRENT_BINARY_DIR}/XrApiLayer_core_validation.json
        core_validation
        ${CMAKE_CURRENT_BINARY_DIR}/libXrApiLayer_core_validation.dylib
        1
        "API Layer to record api calls as they occur"
        ""
    )

elseif(APPLE)
    # Apple api_dump-specific information
    target_compile_options(XrApiLayer_api_dump PRIVATE -Wpointer-arith -Wno-unused-function -Wno-sign-compare)
    set_target_properties(XrApiLayer_api_dump PROPERTIES LINK_FLAGS "-Wl")
    gen_xr_layer_json(
        ${CMAKE_CURRENT_BINARY_DIR}/XrApiLayer_api_dump.json
        api_dump
        ${CMAKE_CURRENT_BINARY_DIR}/libXrApiLayer_api_dump.dylib
        1
        "API Layer to record api calls as they occur"
        ""
    )

else()
    # Linux api_dump-specific information
    target_compile_options(XrApiLayer_api_dump PRIVATE -Wpointer-arith -Wno-unused-function -Wno-sign-compare)
    set_target_properties(XrApiLayer_api_dump PROPERTIES LINK_FLAGS "-Wl,-Bsymbolic,--exclude-libs,ALL")
    gen_xr_layer_json(
        ${CMAKE_CURRENT_BINARY_DIR}/XrApiLayer_api_dump.json
        api_dump
        libXrApiLayer_api_dump.so
        1
        "API Layer to record api calls as they occur"
        ""
    )
    install(TARGETS XrApiLayer_api_dump DESTINATION ${CMAKE_INSTALL_LIBDIR})

    # Linux core_validation-specific information
    target_compile_options(XrApiLayer_core_validation PRIVATE -Wpointer-arith -Wno-unused-function -Wno-sign-compare)
    set_target_properties(XrApiLayer_core_validation PROPERTIES LINK_FLAGS "-Wl,-Bsymbolic,--exclude-libs,ALL")
    gen_xr_layer_json(
        ${CMAKE_CURRENT_BINARY_DIR}/XrApiLayer_core_validation.json
        core_validation
        libXrApiLayer_core_validation.so
        1
        "API Layer to record api calls as they occur"
        ""
    )
    install(TARGETS XrApiLayer_core_validation DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

# Final bits for api_dump API Layer
add_custom_target(api_dump_gen_files DEPENDS
    xr_generated_api_dump.hpp
    xr_generated_api_dump.cpp
)
run_xr_xml_generate(api_dump_generator.py xr_generated_api_dump.hpp)
run_xr_xml_generate(api_dump_generator.py xr_generated_api_dump.cpp)

add_custom_target(api_dump_json_file DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/XrApiLayer_api_dump.json
)

# Final bits for core_validation API Layer
add_custom_target(core_validation_gen_files DEPENDS
    xr_generated_core_validation.hpp
    xr_generated_core_validation.cpp
)
run_xr_xml_generate(validation_layer_generator.py xr_generated_core_validation.hpp)
run_xr_xml_generate(validation_layer_generator.py xr_generated_core_validation.cpp)

add_custom_target(core_validation_json_file DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/XrApiLayer_core_validation.json
)

# Install explicit layers on Linux
set(TARGET_NAMES
    XrApiLayer_api_dump
    XrApiLayer_core_validation)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    foreach(TARGET_NAME ${TARGET_NAMES})
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.json DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/openxr/${MAJOR}/api_layers/explicit.d)
    endforeach()
endif()
