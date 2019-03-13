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

#include "gfxwrapper_opengl.h"  // for GL Extensions
#include "loader_test_utils.hpp"

#define XR_USE_GRAPHICS_API_OPENGL 1
#include "xr_dependencies.h"
#include <openxr/openxr.h>

#include <stdio.h>
#include <stdlib.h>

#if defined(XR_OS_WINDOWS)

bool LoaderTestSetEnvironmentVariable(const std::string &variable, const std::string &value) {
    return TRUE == SetEnvironmentVariable(variable.c_str(), value.c_str());
}

bool LoaderTestGetEnvironmentVariable(const std::string &variable, std::string &value) {
    char buf_data[4096];
    uint32_t num_chars = GetEnvironmentVariable(variable.c_str(), buf_data, 4095);
    if (0 == num_chars) {
        return false;
    }
    buf_data[num_chars - 1] = '\0';
    value = buf_data;
    return true;
}

bool LoaderTestUnsetEnvironmentVariable(const std::string &variable) {
    return TRUE == SetEnvironmentVariable(variable.c_str(), "");
}

#elif defined(XR_OS_LINUX)

bool LoaderTestSetEnvironmentVariable(const std::string &variable, const std::string &value) {
    if (0 == setenv(variable.c_str(), value.c_str(), 1)) {
        return true;
    }
    return false;
}

bool LoaderTestGetEnvironmentVariable(const std::string &variable, std::string &value) {
    char *returned = getenv(variable.c_str());
    if (nullptr == returned) {
        return false;
    }
    value = returned;
    return true;
}

bool LoaderTestUnsetEnvironmentVariable(const std::string &variable) {
    if (0 == unsetenv(variable.c_str())) {
        return true;
    }
    return false;
}

#elif defined(XR_OS_APPLE)

bool LoaderTestSetEnvironmentVariable(const std::string &variable, const std::string &value) {
    if (0 == setenv(variable.c_str(), value.c_str(), 1)) {
        return true;
    }
    return false;
}

bool LoaderTestGetEnvironmentVariable(const std::string &variable, std::string &value) {
    char *returned = getenv(variable.c_str());
    if (nullptr == returned) {
        return false;
    }
    value = returned;
    return true;
}

bool LoaderTestUnsetEnvironmentVariable(const std::string &variable) {
    if (0 == unsetenv(variable.c_str())) {
        return true;
    }
    return false;
}

#else

#error "Unsupported platform"

#endif