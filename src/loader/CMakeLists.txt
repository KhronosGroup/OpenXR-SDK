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

#set (CMAKE_VERBOSE_MAKEFILE 1)

include(GNUInstallDirs)

# Use this feature for Windows to automatically generate an exports file for the DLL.
# See https://blog.kitware.com/create-dlls-on-windows-without-declspec-using-new-cmake-export-all-feature/
include(GenerateExportHeader)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS false)

find_package(PythonInterp 3 REQUIRED)

set(LOADER_NAME openxr_loader)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(FALLBACK_CONFIG_DIRS "/etc/xdg" CACHE STRING
        "Search path to use when XDG_CONFIG_DIRS is unset or empty or the current process is SUID/SGID. Default is freedesktop compliant.")
    set(FALLBACK_DATA_DIRS "/usr/local/share:/usr/share" CACHE STRING
        "Search path to use when XDG_DATA_DIRS is unset or empty or the current process is SUID/SGID. Default is freedesktop compliant.")
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(openxr_loader_RESOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/loader.rc)
    set(LOADER_NAME ${LOADER_NAME}-${MAJOR}_${MINOR})
endif()

# List of all files externally generated outside of the loader that the loader
# needs to build with.
SET(LOADER_EXTERNAL_GEN_FILES
    ${CMAKE_SOURCE_DIR}/src/external/jsoncpp/dist/jsoncpp.cpp
    ${CMAKE_BINARY_DIR}/src/xr_generated_dispatch_table.c
    ${CMAKE_BINARY_DIR}/src/xr_generated_utilities.c
)
run_xr_xml_generate(loader_source_generator.py xr_generated_loader.hpp)
run_xr_xml_generate(loader_source_generator.py xr_generated_loader.cpp)

if(DYNAMIC_LOADER)
    add_definitions(-DXRAPI_DLL_EXPORT)
    set(LIBRARY_TYPE SHARED)
else() # build static lib
    set(LIBRARY_TYPE STATIC)
endif()

add_library(${LOADER_NAME} ${LIBRARY_TYPE}
    api_layer_interface.cpp
    api_layer_interface.hpp
    loader_core.cpp
    loader_instance.cpp
    loader_instance.hpp
    loader_logger.cpp
    loader_logger.hpp
    loader_logger_recorders.cpp
    loader_logger_recorders.hpp
    manifest_file.cpp
    manifest_file.hpp
    runtime_interface.cpp
    runtime_interface.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/xr_generated_loader.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/xr_generated_loader.cpp
    ${CMAKE_SOURCE_DIR}/src/common/filesystem_utils.cpp
    ${CMAKE_SOURCE_DIR}/src/common/filesystem_utils.hpp
    ${CMAKE_SOURCE_DIR}/src/common/hex_and_handles.cpp
    ${CMAKE_SOURCE_DIR}/src/common/hex_and_handles.h
    ${LOADER_EXTERNAL_GEN_FILES}
    ${openxr_loader_RESOURCE_FILE}
)
set_target_properties(${LOADER_NAME} PROPERTIES FOLDER ${LOADER_FOLDER})

