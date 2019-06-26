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
 * Some utilities, primarily for working with OpenXR handles in a generic way.
 *
 * Most are trivial and inlined by default, but a few involve some non-trivial standard headers:
 * the various `...ToHexString`functions.
 * If you want those, you have two ways of using this header:
 *
 * - Out-of-line implementation: Just include this header where you need it, then
 *   in one .cpp file, define XR_UTILS_INCLUDE_IMPLEMENTATION before you include it
 *   to compile the implementation in that file.
 * - Inline implementation: Increases the number of includes, but you don't have to
 *   worry about how many files have defined XR_UTILS_INCLUDE_IMPLEMENTATION.
 *   Define XR_UTILS_INLINE_IMPLEMENTATION in every file before including this.
 */

#pragma once

#include <openxr/openxr.h>

#include <string>

#if defined(XR_UTILS_INLINE_IMPLEMENTATION) && defined(XR_UTILS_INCLUDE_IMPLEMENTATION)
#error "Cannot define both XR_UTILS_INLINE_IMPLEMENTATION and XR_UTILS_INCLUDE_IMPLEMENTATION"
#endif

#if XR_PTR_SIZE == 8
/// Convert a handle into a same-sized integer.
template <typename T>
static inline uint64_t MakeHandleGeneric(T handle) {
    return reinterpret_cast<uint64_t>(handle);
}

/// Treat an integer as a handle
template <typename T>
static inline T& TreatIntegerAsHandle(uint64_t& handle) {
    return reinterpret_cast<T&>(handle);
}

/// @overload
template <typename T>
static inline T const& TreatIntegerAsHandle(uint64_t const& handle) {
    return reinterpret_cast<T const&>(handle);
}

/// Does a correctly-sized integer represent a null handle?
static inline bool IsIntegerNullHandle(uint64_t handle) { return XR_NULL_HANDLE == reinterpret_cast<void*>(handle); }

#else

/// Convert a handle into a same-sized integer: no-op on 32-bit systems
static inline uint64_t MakeHandleGeneric(uint64_t handle) { return handle; }

/// Treat an integer as a handle: no-op on 32-bit systems
template <typename T>
static inline T& TreatIntegerAsHandle(uint64_t& handle) {
    return handle;
}

/// @overload
template <typename T>
static inline T const& TreatIntegerAsHandle(uint64_t const& handle) {
    return handle;
}

/// Does a correctly-sized integer represent a null handle?
static inline bool IsIntegerNullHandle(uint64_t handle) { return XR_NULL_HANDLE == handle; }

#endif

#ifdef XR_UTILS_INLINE_IMPLEMENTATION
#define XR_UTILS_STATIC_INLINE static inline
#else
#define XR_UTILS_STATIC_INLINE
#endif

/// Turns a uint64_t into a string formatted as hex.
///
/// The core of the HandleToHexString implementation is in here.
XR_UTILS_STATIC_INLINE std::string Uint64ToHexString(uint64_t val);

/// Turns a uint32_t into a string formatted as hex.
XR_UTILS_STATIC_INLINE std::string Uint32ToHexString(uint32_t val);

/// Turns an OpenXR handle into a string formatted as hex.
template <typename T>
static inline std::string HandleToHexString(T handle) {
    return Uint64ToHexString(MakeHandleGeneric(handle));
}

#if XR_PTR_SIZE == 8
/// Turns a pointer-sized integer into a string formatted as hex.
static inline std::string UintptrToHexString(uintptr_t val) { return Uint64ToHexString(val); }
#else
/// Turns a pointer-sized integer into a string formatted as hex.
static inline std::string UintptrToHexString(uintptr_t val) { return Uint32ToHexString(val); }
#endif

/// Convert a pointer to a string formatted as hex.
template <typename T>
static inline std::string PointerToHexString(T const* ptr) {
    return UintptrToHexString(reinterpret_cast<uintptr_t>(ptr));
}

// Define this only once in your project, in a non-header-file, to include the implementation.
#if defined(XR_UTILS_INCLUDE_IMPLEMENTATION) || defined(XR_UTILS_INLINE_IMPLEMENTATION)

#include <sstream>
#include <iomanip>

XR_UTILS_STATIC_INLINE std::string Uint64ToHexString(uint64_t val) {
    std::ostringstream oss;
    oss << "0x";
    oss << std::hex << std::setw(16) << std::setfill('0') << val;
    return oss.str();
}

XR_UTILS_STATIC_INLINE std::string Uint32ToHexString(uint32_t val) {
    std::ostringstream oss;
    oss << "0x";
    oss << std::hex << std::setw(8) << std::setfill('0') << val;
    return oss.str();
}

#endif  // defined(XR_UTILS_INCLUDE_IMPLEMENTATION) || defined(XR_UTILS_INLINE_IMPLEMENTATION)
