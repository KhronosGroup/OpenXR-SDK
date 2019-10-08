// Copyright (c) 2017-2019 The Khronos Group Inc.
// Copyright (c) 2017-2019 Valve Corporation
// Copyright (c) 2017-2019 LunarG, Inc.
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
// Author: Ryan Pavlik <ryan.pavlik@collabora.com>
//

#pragma once

#include "loader_logger.hpp"

#include <openxr/openxr.h>

#include <memory>

//! Standard Error logger, on by default. Disabled with environment variable XR_LOADER_DEBUG = "none".
std::unique_ptr<LoaderLogRecorder> MakeStdErrLoaderLogRecorder(void* user_data);

//! Standard Output logger used with XR_LOADER_DEBUG environment variable.
std::unique_ptr<LoaderLogRecorder> MakeStdOutLoaderLogRecorder(void* user_data, XrLoaderLogMessageSeverityFlags flags);

// Debug Utils logger used with XR_EXT_debug_utils
std::unique_ptr<LoaderLogRecorder> MakeDebugUtilsLoaderLogRecorder(const XrDebugUtilsMessengerCreateInfoEXT* create_info,
                                                                   XrDebugUtilsMessengerEXT debug_messenger);

#if _WIN32
//! Win32 debugger output
std::unique_ptr<LoaderLogRecorder> MakeDebuggerLoaderLogRecorder(void* user_data);
#endif

// TODO: Add other Derived classes:
//  - FileLoaderLogRecorder     - During/after xrCreateInstance
//  - PipeLoaderLogRecorder?    - During/after xrCreateInstance
