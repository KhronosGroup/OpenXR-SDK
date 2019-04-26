// Copyright (c) 2017 The Khronos Group Inc.
// Copyright (c) 2017 Valve Corporation
// Copyright (c) 2017 LunarG, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: Mark Young <marky@lunarg.com>
//

#pragma once

#include "xr_dependencies.h"

#if defined(XR_OS_LINUX)
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#endif

// Structure used to track the global runtime file information
struct GlobalRuntimeFile {
    bool valid;
    bool locked;
    std::string file_name;
    std::string runtime_name;
#if defined(XR_OS_LINUX)
    int32_t file_descriptor;
#elif defined(XR_OS_APPLE)
    // TBD
#elif defined(XR_OS_WINDOWS)
    HANDLE file_handle;
#else
    // TBD
#endif
};

// This is a CMake generated file with #defines for any functions/includes
// that it found present.  This is currently necessary to properly determine
// if secure_getenv or __secure_getenv are present
#if !defined(OPENXR_NON_CMAKE_BUILD)
#include "common_cmake_config.h"
#endif  // !defined(OPENXR_NON_CMAKE_BUILD)

// Environment variables
#if defined(XR_OS_LINUX)

static inline char* PlatformUtilsGetEnv(const char* name) { return getenv(name); }

static inline char* PlatformUtilsGetSecureEnv(const char* name) {
#ifdef HAVE_SECURE_GETENV
    return secure_getenv(name);
#elif defined(HAVE___SECURE_GETENV)
    return __secure_getenv(name);
#else
#pragma message(                                                    \
    "Warning:  Falling back to non-secure getenv for environmental" \
    "lookups!  Consider updating to a different libc.")

    return PlatformUtilsGetEnv(name);
#endif
}

static inline void PlatformUtilsFreeEnv(char* val) {
    // No freeing of memory necessary for Linux, but we should at least touch
    // the val and inst pointers to get rid of compiler warnings.
    (void)val;
}

// Prefix for the Linux/Apple global runtime JSON file name
static const std::string rt_dir_prefix = "/usr/local/share/openxr/";
static const std::string rt_filename = "/active_runtime.json";

static inline bool PlatformGetGlobalRuntimeFileName(uint16_t major_version, std::string& file_name) {
    file_name = rt_dir_prefix;
    file_name += std::to_string(major_version);
    file_name += rt_filename;
    return true;
}

static inline bool PlatformLockGlobalRuntimeFile(uint16_t major_version, GlobalRuntimeFile& global_rt_file) {
    bool success = false;
    try {
        global_rt_file = {};
        // Get the filename
        if (PlatformGetGlobalRuntimeFileName(major_version, global_rt_file.file_name)) {
            // Initialize the global runtime file info
            // Open a file descriptor to the file.
            global_rt_file.file_descriptor = open(global_rt_file.file_name.c_str(), O_RDWR);
            if (0 <= global_rt_file.file_descriptor) {
                flock fcntl_lock = {};
                global_rt_file.valid = true;

                // Test to see if a lock's already present
                fcntl_lock.l_type = F_WRLCK;
                fcntl_lock.l_whence = SEEK_SET;
                fcntl_lock.l_start = 0;
                fcntl_lock.l_len = 0;
                if (-1 == fcntl(global_rt_file.file_descriptor, F_OFD_GETLK, &fcntl_lock)) {
                    std::string error_msg = "fcntl(querylock | ";
                    error_msg += global_rt_file.file_name;
                    error_msg += ", FD: ";
                    error_msg += std::to_string(global_rt_file.file_descriptor);
                    error_msg += ", ErrorNo: ";
                    error_msg += std::to_string(errno);
                    error_msg += ")";
                    perror(error_msg.c_str());
                } else if (fcntl_lock.l_type == F_RDLCK || fcntl_lock.l_type == F_WRLCK) {
                    std::cerr << "Process " << std::to_string(fcntl_lock.l_pid) << " has a lock on " << global_rt_file.file_name
                              << " already!" << std::endl;
                } else {
                    // Lock the entire file for writing
                    fcntl_lock.l_type = F_WRLCK;
                    fcntl_lock.l_whence = SEEK_SET;
                    fcntl_lock.l_start = 0;
                    fcntl_lock.l_len = 0;
                    if (-1 == fcntl(global_rt_file.file_descriptor, F_OFD_SETLK, &fcntl_lock)) {
                        std::string error_msg = "fcntl(lock | ";
                        error_msg += global_rt_file.file_name;
                        error_msg += ", FD: ";
                        error_msg += std::to_string(global_rt_file.file_descriptor);
                        error_msg += ", ErrorNo: ";
                        error_msg += std::to_string(errno);
                        error_msg += ")";
                        perror(error_msg.c_str());
                    } else {
                        global_rt_file.locked = true;
                        success = true;
                    }
                }
                // Get the name of the runtime from the JSON (just crudely)
                if (success) {
                    char buffer[4096];
                    std::string contents = "";
                    while (read(global_rt_file.file_descriptor, &buffer, 4095) > 0) {
                        contents += buffer;
                    }
                    std::size_t name_index = contents.find("\"name\"");
                    if (std::string::npos != name_index) {
                        std::size_t begin_quote_index = contents.find("\"", name_index + 6);
                        std::size_t end_quote_index = contents.find("\"", begin_quote_index + 1);
                        if (std::string::npos != begin_quote_index && std::string::npos != end_quote_index) {
                            global_rt_file.runtime_name =
                                contents.substr(begin_quote_index + 1, end_quote_index - begin_quote_index - 1);
                        }
                    }
                }
            } else {
                std::string error_msg = "open(";
                error_msg += global_rt_file.file_name;
                error_msg += ")";
                perror(error_msg.c_str());
            }
        }
    } catch (...) {
    }
    // Cleanup on failure
    if (!success) {
        if (global_rt_file.valid) {
            close(global_rt_file.file_descriptor);
            global_rt_file.valid = false;
            global_rt_file.file_name = "";
        }
    }
    return success;
}

