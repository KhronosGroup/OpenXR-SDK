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

find_package(PythonInterp 3 REQUIRED)

# Custom target for amalgamated jsoncpp files.
add_custom_target(jsoncppAmalgamatedFiles
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/dist/jsoncpp.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/dist/json/json.h
        ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/dist/json/json-forwards.h
)
set_target_properties(jsoncppAmalgamatedFiles PROPERTIES FOLDER ${CODEGEN_FOLDER})

# Command to satisfy the amalgamated file dependencies
add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/dist/jsoncpp.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/dist/json/json.h
        ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/dist/json/json-forwards.h
    COMMAND ${PYTHON_EXECUTABLE} amalgamate.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp
    DEPENDS 
        ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/amalgamate.py
        ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/src/lib_json/json_reader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/include/json/json.h
        ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/include/json/reader.h
    COMMENT "Running amalgamate - ${PYTHON_EXECUTABLE} amalgamate.py in ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp"
)

set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/dist/jsoncpp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/dist/json/json.h
    ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/dist/json/json-forwards.h
    PROPERTIES GENERATED TRUE
)
