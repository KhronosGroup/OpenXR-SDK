// Copyright (c) 2017-2019 The Khronos Group Inc.
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
#include <string>

#if defined(XR_OS_LINUX)
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#endif

// This is a CMake generated file with #defines for any functions/includes
// that it found present and build-time configuration.
// If you don't have this file, on non-Windows you'll need to define
// one of HAVE_SECURE_GETENV or HAVE___SECURE_GETENV depending on which
// of secure_getenv or __secure_getenv are present
#ifdef OPENXR_HAVE_COMMON_CONFIG
#include "common_config.h"
#endif  // OPENXR_HAVE_COMMON_CONFIG

// Environment variables
#if defined(XR_OS_LINUX) || defined(XR_OS_APPLE)

namespace detail {

static inline char* ImplGetEnv(const char* name) { return getenv(name); }

static inline char* ImplGetSecureEnv(const char* name) {
#ifdef HAVE_SECURE_GETENV
    return secure_getenv(name);
#elif defined(HAVE___SECURE_GETENV)
    return __secure_getenv(name);
#else
#pragma message(                                                    \
    "Warning:  Falling back to non-secure getenv for environmental" \
    "lookups!  Consider updating to a different libc.")

    return ImplGetEnv(name);
#endif
}
}  // namespace detail

#endif  // defined(XR_OS_LINUX) || defined(XR_OS_APPLE)
#if defined(XR_OS_LINUX)

static inline std::string PlatformUtilsGetEnv(const char* name) {
    auto str = detail::ImplGetEnv(name);
    if (str == nullptr) {
        return {};
    }
    return str;
}

static inline std::string PlatformUtilsGetSecureEnv(const char* name) {
    auto str = detail::ImplGetSecureEnv(name);
    if (str == nullptr) {
        return {};
    }
    return str;
}

static inline bool PlatformUtilsGetEnvSet(const char* name) { return detail::ImplGetEnv(name) != nullptr; }

#elif defined(XR_OS_APPLE)

static inline std::string PlatformUtilsGetEnv(const char* name) {
    auto str = detail::ImplGetEnv(name);
    if (str == nullptr) {
        return {};
    }
    return str;
}

static inline std::string PlatformUtilsGetSecureEnv(const char* name) {
    auto str = detail::ImplGetSecureEnv(name);
    if (str == nullptr) {
        return {};
    }
    return str;
}

static inline bool PlatformUtilsGetEnvSet(const char* name) { return detail::ImplGetEnv(name) != nullptr; }

// Prefix for the Apple global runtime JSON file name
static const std::string rt_dir_prefix = "/usr/local/share/openxr/";
static const std::string rt_filename = "/active_runtime.json";

static inline bool PlatformGetGlobalRuntimeFileName(uint16_t major_version, std::string& file_name) {
    file_name = rt_dir_prefix;
    file_name += std::to_string(major_version);
    file_name += rt_filename;
    return true;
}

#elif defined(XR_OS_WINDOWS)

#if defined(_DEBUG)
inline void LogError(const std::string& error) { OutputDebugStringA(error.c_str()); }
#else
#define LogError(x)
#endif

inline std::wstring utf8_to_wide(const std::string& utf8Text) {
    if (utf8Text.empty()) {
        return {};
    }

    std::wstring wideText;
    const int wideLength = ::MultiByteToWideChar(CP_UTF8, 0, utf8Text.data(), (int)utf8Text.size(), nullptr, 0);
    if (wideLength == 0) {
        LogError("utf8_to_wide get size error: " + std::to_string(::GetLastError()));
        return {};
    }

    // MultiByteToWideChar returns number of chars of the input buffer, regardless of null terminitor
    wideText.resize(wideLength, 0);
    wchar_t* wideString = const_cast<wchar_t*>(wideText.data());  // mutable data() only exists in c++17
    const int length = ::MultiByteToWideChar(CP_UTF8, 0, utf8Text.data(), (int)utf8Text.size(), wideString, wideLength);
    if (length != wideLength) {
        LogError("utf8_to_wide convert string error: " + std::to_string(::GetLastError()));
        return {};
    }

    return wideText;
}

