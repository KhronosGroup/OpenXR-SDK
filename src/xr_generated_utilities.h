// *********** THIS FILE IS GENERATED - DO NOT EDIT ***********
//     See utility_source_generator.py for modifications
// ************************************************************

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
// Author: Mark Young <marky@lunarg.com>
//

#pragma once

#include <openxr/openxr.h>


#ifdef __cplusplus
extern "C" { 
#endif

XrResult GeneratedXrUtilitiesResultToString(XrResult result,
                                            char buffer[XR_MAX_RESULT_STRING_SIZE]);
XrResult GeneratedXrUtilitiesStructureTypeToString(XrStructureType struct_type,
                                                   char buffer[XR_MAX_STRUCTURE_NAME_SIZE]);
#ifdef __cplusplus
} // extern "C"
#endif

// Current API version of the utililties
#define XR_UTILITIES_API_VERSION XR_CURRENT_API_VERSION

