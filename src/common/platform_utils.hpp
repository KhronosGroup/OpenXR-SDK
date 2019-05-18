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

// Prefix for the Apple global runtime JSON file name
static const std::string rt_dir_prefix = "/usr/local/share/openxr/";
static const std::string rt_filename = "/active_runtime.json";

static inline bool PlatformGetGlobalRuntimeFileName(uint16_t major_version, std::string &file_name) {
    file_name = rt_dir_prefix;
    file_name += std::to_string(major_version);
    file_name += rt_filename;
    return true;
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

#endif
