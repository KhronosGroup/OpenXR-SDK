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

#include <string>

#if defined(XR_OS_LINUX) || defined(XR_OS_APPLE)
#include <unistd.h>

#define TEST_PATH_SEPARATOR ':'
#define TEST_DIRECTORY_SYMBOL '/'
#elif defined(XR_OS_WINDOWS)
#define TEST_PATH_SEPARATOR ';'
#define TEST_DIRECTORY_SYMBOL '\\'
#else
#error "Unsupported platform"
#endif

// Utility functions for setting and reading the environment variables.
bool LoaderTestSetEnvironmentVariable(const std::string& variable, const std::string& value);
bool LoaderTestGetEnvironmentVariable(const std::string& variable, std::string& value);
bool LoaderTestUnsetEnvironmentVariable(const std::string& variable);