set_source_files_properties(
    ${LOADER_EXTERNAL_GEN_FILES}
    PROPERTIES GENERATED TRUE
)
add_dependencies(${LOADER_NAME}
    generate_openxr_header
    xr_global_generated_files
    jsoncppAmalgamatedFiles
)
target_include_directories(${LOADER_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE ${CMAKE_BINARY_DIR}/include
    PRIVATE ${CMAKE_BINARY_DIR}/src
    PRIVATE ${CMAKE_SOURCE_DIR}/src/common
    PRIVATE ${CMAKE_SOURCE_DIR}/src/external/jsoncpp/dist
)
if(VulkanHeaders_FOUND)
    target_include_directories(${LOADER_NAME}
        PRIVATE ${Vulkan_INCLUDE_DIRS}
    )
endif()

target_compile_definitions(${LOADER_NAME}
    PRIVATE API_NAME="OpenXR"
)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(${LOADER_NAME}
        PRIVATE FALLBACK_CONFIG_DIRS="${FALLBACK_CONFIG_DIRS}"
        PRIVATE FALLBACK_DATA_DIRS="${FALLBACK_DATA_DIRS}"
        PRIVATE SYSCONFDIR="${CMAKE_INSTALL_FULL_SYSCONFDIR}"
    )
    if(NOT(CMAKE_INSTALL_FULL_SYSCONFDIR STREQUAL "/etc"))
        target_compile_definitions(openxr_loader PRIVATE EXTRASYSCONFDIR="/etc")
    endif()

    set_target_properties(${LOADER_NAME} PROPERTIES SOVERSION "${MAJOR}" VERSION "${MAJOR}.${MINOR}.${PATCH}")
    target_link_libraries(${LOADER_NAME} -lstdc++fs -ldl -lpthread -lm)

    add_custom_target(lib${LOADER_NAME}.so.${MAJOR}.${MINOR} ALL
        COMMAND ${CMAKE_COMMAND} -E create_symlink lib${LOADER_NAME}.so.${MAJOR}.${MINOR}.${PATCH} lib${LOADER_NAME}.so.${MAJOR}.${MINOR})

    install(TARGETS ${LOADER_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    set(XR_API_VERSION "${MAJOR}.${MINOR}")
    configure_file("openxr.pc.in" "openxr.pc" @ONLY)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/openxr.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    foreach(configuration in CMAKE_C_FLAGS_DEBUG
                             CMAKE_C_FLAGS_RELEASE
                             CMAKE_C_FLAGS_RELWITHDEBINFO
                             CMAKE_CXX_FLAGS_DEBUG
                             CMAKE_CXX_FLAGS_RELEASE
                             CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        # If building DLLs, force static CRT linkage
        if(DYNAMIC_LOADER)
            if (${configuration} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${configuration} "${${configuration}}")
            endif()
        else()  # Otherwise for static libs, link the CRT dynamically
            if (${configuration} MATCHES "/MT")
                string(REGEX REPLACE "/MT" "/MD" ${configuration} "${${configuration}}")
            endif()
        endif()
    endforeach()

    target_link_libraries(${LOADER_NAME} shlwapi)
    target_compile_options(${LOADER_NAME} PRIVATE)
    generate_export_header(${LOADER_NAME})
    # set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS false)

    # Need to copy DLL to client directories so clients can easily load it.
    if( (DYNAMIC_LOADER) AND (CMAKE_GENERATOR MATCHES "^Visual Studio.*") )
        file( TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/${LOADER_NAME}.dll COPY_SRC_PATH )
        # file( TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/../tests/triangle/$<CONFIGURATION>/ COPY_DST_TEST_PATH )
        file( TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/../tests/hello_xr/$<CONFIGURATION>/ COPY_DST_TEST_PATH )
        file( TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/../tests/loader_test/$<CONFIGURATION>/ COPY_DST_LOADER_TEST_PATH )
        add_custom_command( TARGET ${LOADER_NAME} POST_BUILD
            COMMAND xcopy /Y /I ${COPY_SRC_PATH} ${COPY_DST_TEST_PATH}
            COMMAND xcopy /Y /I ${COPY_SRC_PATH} ${COPY_DST_LOADER_TEST_PATH}
        )
    endif()
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_compile_options(openxr_loader
        PRIVATE -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wpointer-arith
        PRIVATE -fno-strict-aliasing -fno-builtin-memcmp "$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>"
    )
    # For GCC version 7.1 or greater, we need to disable the implicit fallthrough warning since
    # there's no consistent way to satisfy all compilers until they all accept the C++17 standard
    if (CMAKE_COMPILER_IS_GNUCC AND NOT (CMAKE_CXX_COMPILER_VERSION LESS 7.1))
        target_compile_options(openxr_loader PRIVATE -Wimplicit-fallthrough=0)
    endif()
endif()

install(TARGETS ${LOADER_NAME} DESTINATION lib)

# Custom commands to build dependencies for above targets
