// Copyright (c) 2018-2019 The Khronos Group Inc.
// Copyright (c) 2018-2019 Valve Corporation
// Copyright (c) 2018-2019 LunarG, Inc.
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

#ifndef API_LAYER_PLATFORM_DEFINES_H_
#define API_LAYER_PLATFORM_DEFINES_H_ 1

#ifdef _WIN32
#define XR_USE_PLATFORM_WIN32 1  // Win32 platform
#endif                           // _WIN32
#if defined(OS_LINUX_XLIB)
#define XR_USE_PLATFORM_XLIB 1  // Xlib platform
#endif                          // OS_LINUX_XLIB
#if defined(OS_LINUX_XCB) || defined(OS_LINUX_XCB_GLX)
#define XR_USE_PLATFORM_XCB 1  // Xcb platform
#endif                         // OS_LINUX_XCB || OS_LINUX_XCB_GLX
#if defined(OS_LINUX_WAYLAND)
#define XR_USE_PLATFORM_WAYLAND 1  // Wayland platform
#endif                             // OS_LINUX_WAYLAND
#if defined(XR_OS_APPLE_IOS)
#define XR_USE_PLATFORM_IOS 1            // iOS platform
#define XR_USE_GRAPHICS_API_OPENGL_ES 1  // OpenGLES exists
#define XR_USE_GRAPHICS_API_METAL 1      // Metal exists
#endif                                   // XR_OS_APPLE_IOS
#if defined(XR_OS_APPLE_MACOS)
#define XR_USE_PLATFORM_MACOS 1       // MacOS platform
#define XR_USE_GRAPHICS_API_OPENGL 1  // OpenGL exists
#define XR_USE_GRAPHICS_API_METAL 1   // Metal exists
#endif                                // XR_OS_APPLE_MACOS

#include "xr_dependencies.h" // IWYU pragma: export

#endif  // API_LAYER_PLATFORM_DEFINES_H_
