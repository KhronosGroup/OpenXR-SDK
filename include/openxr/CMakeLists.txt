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

# Generate OpenXR header files.

find_package(PythonInterp 3 REQUIRED)
set(XR_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
# Define openxrHeader target to generate the OpenXR header files.
# Other targets that need the OpenXR headers should use openxrHeader as a dependency.
add_custom_target(generate_openxr_header
    SOURCES ${XR_ROOT}/specification/registry/xr.xml
    DEPENDS 
        ${CMAKE_CURRENT_BINARY_DIR}/openxr_platform_defines.h
        ${CMAKE_CURRENT_BINARY_DIR}/openxr.h
        ${CMAKE_CURRENT_BINARY_DIR}/openxr_platform.h
)

# Copy the openxr_platform_defines.h file and place it in the binary (build) directory.
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/openxr_platform_defines.h
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/openxr_platform_defines.h ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/openxr_platform_defines.h
    COMMENT "Copying ${CMAKE_CURRENT_SOURCE_DIR}/openxr_platform_defines.h to ${CMAKE_CURRENT_BINARY_DIR}"
)

# Generate the openxr_platform.h file and place it in the binary (build) directory.
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/openxr_platform.h
    COMMAND ${PYTHON_EXECUTABLE} ${XR_ROOT}/specification/scripts/genxr.py
        -registry ${XR_ROOT}/specification/registry/xr.xml
        -o ${CMAKE_CURRENT_BINARY_DIR} openxr_platform.h
    DEPENDS
        ${XR_ROOT}/specification/scripts/genxr.py
        ${XR_ROOT}/specification/registry/xr.xml
    COMMENT "Generating ${CMAKE_CURRENT_BINARY_DIR}/openxr_platform.h"
)

# Generate the openxr.h file and place it in the binary (build) directory.
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/openxr.h
    COMMAND ${PYTHON_EXECUTABLE} ${XR_ROOT}/specification/scripts/genxr.py
        -registry ${XR_ROOT}/specification/registry/xr.xml
        -o ${CMAKE_CURRENT_BINARY_DIR} openxr.h
    DEPENDS
        ${XR_ROOT}/specification/scripts/genxr.py
        ${XR_ROOT}/specification/registry/xr.xml
    COMMENT "Generating ${CMAKE_CURRENT_BINARY_DIR}/openxr.h"
)

set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/openxr.h
    ${CMAKE_CURRENT_BINARY_DIR}/openxr_platform.h
    PROPERTIES GENERATED TRUE
)

set(INSTALL_HEADERS 
    ${CMAKE_CURRENT_BINARY_DIR}/openxr_platform_defines.h
    ${CMAKE_CURRENT_BINARY_DIR}/openxr.h
    ${CMAKE_CURRENT_BINARY_DIR}/openxr_platform.h)

INSTALL(FILES ${INSTALL_HEADERS} 
    DESTINATION include/openxr
)