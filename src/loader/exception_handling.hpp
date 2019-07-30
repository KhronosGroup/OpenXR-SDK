// Copyright (c) 2019 The Khronos Group Inc.
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
// Author: Ryan Pavlik <ryan.pavlik@collabora.com
//
// Provides protection for C ABI functions if standard library functions may throw.

#pragma once

#if !defined(OPENXR_NON_CMAKE_BUILD)
#include "common_cmake_config.h"
#endif  // !defined(OPENXR_NON_CMAKE_BUILD)

#ifdef XRLOADER_ENABLE_EXCEPTION_HANDLING
#include <stdexcept>
#define XRLOADER_ABI_TRY try
#define XRLOADER_ABI_CATCH_BAD_ALLOC_OOM                               \
    catch (const std::bad_alloc&) {                                    \
        LoaderLogger::LogErrorMessage("", "failed allocating memory"); \
        return XR_ERROR_OUT_OF_MEMORY;                                 \
    }
#define XRLOADER_ABI_CATCH_FALLBACK                                                     \
    catch (const std::exception& e) {                                                   \
        LoaderLogger::LogErrorMessage("", "Unknown failure: " + std::string(e.what())); \
        return XR_ERROR_RUNTIME_FAILURE;                                                \
    }                                                                                   \
    catch (...) {                                                                       \
        LoaderLogger::LogErrorMessage("", "Unknown failure");                           \
        return XR_ERROR_RUNTIME_FAILURE;                                                \
    }

#else
#define XRLOADER_ABI_TRY
#define XRLOADER_ABI_CATCH_BAD_ALLOC_OOM
#define XRLOADER_ABI_CATCH_FALLBACK
#endif
