// Copyright (c) 2018-2019 The Khronos Group Inc.
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
// This file includes headers with types which openxr.h depends on in order
// to compile when platforms, graphics apis, and the like are enabled.

#pragma once

#ifdef XR_USE_PLATFORM_ANDROID
#include <android/native_window.h>
#endif  // XR_USE_PLATFORM_ANDROID

#ifdef XR_USE_PLATFORM_WIN32

#include <winapifamily.h>
#if !(WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP | WINAPI_PARTITION_SYSTEM))
#pragma push_macro("WINAPI_PARTITION_DESKTOP")
#undef WINAPI_PARTITION_DESKTOP
#define WINAPI_PARTITION_DESKTOP 1  // Enable desktop partition apis, such as RegOpenKeyEx, LoadLibraryEx etc.
#define CHANGED_WINAPI_PARTITION_DESKTOP_VALUE
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#if defined(CHANGED_WINAPI_PARTITION_DESKTOP_VALUE)
#pragma pop_macro("WINAPI_PARTITION_DESKTOP")
#endif

#endif  // XR_USE_PLATFORM_WIN32

#ifdef XR_USE_GRAPHICS_API_D3D10
// d3d10_1 must be included to ensure proper SAL annotations, otherwise the compiler will emit:
//    #error:  d3d10.h is included before d3d10_1.h, and it will confuse tools that honor SAL annotations.
//    If possibly targeting d3d10.1, include d3d10_1.h instead of d3d10.h, or ensure d3d10_1.h is included before d3d10.h
#include <d3d10_1.h>
#endif  // XR_USE_GRAPHICS_API_D3D10

#ifdef XR_USE_GRAPHICS_API_D3D11
#include <d3d11.h>
#endif  // XR_USE_GRAPHICS_API_D3D11

#ifdef XR_USE_GRAPHICS_API_D3D12
#include <d3d12.h>
#endif  // XR_USE_GRAPHICS_API_D3D12

#ifdef XR_USE_PLATFORM_XLIB
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif  // XR_USE_PLATFORM_XLIB

#ifdef XR_USE_PLATFORM_XCB
#include <xcb/xcb.h>
#endif  // XR_USE_PLATFORM_XCB

#ifdef XR_USE_GRAPHICS_API_OPENGL
#if defined(XR_USE_PLATFORM_XLIB) || defined(XR_USE_PLATFORM_XCB)
#include <GL/glx.h>
#endif  // (XR_USE_PLATFORM_XLIB || XR_USE_PLATFORM_XCB)
#ifdef XR_USE_PLATFORM_XCB
#include <xcb/glx.h>
#endif  // XR_USE_PLATFORM_XCB
#ifdef XR_USE_PLATFORM_MACOS
#include <CL/cl_gl_ext.h>
#endif  // XR_USE_PLATFORM_MACOS
#endif  // XR_USE_GRAPHICS_API_OPENGL

#ifdef XR_USE_GRAPHICS_API_OPENGL_ES
#include <EGL/egl.h>
#endif  // XR_USE_GRAPHICS_API_OPENGL_ES

#ifdef XR_USE_GRAPHICS_API_VULKAN
#include <vulkan/vulkan.h>
#endif  // XR_USE_GRAPHICS_API_VULKAN

#ifdef XR_USE_PLATFORM_WAYLAND
#include "wayland-client.h"
#endif  // XR_USE_PLATFORM_WAYLAND
