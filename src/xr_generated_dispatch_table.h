// *********** THIS FILE IS GENERATED - DO NOT EDIT ***********
//     See utility_source_generator.py for modifications
// ************************************************************

// Copyright (c) 2017-2020 The Khronos Group Inc.
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
#include "xr_dependencies.h"
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>


#ifdef __cplusplus
extern "C" { 
#endif
// Generated dispatch table
struct XrGeneratedDispatchTable {

    // ---- Core 1.0 commands
    PFN_xrGetInstanceProcAddr GetInstanceProcAddr;
    PFN_xrEnumerateApiLayerProperties EnumerateApiLayerProperties;
    PFN_xrEnumerateInstanceExtensionProperties EnumerateInstanceExtensionProperties;
    PFN_xrCreateInstance CreateInstance;
    PFN_xrDestroyInstance DestroyInstance;
    PFN_xrGetInstanceProperties GetInstanceProperties;
    PFN_xrPollEvent PollEvent;
    PFN_xrResultToString ResultToString;
    PFN_xrStructureTypeToString StructureTypeToString;
    PFN_xrGetSystem GetSystem;
    PFN_xrGetSystemProperties GetSystemProperties;
    PFN_xrEnumerateEnvironmentBlendModes EnumerateEnvironmentBlendModes;
    PFN_xrCreateSession CreateSession;
    PFN_xrDestroySession DestroySession;
    PFN_xrEnumerateReferenceSpaces EnumerateReferenceSpaces;
    PFN_xrCreateReferenceSpace CreateReferenceSpace;
    PFN_xrGetReferenceSpaceBoundsRect GetReferenceSpaceBoundsRect;
    PFN_xrCreateActionSpace CreateActionSpace;
    PFN_xrLocateSpace LocateSpace;
    PFN_xrDestroySpace DestroySpace;
    PFN_xrEnumerateViewConfigurations EnumerateViewConfigurations;
    PFN_xrGetViewConfigurationProperties GetViewConfigurationProperties;
    PFN_xrEnumerateViewConfigurationViews EnumerateViewConfigurationViews;
    PFN_xrEnumerateSwapchainFormats EnumerateSwapchainFormats;
    PFN_xrCreateSwapchain CreateSwapchain;
    PFN_xrDestroySwapchain DestroySwapchain;
    PFN_xrEnumerateSwapchainImages EnumerateSwapchainImages;
    PFN_xrAcquireSwapchainImage AcquireSwapchainImage;
    PFN_xrWaitSwapchainImage WaitSwapchainImage;
    PFN_xrReleaseSwapchainImage ReleaseSwapchainImage;
    PFN_xrBeginSession BeginSession;
    PFN_xrEndSession EndSession;
    PFN_xrRequestExitSession RequestExitSession;
    PFN_xrWaitFrame WaitFrame;
    PFN_xrBeginFrame BeginFrame;
    PFN_xrEndFrame EndFrame;
    PFN_xrLocateViews LocateViews;
    PFN_xrStringToPath StringToPath;
    PFN_xrPathToString PathToString;
    PFN_xrCreateActionSet CreateActionSet;
    PFN_xrDestroyActionSet DestroyActionSet;
    PFN_xrCreateAction CreateAction;
    PFN_xrDestroyAction DestroyAction;
    PFN_xrSuggestInteractionProfileBindings SuggestInteractionProfileBindings;
    PFN_xrAttachSessionActionSets AttachSessionActionSets;
    PFN_xrGetCurrentInteractionProfile GetCurrentInteractionProfile;
    PFN_xrGetActionStateBoolean GetActionStateBoolean;
    PFN_xrGetActionStateFloat GetActionStateFloat;
    PFN_xrGetActionStateVector2f GetActionStateVector2f;
    PFN_xrGetActionStatePose GetActionStatePose;
    PFN_xrSyncActions SyncActions;
    PFN_xrEnumerateBoundSourcesForAction EnumerateBoundSourcesForAction;
    PFN_xrGetInputSourceLocalizedName GetInputSourceLocalizedName;
    PFN_xrApplyHapticFeedback ApplyHapticFeedback;
    PFN_xrStopHapticFeedback StopHapticFeedback;

    // ---- XR_KHR_android_thread_settings extension commands
#if defined(XR_USE_PLATFORM_ANDROID)
    PFN_xrSetAndroidApplicationThreadKHR SetAndroidApplicationThreadKHR;
#endif // defined(XR_USE_PLATFORM_ANDROID)

    // ---- XR_KHR_android_surface_swapchain extension commands
#if defined(XR_USE_PLATFORM_ANDROID)
    PFN_xrCreateSwapchainAndroidSurfaceKHR CreateSwapchainAndroidSurfaceKHR;
#endif // defined(XR_USE_PLATFORM_ANDROID)

    // ---- XR_KHR_opengl_enable extension commands
#if defined(XR_USE_GRAPHICS_API_OPENGL)
    PFN_xrGetOpenGLGraphicsRequirementsKHR GetOpenGLGraphicsRequirementsKHR;
#endif // defined(XR_USE_GRAPHICS_API_OPENGL)

    // ---- XR_KHR_opengl_es_enable extension commands
#if defined(XR_USE_GRAPHICS_API_OPENGL_ES)
    PFN_xrGetOpenGLESGraphicsRequirementsKHR GetOpenGLESGraphicsRequirementsKHR;
#endif // defined(XR_USE_GRAPHICS_API_OPENGL_ES)

    // ---- XR_KHR_vulkan_enable extension commands
#if defined(XR_USE_GRAPHICS_API_VULKAN)
    PFN_xrGetVulkanInstanceExtensionsKHR GetVulkanInstanceExtensionsKHR;
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
    PFN_xrGetVulkanDeviceExtensionsKHR GetVulkanDeviceExtensionsKHR;
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
    PFN_xrGetVulkanGraphicsDeviceKHR GetVulkanGraphicsDeviceKHR;
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
    PFN_xrGetVulkanGraphicsRequirementsKHR GetVulkanGraphicsRequirementsKHR;
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)

    // ---- XR_KHR_D3D11_enable extension commands
#if defined(XR_USE_GRAPHICS_API_D3D11)
    PFN_xrGetD3D11GraphicsRequirementsKHR GetD3D11GraphicsRequirementsKHR;
#endif // defined(XR_USE_GRAPHICS_API_D3D11)

    // ---- XR_KHR_D3D12_enable extension commands
#if defined(XR_USE_GRAPHICS_API_D3D12)
    PFN_xrGetD3D12GraphicsRequirementsKHR GetD3D12GraphicsRequirementsKHR;
#endif // defined(XR_USE_GRAPHICS_API_D3D12)

    // ---- XR_KHR_visibility_mask extension commands
    PFN_xrGetVisibilityMaskKHR GetVisibilityMaskKHR;

    // ---- XR_KHR_win32_convert_performance_counter_time extension commands
#if defined(XR_USE_PLATFORM_WIN32)
    PFN_xrConvertWin32PerformanceCounterToTimeKHR ConvertWin32PerformanceCounterToTimeKHR;
#endif // defined(XR_USE_PLATFORM_WIN32)
#if defined(XR_USE_PLATFORM_WIN32)
    PFN_xrConvertTimeToWin32PerformanceCounterKHR ConvertTimeToWin32PerformanceCounterKHR;
#endif // defined(XR_USE_PLATFORM_WIN32)

    // ---- XR_KHR_convert_timespec_time extension commands
#if defined(XR_USE_TIMESPEC)
    PFN_xrConvertTimespecTimeToTimeKHR ConvertTimespecTimeToTimeKHR;
#endif // defined(XR_USE_TIMESPEC)
#if defined(XR_USE_TIMESPEC)
    PFN_xrConvertTimeToTimespecTimeKHR ConvertTimeToTimespecTimeKHR;
#endif // defined(XR_USE_TIMESPEC)

    // ---- XR_EXT_performance_settings extension commands
    PFN_xrPerfSettingsSetPerformanceLevelEXT PerfSettingsSetPerformanceLevelEXT;

    // ---- XR_EXT_thermal_query extension commands
    PFN_xrThermalGetTemperatureTrendEXT ThermalGetTemperatureTrendEXT;

    // ---- XR_EXT_debug_utils extension commands
    PFN_xrSetDebugUtilsObjectNameEXT SetDebugUtilsObjectNameEXT;
    PFN_xrCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT;
    PFN_xrDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT;
    PFN_xrSubmitDebugUtilsMessageEXT SubmitDebugUtilsMessageEXT;
    PFN_xrSessionBeginDebugUtilsLabelRegionEXT SessionBeginDebugUtilsLabelRegionEXT;
    PFN_xrSessionEndDebugUtilsLabelRegionEXT SessionEndDebugUtilsLabelRegionEXT;
    PFN_xrSessionInsertDebugUtilsLabelEXT SessionInsertDebugUtilsLabelEXT;

    // ---- XR_MSFT_spatial_anchor extension commands
    PFN_xrCreateSpatialAnchorMSFT CreateSpatialAnchorMSFT;
    PFN_xrCreateSpatialAnchorSpaceMSFT CreateSpatialAnchorSpaceMSFT;
    PFN_xrDestroySpatialAnchorMSFT DestroySpatialAnchorMSFT;

    // ---- XR_EXT_conformance_automation extension commands
    PFN_xrSetInputDeviceActiveEXT SetInputDeviceActiveEXT;
    PFN_xrSetInputDeviceStateBoolEXT SetInputDeviceStateBoolEXT;
    PFN_xrSetInputDeviceStateFloatEXT SetInputDeviceStateFloatEXT;
    PFN_xrSetInputDeviceStateVector2fEXT SetInputDeviceStateVector2fEXT;
    PFN_xrSetInputDeviceLocationEXT SetInputDeviceLocationEXT;
};


// Prototype for dispatch table helper function
void GeneratedXrPopulateDispatchTable(struct XrGeneratedDispatchTable *table,
                                      XrInstance instance,
                                      PFN_xrGetInstanceProcAddr get_inst_proc_addr);

#ifdef __cplusplus
} // extern "C"
#endif