static inline bool PlatformUnlockGlobalRuntimeFile(GlobalRuntimeFile& global_rt_file) {
    bool success = false;
    try {
        if (global_rt_file.valid && global_rt_file.locked) {
            flock fcntl_lock = {};
            // Unlock the entire file for writing
            fcntl_lock.l_type = F_UNLCK;
            fcntl_lock.l_whence = SEEK_SET;
            fcntl_lock.l_start = 0;
            fcntl_lock.l_len = 0;
            if (-1 == fcntl(global_rt_file.file_descriptor, F_OFD_SETLK, &fcntl_lock)) {
                std::string error_msg = "fcntl(unlock | ";
                error_msg += global_rt_file.file_name;
                error_msg += ", FD: ";
                error_msg += std::to_string(global_rt_file.file_descriptor);
                error_msg += ", ErrorNo: ";
                error_msg += std::to_string(errno);
                error_msg += ")";
                perror(error_msg.c_str());
            } else {
                global_rt_file.locked = false;
                success = true;
            }
            if (-1 == close(global_rt_file.file_descriptor)) {
                std::string error_msg = "close(";
                error_msg += global_rt_file.file_name;
                error_msg += ", FD: ";
                error_msg += std::to_string(global_rt_file.file_descriptor);
                error_msg += ", ErrorNo: ";
                error_msg += std::to_string(errno);
                error_msg += ")";
                perror(error_msg.c_str());
                success = false;
            } else {
                global_rt_file.file_descriptor = -1;
                global_rt_file.valid = false;
                global_rt_file.file_name = "";
            }
        }
    } catch (...) {
    }
    return success;
}

#elif defined(XR_OS_APPLE)

static inline char *PlatformUtilsGetEnv(const char *name) { return getenv(name); }

static inline char *PlatformUtilsGetSecureEnv(const char *name) {
#ifdef HAVE_SECURE_GETENV
    return secure_getenv(name);
#elif defined(HAVE___SECURE_GETENV)
    return __secure_getenv(name);
#else
#pragma message(                                                    \
    "Warning:  Falling back to non-secure getenv for environmental" \
    "lookups!  Consider updating to a different libc.")

    return PlatformUtilsGetEnv(name);
#endif
}

static inline void PlatformUtilsFreeEnv(char *val) {
    // No freeing of memory necessary for Linux, but we should at least touch
    // the val and inst pointers to get rid of compiler warnings.
    (void)val;
}

// Prefix for the Linux/Apple global runtime JSON file name
static const std::string rt_dir_prefix = "/usr/local/share/openxr/";
static const std::string rt_filename = "/active_runtime.json";

static inline bool PlatformGetGlobalRuntimeFileName(uint16_t major_version, std::string &file_name) {
    file_name = rt_dir_prefix;
    file_name += std::to_string(major_version);
    file_name += rt_filename;
    return true;
}

static inline bool PlatformLockGlobalRuntimeFile(uint16_t major_version, GlobalRuntimeFile &global_rt_file) {
    // Stub func
    (void)major_version;
    (void)global_rt_file;
    return false;
}

static inline bool PlatformUnlockGlobalRuntimeFile(GlobalRuntimeFile &global_rt_file) {
    // Stub func
    (void)global_rt_file;
    return false;
}

#elif defined(XR_OS_WINDOWS)

