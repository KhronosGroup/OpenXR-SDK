// *********** THIS FILE IS GENERATED - DO NOT EDIT ***********
//     See loader_source_generator.py for modifications
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
#include <unordered_map>
#include <thread>
#include <mutex>

#include "xr_dependencies.h"
#include "openxr/openxr.h"
#include "openxr/openxr_platform.h"

#include "loader_interfaces.h"

#include "loader_instance.hpp"

#include "loader_platform.hpp"


#ifdef __cplusplus
extern "C" { 
#endif

// Loader manually generated function prototypes


// ---- Core 1.0 loader manual functions
extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetInstanceProcAddr(
    XrInstance                                  instance,
    const char*                                 name,
    PFN_xrVoidFunction*                         function);
extern "C" XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermGetInstanceProcAddr(
    XrInstance                                  instance,
    const char*                                 name,
    PFN_xrVoidFunction*                         function);
extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateApiLayerProperties(
    uint32_t                                    propertyCapacityInput,
    uint32_t*                                   propertyCountOutput,
    XrApiLayerProperties*                       properties);
extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateInstanceExtensionProperties(
    const char*                                 layerName,
    uint32_t                                    propertyCapacityInput,
    uint32_t*                                   propertyCountOutput,
    XrExtensionProperties*                      properties);
extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrCreateInstance(
    const XrInstanceCreateInfo*                 createInfo,
    XrInstance*                                 instance);
extern "C" XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermCreateInstance(
    const XrInstanceCreateInfo*                 createInfo,
    XrInstance*                                 instance);
extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrDestroyInstance(
    XrInstance                                  instance);
extern "C" XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermDestroyInstance(
    XrInstance                                  instance);
extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetInstanceProperties(
    XrInstance                                  instance,
    XrInstanceProperties*                       instanceProperties);
extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrPollEvent(
    XrInstance                                  instance,
    XrEventDataBuffer*                          eventData);
extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrResultToString(
    XrInstance                                  instance,
    XrResult                                    value,
    char                                        buffer[XR_MAX_RESULT_STRING_SIZE]);
extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrStructureTypeToString(
    XrInstance                                  instance,
    XrStructureType                             value,
    char                                        buffer[XR_MAX_STRUCTURE_NAME_SIZE]);
extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetSystem(
    XrInstance                                  instance,
    const XrSystemGetInfo*                      getInfo,
    XrSystemId*                                 systemId);
extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetSystemProperties(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrSystemProperties*                         properties);
extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateEnvironmentBlendModes(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrViewConfigurationType                     viewConfigurationType,
    uint32_t                                    environmentBlendModeCapacityInput,
    uint32_t*                                   environmentBlendModeCountOutput,
    XrEnvironmentBlendMode*                     environmentBlendModes);
extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrCreateSession(
    XrInstance                                  instance,
    const XrSessionCreateInfo*                  createInfo,
    XrSession*                                  session);
extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateViewConfigurations(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    uint32_t                                    viewConfigurationTypeCapacityInput,
    uint32_t*                                   viewConfigurationTypeCountOutput,
    XrViewConfigurationType*                    viewConfigurationTypes);
extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetViewConfigurationProperties(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrViewConfigurationType                     viewConfigurationType,
    XrViewConfigurationProperties*              configurationProperties);
extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateViewConfigurationViews(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrViewConfigurationType                     viewConfigurationType,
    uint32_t                                    viewCapacityInput,
    uint32_t*                                   viewCountOutput,
    XrViewConfigurationView*                    views);
extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrStringToPath(
    XrInstance                                  instance,
    const char*                                 pathString,
    XrPath*                                     path);
extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrPathToString(
    XrInstance                                  instance,
    XrPath                                      path,
    uint32_t                                    bufferCapacityInput,
    uint32_t*                                   bufferCountOutput,
    char*                                       buffer);
extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrCreateActionSet(
    XrInstance                                  instance,
    const XrActionSetCreateInfo*                createInfo,
    XrActionSet*                                actionSet);
extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrSuggestInteractionProfileBindings(
    XrInstance                                  instance,
    const XrInteractionProfileSuggestedBinding* suggestedBindings);

// ---- XR_KHR_opengl_enable loader manual functions
#if defined(XR_USE_GRAPHICS_API_OPENGL)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrGetOpenGLGraphicsRequirementsKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrGraphicsRequirementsOpenGLKHR*            graphicsRequirements);
#endif // defined(XR_USE_GRAPHICS_API_OPENGL)

// ---- XR_KHR_opengl_es_enable loader manual functions
#if defined(XR_USE_GRAPHICS_API_OPENGL_ES)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrGetOpenGLESGraphicsRequirementsKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrGraphicsRequirementsOpenGLESKHR*          graphicsRequirements);
#endif // defined(XR_USE_GRAPHICS_API_OPENGL_ES)

// ---- XR_KHR_vulkan_enable loader manual functions
#if defined(XR_USE_GRAPHICS_API_VULKAN)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrGetVulkanInstanceExtensionsKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    uint32_t                                    bufferCapacityInput,
    uint32_t*                                   bufferCountOutput,
    char*                                       buffer);
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrGetVulkanDeviceExtensionsKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    uint32_t                                    bufferCapacityInput,
    uint32_t*                                   bufferCountOutput,
    char*                                       buffer);
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrGetVulkanGraphicsDeviceKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    VkInstance                                  vkInstance,
    VkPhysicalDevice*                           vkPhysicalDevice);
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrGetVulkanGraphicsRequirementsKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrGraphicsRequirementsVulkanKHR*            graphicsRequirements);
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)

