// Copyright (c) 2017-2019 The Khronos Group Inc.
// Copyright (c) 2017-2019 Valve Corporation
// Copyright (c) 2017-2019 LunarG, Inc.
// Copyright (c) 2019 Collabora, Ltd.
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

/*!
 * @file
 *
 * Implementations of the include-requiring hex conversion functions.
 */

#include "hex_and_handles.h"

#include <sstream>
#include <iomanip>

std::string Uint64ToHexString(uint64_t val) {
    std::ostringstream oss;
    oss << "0x";
    oss << std::hex << std::setw(sizeof(val) * 2) << std::setfill('0') << val;
    return oss.str();
}

std::string Uint32ToHexString(uint32_t val) {
    std::ostringstream oss;
    oss << "0x";
    oss << std::hex << std::setw(sizeof(val) * 2) << std::setfill('0') << val;
    return oss.str();
}