static inline char *PlatformUtilsGetEnv(const char *name) {
    char *retVal;
    DWORD valSize;

    valSize = GetEnvironmentVariableA(name, nullptr, 0);

    // valSize DOES include the null terminator, so for any set variable
    // will always be at least 1. If it's 0, the variable wasn't set.
    if (valSize == 0) {
        return nullptr;
    }

    // Allocate the space necessary for the registry entry
    retVal = new char[valSize + 1];
    GetEnvironmentVariableA(name, retVal, valSize);
    return retVal;
}

static inline char *PlatformUtilsGetSecureEnv(const char *name) {
    // No secure version for Windows as far as I know
    return PlatformUtilsGetEnv(name);
}

static inline void PlatformUtilsFreeEnv(char *val) {
    if (nullptr != val) {
        delete[] val;
        val = nullptr;
    }
}

// Prefix for the Windows global runtime JSON file name
static const std::string rt_file_folder = "\\ProgramData\\Khronos\\OpenXR\\";
static const std::string rt_file_prefix = "\\openxr_runtime_";

static inline bool PlatformGetGlobalRuntimeFileName(uint16_t major_version, std::string &file_name) {
    bool ret_value = false;
    try {
        char *sys_drive = PlatformUtilsGetSecureEnv("SystemDrive");
        if (nullptr != sys_drive) {
            file_name = sys_drive;
            PlatformUtilsFreeEnv(sys_drive);
            file_name += rt_file_folder;
            if (sizeof(void *) == 8) {
                file_name += "64";
            } else {
                file_name += "32";
            }
            file_name += rt_file_prefix;
            file_name += std::to_string(major_version);
            file_name += ".json";
            ret_value = true;
        }
    } catch (...) {
    }
    return ret_value;
}

static inline bool PlatformLockGlobalRuntimeFile(uint16_t major_version, GlobalRuntimeFile &global_rt_file) {
    bool ret_value = false;
    std::string rt_filename;
    if (PlatformGetGlobalRuntimeFileName(major_version, rt_filename)) {
        HANDLE file_handle = CreateFileA(rt_filename.c_str(), (GENERIC_READ | GENERIC_WRITE), FILE_SHARE_READ, nullptr,
                                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file_handle != INVALID_HANDLE_VALUE) {
            global_rt_file.valid = true;
            global_rt_file.locked = true;
            global_rt_file.file_handle = file_handle;
            ret_value = true;
        }
        // Get the name of the runtime from the JSON (just crudely)
        if (ret_value) {
            char buffer[4096];
            DWORD num_bytes_read = 0;
            std::string contents = "";
            while (FALSE != ReadFile(file_handle, buffer, 4095, &num_bytes_read, nullptr)) {
                if (num_bytes_read == 0) {
                    break;
                }
                contents += buffer;
            }
            std::size_t name_index = contents.find("\"name\"");
            if (std::string::npos != name_index) {
                std::size_t begin_quote_index = contents.find("\"", name_index + 6);
                std::size_t end_quote_index = contents.find("\"", begin_quote_index + 1);
                if (std::string::npos != begin_quote_index && std::string::npos != end_quote_index) {
                    global_rt_file.runtime_name = contents.substr(begin_quote_index + 1, end_quote_index - begin_quote_index - 1);
                }
            }
        }
    }
    return ret_value;
}

static inline bool PlatformUnlockGlobalRuntimeFile(GlobalRuntimeFile &global_rt_file) {
    if (global_rt_file.valid && global_rt_file.locked && CloseHandle(global_rt_file.file_handle)) {
        global_rt_file.valid = false;
        global_rt_file.locked = false;
        global_rt_file.file_handle = nullptr;
        return true;
    }
    return false;
}

#else  // Not Linux or Windows

static inline char *PlatformUtilsGetEnv(const char *name) {
    // Stub func
    (void)name;
    return nullptr;
}

static inline char *PlatformUtilsGetSecureEnv(const char *name) {
    // Stub func
    (void)name;
    return nullptr;
}

static inline void PlatformUtilsFreeEnv(char *val) {
    // Stub func
    (void)val;
}

static inline bool PlatformGetGlobalRuntimeFileName(uint16_t major_version, std::string &file_name) {
    // Stub func
    (void)major_version;
    (void)file_name;
    return false;
}

static inline bool PlatformLockGlobalRuntimeFile(uint16_t major_version, GlobalRuntimeFile &global_rt_file) {
    // Stub func
    (void)major_version;
    (void)global_rt_file;
    return false;
}

static inline bool PlatformUnlockGlobalRuntimeFile(GlobalRuntimeFile &global_rt_file) {
    // Stub func
    (void)global_rt_file;
    return false;
}

#endif