// ---- XR_KHR_D3D11_enable loader manual functions
#if defined(XR_USE_GRAPHICS_API_D3D11)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrGetD3D11GraphicsRequirementsKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrGraphicsRequirementsD3D11KHR*             graphicsRequirements);
#endif // defined(XR_USE_GRAPHICS_API_D3D11)

// ---- XR_KHR_D3D12_enable loader manual functions
#if defined(XR_USE_GRAPHICS_API_D3D12)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrGetD3D12GraphicsRequirementsKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrGraphicsRequirementsD3D12KHR*             graphicsRequirements);
#endif // defined(XR_USE_GRAPHICS_API_D3D12)

// ---- XR_KHR_win32_convert_performance_counter_time loader manual functions
#if defined(XR_USE_PLATFORM_WIN32)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrConvertWin32PerformanceCounterToTimeKHR(
    XrInstance                                  instance,
    const LARGE_INTEGER*                        performanceCounter,
    XrTime*                                     time);
#endif // defined(XR_USE_PLATFORM_WIN32)
#if defined(XR_USE_PLATFORM_WIN32)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrConvertTimeToWin32PerformanceCounterKHR(
    XrInstance                                  instance,
    XrTime                                      time,
    LARGE_INTEGER*                              performanceCounter);
#endif // defined(XR_USE_PLATFORM_WIN32)

// ---- XR_KHR_convert_timespec_time loader manual functions
#if defined(XR_USE_TIMESPEC)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrConvertTimespecTimeToTimeKHR(
    XrInstance                                  instance,
    const struct timespec*                      timespecTime,
    XrTime*                                     time);
#endif // defined(XR_USE_TIMESPEC)
#if defined(XR_USE_TIMESPEC)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrConvertTimeToTimespecTimeKHR(
    XrInstance                                  instance,
    XrTime                                      time,
    struct timespec*                            timespecTime);
#endif // defined(XR_USE_TIMESPEC)

// ---- XR_EXT_debug_utils loader manual functions
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrSetDebugUtilsObjectNameEXT(
    XrInstance                                  instance,
    const XrDebugUtilsObjectNameInfoEXT*        nameInfo);
extern "C" XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermSetDebugUtilsObjectNameEXT(
    XrInstance                                  instance,
    const XrDebugUtilsObjectNameInfoEXT*        nameInfo);
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrCreateDebugUtilsMessengerEXT(
    XrInstance                                  instance,
    const XrDebugUtilsMessengerCreateInfoEXT*   createInfo,
    XrDebugUtilsMessengerEXT*                   messenger);
extern "C" XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermCreateDebugUtilsMessengerEXT(
    XrInstance                                  instance,
    const XrDebugUtilsMessengerCreateInfoEXT*   createInfo,
    XrDebugUtilsMessengerEXT*                   messenger);
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrDestroyDebugUtilsMessengerEXT(
    XrDebugUtilsMessengerEXT                    messenger);
extern "C" XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermDestroyDebugUtilsMessengerEXT(
    XrDebugUtilsMessengerEXT                    messenger);
extern "C" XRAPI_ATTR XrResult                                    XRAPI_CALL xrSubmitDebugUtilsMessageEXT(
    XrInstance                                  instance,
    XrDebugUtilsMessageSeverityFlagsEXT         messageSeverity,
    XrDebugUtilsMessageTypeFlagsEXT             messageTypes,
    const XrDebugUtilsMessengerCallbackDataEXT* callbackData);
extern "C" XRAPI_ATTR XrResult                                    XRAPI_CALL LoaderXrTermSubmitDebugUtilsMessageEXT(
    XrInstance                                  instance,
    XrDebugUtilsMessageSeverityFlagsEXT         messageSeverity,
    XrDebugUtilsMessageTypeFlagsEXT             messageTypes,
    const XrDebugUtilsMessengerCallbackDataEXT* callbackData);
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrSessionBeginDebugUtilsLabelRegionEXT(
    XrSession                                   session,
    const XrDebugUtilsLabelEXT*                 labelInfo);
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrSessionEndDebugUtilsLabelRegionEXT(
    XrSession                                   session);
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrSessionInsertDebugUtilsLabelEXT(
    XrSession                                   session,
    const XrDebugUtilsLabelEXT*                 labelInfo);

// Special use function to handle creating API Layer information during xrCreateInstance
XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermCreateApiLayerInstance(const XrInstanceCreateInfo* info,
                                                                  const struct XrApiLayerCreateInfo* apiLayerInfo,
                                                                  XrInstance* instance);


// Generated loader terminator prototypes
// Instance Init Dispatch Table (put all terminators in first)
void LoaderGenInitInstanceDispatchTable(XrInstance runtime_instance,
                                        std::unique_ptr<XrGeneratedDispatchTable>& table);

#ifdef __cplusplus
} // extern "C"
#endif

// Unordered maps and mutexes to lookup the instance for a given object type
extern HandleLoaderMap<XrInstance> g_instance_map;
extern HandleLoaderMap<XrSession> g_session_map;
extern HandleLoaderMap<XrSpace> g_space_map;
extern HandleLoaderMap<XrAction> g_action_map;
extern HandleLoaderMap<XrSwapchain> g_swapchain_map;
extern HandleLoaderMap<XrActionSet> g_actionset_map;
extern HandleLoaderMap<XrDebugUtilsMessengerEXT> g_debugutilsmessengerext_map;
extern HandleLoaderMap<XrSpatialAnchorMSFT> g_spatialanchormsft_map;

// Function used to clean up any residual map values that point to an instance prior to that
// instance being deleted.
void LoaderCleanUpMapsForInstance(LoaderInstance *instance);


