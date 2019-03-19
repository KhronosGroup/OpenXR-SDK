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

add_executable(list
    list.cpp
)
add_dependencies(list
    generate_openxr_header
)
target_include_directories(list
    PRIVATE ${CMAKE_SOURCE_DIR}/src
    PRIVATE ${CMAKE_SOURCE_DIR}/src/common
    PRIVATE ${CMAKE_BINARY_DIR}/include
    PRIVATE ${CMAKE_SOURCE_DIR}/external/include
)
if(VulkanHeaders_FOUND)
    target_include_directories(list
        PRIVATE ${VulkanHeaders_INCLUDE_DIRS}
    )
endif()

if(WIN32)
    set(OPENXR_LIB openxr_loader-${MAJOR}_${MINOR})
else()
    set(OPENXR_LIB openxr_loader)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_options(list PRIVATE /Zc:wchar_t /Zc:forScope /W4 /WX)
    target_link_libraries(list ${OPENXR_LIB} opengl32)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_options(list PRIVATE -Wall)
    target_link_libraries(list ${OPENXR_LIB} m pthread GL ${XLIB_LIBRARIES} ${XCB_LIBRARIES} ${WAYLAND_LIBRARIES})
endif()

set_target_properties(list
    PROPERTIES FOLDER tests_loader
)