inline std::string wide_to_utf8(const std::wstring& wideText) {
    if (wideText.empty()) {
        return {};
    }

    std::string narrowText;
    int narrowLength = ::WideCharToMultiByte(CP_UTF8, 0, wideText.data(), (int)wideText.size(), nullptr, 0, nullptr, nullptr);
    if (narrowLength == 0) {
        LogError("wide_to_utf8 get size error: " + std::to_string(::GetLastError()));
        return {};
    }

    // WideCharToMultiByte returns number of chars of the input buffer, regardless of null terminitor
    narrowText.resize(narrowLength, 0);
    char* narrowString = const_cast<char*>(narrowText.data());  // mutable data() only exists in c++17
    const int length =
        ::WideCharToMultiByte(CP_UTF8, 0, wideText.data(), (int)wideText.size(), narrowString, narrowLength, nullptr, nullptr);
    if (length != narrowLength) {
        LogError("wide_to_utf8 convert string error: " + std::to_string(::GetLastError()));
        return {};
    }

    return narrowText;
}

static inline bool IsHighIntegrityLevel() {
    static bool isHighIntegrityLevel = ([] {
        HANDLE processToken;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_QUERY_SOURCE, &processToken)) {
            // Maximum possible size of SID_AND_ATTRIBUTES is maximum size of a SID + size of attributes DWORD.
            uint8_t mandatoryLabelBuffer[SECURITY_MAX_SID_SIZE + sizeof(DWORD)]{};
            DWORD bufferSize;
            if (GetTokenInformation(processToken, TokenIntegrityLevel, mandatoryLabelBuffer, sizeof(mandatoryLabelBuffer),
                                    &bufferSize) != 0) {
                const auto mandatoryLabel = reinterpret_cast<const TOKEN_MANDATORY_LABEL*>(mandatoryLabelBuffer);
                const DWORD subAuthorityCount = *GetSidSubAuthorityCount(mandatoryLabel->Label.Sid);
                const DWORD integrityLevel = *GetSidSubAuthority(mandatoryLabel->Label.Sid, subAuthorityCount - 1);
                return integrityLevel > SECURITY_MANDATORY_MEDIUM_RID;
            }

            CloseHandle(processToken);
        }

        return false;
    })();

    return isHighIntegrityLevel;
}

static inline bool PlatformUtilsGetEnvSet(const char* name) {
    const std::wstring wname = utf8_to_wide(name);
    const DWORD valSize = ::GetEnvironmentVariableW(wname.c_str(), nullptr, 0);
    // GetEnvironmentVariable returns 0 when environment variable does not exist
    return 0 != valSize;
}

static inline std::string PlatformUtilsGetEnv(const char* name) {
    const std::wstring wname = utf8_to_wide(name);
    const DWORD valSize = ::GetEnvironmentVariableW(wname.c_str(), nullptr, 0);
    // GetEnvironmentVariable returns 0 when environment variable does not exist
    if (valSize == 0) {
        return {};
    }

    // GetEnvironmentVariable returns size including null terminator for "query size" call.
    std::wstring wValue(valSize, 0);
    wchar_t* wValueData = const_cast<wchar_t*>(wValue.data());  // mutable data() only exists in c++17

    // GetEnvironmentVariable returns string length, excluding null terminator for "get value" call.
    const int length = ::GetEnvironmentVariableW(wname.c_str(), wValueData, (DWORD)wValue.size());
    if (!length) {
        LogError("GetEnvironmentVariable get value error: " + std::to_string(::GetLastError()));
        return {};
    }

    return wide_to_utf8(wValue);
}

static inline std::string PlatformUtilsGetSecureEnv(const char* name) {
    // Do not allow high integrity processes to act on data that can be controlled by medium integrity processes.
    if (IsHighIntegrityLevel()) {
        return nullptr;
    }

    // No secure version for Windows so the above integrity check is needed.
    return PlatformUtilsGetEnv(name);
}

#else  // Not Linux or Windows

static inline std::string PlatformUtilsGetEnv(const char * /* name */) {
    // Stub func
    return {};
}

static inline char *PlatformUtilsGetSecureEnv(const char * /* name */) {
    // Stub func
    return {};
}

static inline void PlatformUtilsFreeEnv(char * /* val */) {
    // Stub func
}

static inline bool PlatformGetGlobalRuntimeFileName(uint16_t /* major_version */, std::string const & /* file_name */) {
    // Stub func
    return false;
}

#endif
