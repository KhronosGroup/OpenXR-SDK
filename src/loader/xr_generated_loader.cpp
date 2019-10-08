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

#include "xr_generated_loader.hpp"

#include "api_layer_interface.hpp"
#include "exception_handling.hpp"
#include "hex_and_handles.h"
#include "loader_instance.hpp"
#include "loader_logger.hpp"
#include "loader_platform.hpp"
#include "runtime_interface.hpp"
#include "xr_generated_dispatch_table.h"

#include "xr_dependencies.h"
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <cstring>
#include <memory>
#include <new>
#include <string>
#include <unordered_map>

// Unordered maps to lookup the instance for a given object type
HandleLoaderMap<XrInstance> g_instance_map;
HandleLoaderMap<XrSession> g_session_map;
HandleLoaderMap<XrSpace> g_space_map;
HandleLoaderMap<XrAction> g_action_map;
HandleLoaderMap<XrSwapchain> g_swapchain_map;
HandleLoaderMap<XrActionSet> g_actionset_map;
HandleLoaderMap<XrDebugUtilsMessengerEXT> g_debugutilsmessengerext_map;
HandleLoaderMap<XrSpatialAnchorMSFT> g_spatialanchormsft_map;

// Function used to clean up any residual map values that point to an instance prior to that
// instance being deleted.
void LoaderCleanUpMapsForInstance(LoaderInstance *instance) {
    g_instance_map.RemoveHandlesForLoader(*instance);
    g_session_map.RemoveHandlesForLoader(*instance);
    g_space_map.RemoveHandlesForLoader(*instance);
    g_action_map.RemoveHandlesForLoader(*instance);
    g_swapchain_map.RemoveHandlesForLoader(*instance);
    g_actionset_map.RemoveHandlesForLoader(*instance);
    g_debugutilsmessengerext_map.RemoveHandlesForLoader(*instance);
    g_spatialanchormsft_map.RemoveHandlesForLoader(*instance);
}


// Automatically generated instance trampolines and terminators

// ---- Core 1.0 commands
extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetInstanceProperties(
    XrInstance                                  instance,
    XrInstanceProperties*                       instanceProperties) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrGetInstanceProperties-instance-parameter",
                "xrGetInstanceProperties",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->GetInstanceProperties(instance, instanceProperties);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrPollEvent(
    XrInstance                                  instance,
    XrEventDataBuffer*                          eventData) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrPollEvent-instance-parameter",
                "xrPollEvent",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->PollEvent(instance, eventData);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrResultToString(
    XrInstance                                  instance,
    XrResult                                    value,
    char                                        buffer[XR_MAX_RESULT_STRING_SIZE]) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrResultToString-instance-parameter",
                "xrResultToString",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->ResultToString(instance, value, buffer);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrStructureTypeToString(
    XrInstance                                  instance,
    XrStructureType                             value,
    char                                        buffer[XR_MAX_STRUCTURE_NAME_SIZE]) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrStructureTypeToString-instance-parameter",
                "xrStructureTypeToString",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->StructureTypeToString(instance, value, buffer);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetSystem(
    XrInstance                                  instance,
    const XrSystemGetInfo*                      getInfo,
    XrSystemId*                                 systemId) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrGetSystem-instance-parameter",
                "xrGetSystem",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->GetSystem(instance, getInfo, systemId);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetSystemProperties(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrSystemProperties*                         properties) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrGetSystemProperties-instance-parameter",
                "xrGetSystemProperties",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->GetSystemProperties(instance, systemId, properties);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateEnvironmentBlendModes(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrViewConfigurationType                     viewConfigurationType,
    uint32_t                                    environmentBlendModeCapacityInput,
    uint32_t*                                   environmentBlendModeCountOutput,
    XrEnvironmentBlendMode*                     environmentBlendModes) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrEnumerateEnvironmentBlendModes-instance-parameter",
                "xrEnumerateEnvironmentBlendModes",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->EnumerateEnvironmentBlendModes(instance, systemId, viewConfigurationType, environmentBlendModeCapacityInput, environmentBlendModeCountOutput, environmentBlendModes);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrCreateSession(
    XrInstance                                  instance,
    const XrSessionCreateInfo*                  createInfo,
    XrSession*                                  session) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrCreateSession-instance-parameter",
                "xrCreateSession",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
    result = dispatch_table->CreateSession(instance, createInfo, session);
        if (XR_SUCCESS == result && nullptr != session) {
            XrResult insert_result = g_session_map.Insert(*session, *loader_instance);
            if (XR_FAILED(insert_result)) {
                LoaderLogger::LogErrorMessage(
                    "xrCreateSession",
                    "Failed inserting new session into map: may be null or not unique");
            }
        }
    return result;
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrDestroySession(
    XrSession                                   session) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    // Destroy the mapping entry for this item if it was valid.
    if (nullptr != loader_instance) {
            g_session_map.Erase(session);
    }
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrDestroySession-session-parameter",
                "xrDestroySession",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
    result = dispatch_table->DestroySession(session);
    return result;
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateReferenceSpaces(
    XrSession                                   session,
    uint32_t                                    spaceCapacityInput,
    uint32_t*                                   spaceCountOutput,
    XrReferenceSpaceType*                       spaces) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrEnumerateReferenceSpaces-session-parameter",
                "xrEnumerateReferenceSpaces",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->EnumerateReferenceSpaces(session, spaceCapacityInput, spaceCountOutput, spaces);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrCreateReferenceSpace(
    XrSession                                   session,
    const XrReferenceSpaceCreateInfo*           createInfo,
    XrSpace*                                    space) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrCreateReferenceSpace-session-parameter",
                "xrCreateReferenceSpace",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
    result = dispatch_table->CreateReferenceSpace(session, createInfo, space);
        if (XR_SUCCESS == result && nullptr != space) {
            XrResult insert_result = g_space_map.Insert(*space, *loader_instance);
            if (XR_FAILED(insert_result)) {
                LoaderLogger::LogErrorMessage(
                    "xrCreateReferenceSpace",
                    "Failed inserting new space into map: may be null or not unique");
            }
        }
    return result;
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetReferenceSpaceBoundsRect(
    XrSession                                   session,
    XrReferenceSpaceType                        referenceSpaceType,
    XrExtent2Df*                                bounds) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrGetReferenceSpaceBoundsRect-session-parameter",
                "xrGetReferenceSpaceBoundsRect",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->GetReferenceSpaceBoundsRect(session, referenceSpaceType, bounds);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrCreateActionSpace(
    XrSession                                   session,
    const XrActionSpaceCreateInfo*              createInfo,
    XrSpace*                                    space) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrCreateActionSpace-session-parameter",
                "xrCreateActionSpace",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
    result = dispatch_table->CreateActionSpace(session, createInfo, space);
        if (XR_SUCCESS == result && nullptr != space) {
            XrResult insert_result = g_space_map.Insert(*space, *loader_instance);
            if (XR_FAILED(insert_result)) {
                LoaderLogger::LogErrorMessage(
                    "xrCreateActionSpace",
                    "Failed inserting new space into map: may be null or not unique");
            }
        }
    return result;
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrLocateSpace(
    XrSpace                                     space,
    XrSpace                                     baseSpace,
    XrTime                                      time,
    XrSpaceLocation*                            location) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_space_map.Get(space);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrLocateSpace-space-parameter",
                "xrLocateSpace",
                "space is not a valid XrSpace",
                {XrSdkLogObjectInfo{space, XR_OBJECT_TYPE_SPACE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->LocateSpace(space, baseSpace, time, location);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrDestroySpace(
    XrSpace                                     space) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_space_map.Get(space);
    // Destroy the mapping entry for this item if it was valid.
    if (nullptr != loader_instance) {
            g_space_map.Erase(space);
    }
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrDestroySpace-space-parameter",
                "xrDestroySpace",
                "space is not a valid XrSpace",
                {XrSdkLogObjectInfo{space, XR_OBJECT_TYPE_SPACE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
    result = dispatch_table->DestroySpace(space);
    return result;
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateViewConfigurations(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    uint32_t                                    viewConfigurationTypeCapacityInput,
    uint32_t*                                   viewConfigurationTypeCountOutput,
    XrViewConfigurationType*                    viewConfigurationTypes) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrEnumerateViewConfigurations-instance-parameter",
                "xrEnumerateViewConfigurations",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->EnumerateViewConfigurations(instance, systemId, viewConfigurationTypeCapacityInput, viewConfigurationTypeCountOutput, viewConfigurationTypes);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetViewConfigurationProperties(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrViewConfigurationType                     viewConfigurationType,
    XrViewConfigurationProperties*              configurationProperties) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrGetViewConfigurationProperties-instance-parameter",
                "xrGetViewConfigurationProperties",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->GetViewConfigurationProperties(instance, systemId, viewConfigurationType, configurationProperties);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateViewConfigurationViews(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrViewConfigurationType                     viewConfigurationType,
    uint32_t                                    viewCapacityInput,
    uint32_t*                                   viewCountOutput,
    XrViewConfigurationView*                    views) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrEnumerateViewConfigurationViews-instance-parameter",
                "xrEnumerateViewConfigurationViews",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->EnumerateViewConfigurationViews(instance, systemId, viewConfigurationType, viewCapacityInput, viewCountOutput, views);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateSwapchainFormats(
    XrSession                                   session,
    uint32_t                                    formatCapacityInput,
    uint32_t*                                   formatCountOutput,
    int64_t*                                    formats) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrEnumerateSwapchainFormats-session-parameter",
                "xrEnumerateSwapchainFormats",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->EnumerateSwapchainFormats(session, formatCapacityInput, formatCountOutput, formats);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrCreateSwapchain(
    XrSession                                   session,
    const XrSwapchainCreateInfo*                createInfo,
    XrSwapchain*                                swapchain) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrCreateSwapchain-session-parameter",
                "xrCreateSwapchain",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
    result = dispatch_table->CreateSwapchain(session, createInfo, swapchain);
        if (XR_SUCCESS == result && nullptr != swapchain) {
            XrResult insert_result = g_swapchain_map.Insert(*swapchain, *loader_instance);
            if (XR_FAILED(insert_result)) {
                LoaderLogger::LogErrorMessage(
                    "xrCreateSwapchain",
                    "Failed inserting new swapchain into map: may be null or not unique");
            }
        }
    return result;
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrDestroySwapchain(
    XrSwapchain                                 swapchain) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_swapchain_map.Get(swapchain);
    // Destroy the mapping entry for this item if it was valid.
    if (nullptr != loader_instance) {
            g_swapchain_map.Erase(swapchain);
    }
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrDestroySwapchain-swapchain-parameter",
                "xrDestroySwapchain",
                "swapchain is not a valid XrSwapchain",
                {XrSdkLogObjectInfo{swapchain, XR_OBJECT_TYPE_SWAPCHAIN}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
    result = dispatch_table->DestroySwapchain(swapchain);
    return result;
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateSwapchainImages(
    XrSwapchain                                 swapchain,
    uint32_t                                    imageCapacityInput,
    uint32_t*                                   imageCountOutput,
    XrSwapchainImageBaseHeader*                 images) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_swapchain_map.Get(swapchain);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrEnumerateSwapchainImages-swapchain-parameter",
                "xrEnumerateSwapchainImages",
                "swapchain is not a valid XrSwapchain",
                {XrSdkLogObjectInfo{swapchain, XR_OBJECT_TYPE_SWAPCHAIN}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->EnumerateSwapchainImages(swapchain, imageCapacityInput, imageCountOutput, images);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrAcquireSwapchainImage(
    XrSwapchain                                 swapchain,
    const XrSwapchainImageAcquireInfo*          acquireInfo,
    uint32_t*                                   index) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_swapchain_map.Get(swapchain);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrAcquireSwapchainImage-swapchain-parameter",
                "xrAcquireSwapchainImage",
                "swapchain is not a valid XrSwapchain",
                {XrSdkLogObjectInfo{swapchain, XR_OBJECT_TYPE_SWAPCHAIN}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->AcquireSwapchainImage(swapchain, acquireInfo, index);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrWaitSwapchainImage(
    XrSwapchain                                 swapchain,
    const XrSwapchainImageWaitInfo*             waitInfo) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_swapchain_map.Get(swapchain);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrWaitSwapchainImage-swapchain-parameter",
                "xrWaitSwapchainImage",
                "swapchain is not a valid XrSwapchain",
                {XrSdkLogObjectInfo{swapchain, XR_OBJECT_TYPE_SWAPCHAIN}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->WaitSwapchainImage(swapchain, waitInfo);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrReleaseSwapchainImage(
    XrSwapchain                                 swapchain,
    const XrSwapchainImageReleaseInfo*          releaseInfo) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_swapchain_map.Get(swapchain);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrReleaseSwapchainImage-swapchain-parameter",
                "xrReleaseSwapchainImage",
                "swapchain is not a valid XrSwapchain",
                {XrSdkLogObjectInfo{swapchain, XR_OBJECT_TYPE_SWAPCHAIN}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->ReleaseSwapchainImage(swapchain, releaseInfo);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrBeginSession(
    XrSession                                   session,
    const XrSessionBeginInfo*                   beginInfo) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrBeginSession-session-parameter",
                "xrBeginSession",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->BeginSession(session, beginInfo);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEndSession(
    XrSession                                   session) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrEndSession-session-parameter",
                "xrEndSession",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->EndSession(session);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrRequestExitSession(
    XrSession                                   session) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrRequestExitSession-session-parameter",
                "xrRequestExitSession",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->RequestExitSession(session);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrWaitFrame(
    XrSession                                   session,
    const XrFrameWaitInfo*                      frameWaitInfo,
    XrFrameState*                               frameState) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrWaitFrame-session-parameter",
                "xrWaitFrame",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->WaitFrame(session, frameWaitInfo, frameState);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrBeginFrame(
    XrSession                                   session,
    const XrFrameBeginInfo*                     frameBeginInfo) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrBeginFrame-session-parameter",
                "xrBeginFrame",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->BeginFrame(session, frameBeginInfo);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEndFrame(
    XrSession                                   session,
    const XrFrameEndInfo*                       frameEndInfo) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrEndFrame-session-parameter",
                "xrEndFrame",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->EndFrame(session, frameEndInfo);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrLocateViews(
    XrSession                                   session,
    const XrViewLocateInfo*                     viewLocateInfo,
    XrViewState*                                viewState,
    uint32_t                                    viewCapacityInput,
    uint32_t*                                   viewCountOutput,
    XrView*                                     views) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrLocateViews-session-parameter",
                "xrLocateViews",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->LocateViews(session, viewLocateInfo, viewState, viewCapacityInput, viewCountOutput, views);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrStringToPath(
    XrInstance                                  instance,
    const char*                                 pathString,
    XrPath*                                     path) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrStringToPath-instance-parameter",
                "xrStringToPath",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->StringToPath(instance, pathString, path);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrPathToString(
    XrInstance                                  instance,
    XrPath                                      path,
    uint32_t                                    bufferCapacityInput,
    uint32_t*                                   bufferCountOutput,
    char*                                       buffer) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrPathToString-instance-parameter",
                "xrPathToString",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->PathToString(instance, path, bufferCapacityInput, bufferCountOutput, buffer);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrCreateActionSet(
    XrInstance                                  instance,
    const XrActionSetCreateInfo*                createInfo,
    XrActionSet*                                actionSet) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrCreateActionSet-instance-parameter",
                "xrCreateActionSet",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
    result = dispatch_table->CreateActionSet(instance, createInfo, actionSet);
        if (XR_SUCCESS == result && nullptr != actionSet) {
            XrResult insert_result = g_actionset_map.Insert(*actionSet, *loader_instance);
            if (XR_FAILED(insert_result)) {
                LoaderLogger::LogErrorMessage(
                    "xrCreateActionSet",
                    "Failed inserting new actionset into map: may be null or not unique");
            }
        }
    return result;
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrDestroyActionSet(
    XrActionSet                                 actionSet) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_actionset_map.Get(actionSet);
    // Destroy the mapping entry for this item if it was valid.
    if (nullptr != loader_instance) {
            g_actionset_map.Erase(actionSet);
    }
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrDestroyActionSet-actionSet-parameter",
                "xrDestroyActionSet",
                "actionSet is not a valid XrActionSet",
                {XrSdkLogObjectInfo{actionSet, XR_OBJECT_TYPE_ACTION_SET}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
    result = dispatch_table->DestroyActionSet(actionSet);
    return result;
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrCreateAction(
    XrActionSet                                 actionSet,
    const XrActionCreateInfo*                   createInfo,
    XrAction*                                   action) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_actionset_map.Get(actionSet);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrCreateAction-actionSet-parameter",
                "xrCreateAction",
                "actionSet is not a valid XrActionSet",
                {XrSdkLogObjectInfo{actionSet, XR_OBJECT_TYPE_ACTION_SET}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
    result = dispatch_table->CreateAction(actionSet, createInfo, action);
        if (XR_SUCCESS == result && nullptr != action) {
            XrResult insert_result = g_action_map.Insert(*action, *loader_instance);
            if (XR_FAILED(insert_result)) {
                LoaderLogger::LogErrorMessage(
                    "xrCreateAction",
                    "Failed inserting new action into map: may be null or not unique");
            }
        }
    return result;
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrDestroyAction(
    XrAction                                    action) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_action_map.Get(action);
    // Destroy the mapping entry for this item if it was valid.
    if (nullptr != loader_instance) {
            g_action_map.Erase(action);
    }
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrDestroyAction-action-parameter",
                "xrDestroyAction",
                "action is not a valid XrAction",
                {XrSdkLogObjectInfo{action, XR_OBJECT_TYPE_ACTION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
    result = dispatch_table->DestroyAction(action);
    return result;
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrSuggestInteractionProfileBindings(
    XrInstance                                  instance,
    const XrInteractionProfileSuggestedBinding* suggestedBindings) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrSuggestInteractionProfileBindings-instance-parameter",
                "xrSuggestInteractionProfileBindings",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->SuggestInteractionProfileBindings(instance, suggestedBindings);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrAttachSessionActionSets(
    XrSession                                   session,
    const XrSessionActionSetsAttachInfo*        attachInfo) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrAttachSessionActionSets-session-parameter",
                "xrAttachSessionActionSets",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->AttachSessionActionSets(session, attachInfo);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetCurrentInteractionProfile(
    XrSession                                   session,
    XrPath                                      topLevelUserPath,
    XrInteractionProfileState*                  interactionProfile) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrGetCurrentInteractionProfile-session-parameter",
                "xrGetCurrentInteractionProfile",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->GetCurrentInteractionProfile(session, topLevelUserPath, interactionProfile);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetActionStateBoolean(
    XrSession                                   session,
    const XrActionStateGetInfo*                 getInfo,
    XrActionStateBoolean*                       state) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrGetActionStateBoolean-session-parameter",
                "xrGetActionStateBoolean",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->GetActionStateBoolean(session, getInfo, state);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetActionStateFloat(
    XrSession                                   session,
    const XrActionStateGetInfo*                 getInfo,
    XrActionStateFloat*                         state) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrGetActionStateFloat-session-parameter",
                "xrGetActionStateFloat",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->GetActionStateFloat(session, getInfo, state);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetActionStateVector2f(
    XrSession                                   session,
    const XrActionStateGetInfo*                 getInfo,
    XrActionStateVector2f*                      state) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrGetActionStateVector2f-session-parameter",
                "xrGetActionStateVector2f",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->GetActionStateVector2f(session, getInfo, state);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetActionStatePose(
    XrSession                                   session,
    const XrActionStateGetInfo*                 getInfo,
    XrActionStatePose*                          state) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrGetActionStatePose-session-parameter",
                "xrGetActionStatePose",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->GetActionStatePose(session, getInfo, state);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrSyncActions(
    XrSession                                   session,
    const XrActionsSyncInfo*                    syncInfo) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrSyncActions-session-parameter",
                "xrSyncActions",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->SyncActions(session, syncInfo);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateBoundSourcesForAction(
    XrSession                                   session,
    const XrBoundSourcesForActionEnumerateInfo* enumerateInfo,
    uint32_t                                    sourceCapacityInput,
    uint32_t*                                   sourceCountOutput,
    XrPath*                                     sources) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrEnumerateBoundSourcesForAction-session-parameter",
                "xrEnumerateBoundSourcesForAction",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->EnumerateBoundSourcesForAction(session, enumerateInfo, sourceCapacityInput, sourceCountOutput, sources);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetInputSourceLocalizedName(
    XrSession                                   session,
    const XrInputSourceLocalizedNameGetInfo*    getInfo,
    uint32_t                                    bufferCapacityInput,
    uint32_t*                                   bufferCountOutput,
    char*                                       buffer) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrGetInputSourceLocalizedName-session-parameter",
                "xrGetInputSourceLocalizedName",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->GetInputSourceLocalizedName(session, getInfo, bufferCapacityInput, bufferCountOutput, buffer);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrApplyHapticFeedback(
    XrSession                                   session,
    const XrHapticActionInfo*                   hapticActionInfo,
    const XrHapticBaseHeader*                   hapticFeedback) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrApplyHapticFeedback-session-parameter",
                "xrApplyHapticFeedback",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->ApplyHapticFeedback(session, hapticActionInfo, hapticFeedback);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrStopHapticFeedback(
    XrSession                                   session,
    const XrHapticActionInfo*                   hapticActionInfo) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrStopHapticFeedback-session-parameter",
                "xrStopHapticFeedback",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    return dispatch_table->StopHapticFeedback(session, hapticActionInfo);
}
XRLOADER_ABI_CATCH_FALLBACK



// ---- XR_KHR_android_thread_settings extension commands
#if defined(XR_USE_PLATFORM_ANDROID)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrSetAndroidApplicationThreadKHR(
    XrSession                                   session,
    XrAndroidThreadTypeKHR                      threadType,
    uint32_t                                    threadId) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrSetAndroidApplicationThreadKHR-session-parameter",
                "xrSetAndroidApplicationThreadKHR",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    if (!loader_instance->ExtensionIsEnabled("XR_KHR_android_thread_settings")) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrSetAndroidApplicationThreadKHR-extension-notenabled",
                                                "xrSetAndroidApplicationThreadKHR",
                                                "The XR_KHR_android_thread_settings extension has not been enabled prior to calling xrSetAndroidApplicationThreadKHR");
        return XR_ERROR_FUNCTION_UNSUPPORTED;
    }

    return dispatch_table->SetAndroidApplicationThreadKHR(session, threadType, threadId);
}
XRLOADER_ABI_CATCH_FALLBACK

#endif // defined(XR_USE_PLATFORM_ANDROID)


// ---- XR_KHR_android_surface_swapchain extension commands
#if defined(XR_USE_PLATFORM_ANDROID)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrCreateSwapchainAndroidSurfaceKHR(
    XrSession                                   session,
    const XrSwapchainCreateInfo*                info,
    XrSwapchain*                                swapchain,
    jobject*                                    surface) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrCreateSwapchainAndroidSurfaceKHR-session-parameter",
                "xrCreateSwapchainAndroidSurfaceKHR",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
    if (!loader_instance->ExtensionIsEnabled("XR_KHR_android_surface_swapchain")) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrCreateSwapchainAndroidSurfaceKHR-extension-notenabled",
                                                "xrCreateSwapchainAndroidSurfaceKHR",
                                                "The XR_KHR_android_surface_swapchain extension has not been enabled prior to calling xrCreateSwapchainAndroidSurfaceKHR");
        return XR_ERROR_FUNCTION_UNSUPPORTED;
    }

    result = dispatch_table->CreateSwapchainAndroidSurfaceKHR(session, info, swapchain, surface);
    return result;
}
XRLOADER_ABI_CATCH_FALLBACK

#endif // defined(XR_USE_PLATFORM_ANDROID)


// ---- XR_KHR_opengl_enable extension commands
#if defined(XR_USE_GRAPHICS_API_OPENGL)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrGetOpenGLGraphicsRequirementsKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrGraphicsRequirementsOpenGLKHR*            graphicsRequirements) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrGetOpenGLGraphicsRequirementsKHR-instance-parameter",
                "xrGetOpenGLGraphicsRequirementsKHR",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    if (!loader_instance->ExtensionIsEnabled("XR_KHR_opengl_enable")) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrGetOpenGLGraphicsRequirementsKHR-extension-notenabled",
                                                "xrGetOpenGLGraphicsRequirementsKHR",
                                                "The XR_KHR_opengl_enable extension has not been enabled prior to calling xrGetOpenGLGraphicsRequirementsKHR");
        return XR_ERROR_FUNCTION_UNSUPPORTED;
    }

    return dispatch_table->GetOpenGLGraphicsRequirementsKHR(instance, systemId, graphicsRequirements);
}
XRLOADER_ABI_CATCH_FALLBACK

#endif // defined(XR_USE_GRAPHICS_API_OPENGL)


// ---- XR_KHR_opengl_es_enable extension commands
#if defined(XR_USE_GRAPHICS_API_OPENGL_ES)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrGetOpenGLESGraphicsRequirementsKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrGraphicsRequirementsOpenGLESKHR*          graphicsRequirements) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrGetOpenGLESGraphicsRequirementsKHR-instance-parameter",
                "xrGetOpenGLESGraphicsRequirementsKHR",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    if (!loader_instance->ExtensionIsEnabled("XR_KHR_opengl_es_enable")) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrGetOpenGLESGraphicsRequirementsKHR-extension-notenabled",
                                                "xrGetOpenGLESGraphicsRequirementsKHR",
                                                "The XR_KHR_opengl_es_enable extension has not been enabled prior to calling xrGetOpenGLESGraphicsRequirementsKHR");
        return XR_ERROR_FUNCTION_UNSUPPORTED;
    }

    return dispatch_table->GetOpenGLESGraphicsRequirementsKHR(instance, systemId, graphicsRequirements);
}
XRLOADER_ABI_CATCH_FALLBACK

#endif // defined(XR_USE_GRAPHICS_API_OPENGL_ES)


// ---- XR_KHR_vulkan_enable extension commands
#if defined(XR_USE_GRAPHICS_API_VULKAN)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrGetVulkanInstanceExtensionsKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    uint32_t                                    bufferCapacityInput,
    uint32_t*                                   bufferCountOutput,
    char*                                       buffer) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrGetVulkanInstanceExtensionsKHR-instance-parameter",
                "xrGetVulkanInstanceExtensionsKHR",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    if (!loader_instance->ExtensionIsEnabled("XR_KHR_vulkan_enable")) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrGetVulkanInstanceExtensionsKHR-extension-notenabled",
                                                "xrGetVulkanInstanceExtensionsKHR",
                                                "The XR_KHR_vulkan_enable extension has not been enabled prior to calling xrGetVulkanInstanceExtensionsKHR");
        return XR_ERROR_FUNCTION_UNSUPPORTED;
    }

    return dispatch_table->GetVulkanInstanceExtensionsKHR(instance, systemId, bufferCapacityInput, bufferCountOutput, buffer);
}
XRLOADER_ABI_CATCH_FALLBACK

#endif // defined(XR_USE_GRAPHICS_API_VULKAN)

#if defined(XR_USE_GRAPHICS_API_VULKAN)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrGetVulkanDeviceExtensionsKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    uint32_t                                    bufferCapacityInput,
    uint32_t*                                   bufferCountOutput,
    char*                                       buffer) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrGetVulkanDeviceExtensionsKHR-instance-parameter",
                "xrGetVulkanDeviceExtensionsKHR",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    if (!loader_instance->ExtensionIsEnabled("XR_KHR_vulkan_enable")) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrGetVulkanDeviceExtensionsKHR-extension-notenabled",
                                                "xrGetVulkanDeviceExtensionsKHR",
                                                "The XR_KHR_vulkan_enable extension has not been enabled prior to calling xrGetVulkanDeviceExtensionsKHR");
        return XR_ERROR_FUNCTION_UNSUPPORTED;
    }

    return dispatch_table->GetVulkanDeviceExtensionsKHR(instance, systemId, bufferCapacityInput, bufferCountOutput, buffer);
}
XRLOADER_ABI_CATCH_FALLBACK

#endif // defined(XR_USE_GRAPHICS_API_VULKAN)

#if defined(XR_USE_GRAPHICS_API_VULKAN)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrGetVulkanGraphicsDeviceKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    VkInstance                                  vkInstance,
    VkPhysicalDevice*                           vkPhysicalDevice) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrGetVulkanGraphicsDeviceKHR-instance-parameter",
                "xrGetVulkanGraphicsDeviceKHR",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    if (!loader_instance->ExtensionIsEnabled("XR_KHR_vulkan_enable")) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrGetVulkanGraphicsDeviceKHR-extension-notenabled",
                                                "xrGetVulkanGraphicsDeviceKHR",
                                                "The XR_KHR_vulkan_enable extension has not been enabled prior to calling xrGetVulkanGraphicsDeviceKHR");
        return XR_ERROR_FUNCTION_UNSUPPORTED;
    }

    return dispatch_table->GetVulkanGraphicsDeviceKHR(instance, systemId, vkInstance, vkPhysicalDevice);
}
XRLOADER_ABI_CATCH_FALLBACK

#endif // defined(XR_USE_GRAPHICS_API_VULKAN)

#if defined(XR_USE_GRAPHICS_API_VULKAN)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrGetVulkanGraphicsRequirementsKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrGraphicsRequirementsVulkanKHR*            graphicsRequirements) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrGetVulkanGraphicsRequirementsKHR-instance-parameter",
                "xrGetVulkanGraphicsRequirementsKHR",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    if (!loader_instance->ExtensionIsEnabled("XR_KHR_vulkan_enable")) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrGetVulkanGraphicsRequirementsKHR-extension-notenabled",
                                                "xrGetVulkanGraphicsRequirementsKHR",
                                                "The XR_KHR_vulkan_enable extension has not been enabled prior to calling xrGetVulkanGraphicsRequirementsKHR");
        return XR_ERROR_FUNCTION_UNSUPPORTED;
    }

    return dispatch_table->GetVulkanGraphicsRequirementsKHR(instance, systemId, graphicsRequirements);
}
XRLOADER_ABI_CATCH_FALLBACK

#endif // defined(XR_USE_GRAPHICS_API_VULKAN)


// ---- XR_KHR_D3D11_enable extension commands
#if defined(XR_USE_GRAPHICS_API_D3D11)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrGetD3D11GraphicsRequirementsKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrGraphicsRequirementsD3D11KHR*             graphicsRequirements) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrGetD3D11GraphicsRequirementsKHR-instance-parameter",
                "xrGetD3D11GraphicsRequirementsKHR",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    if (!loader_instance->ExtensionIsEnabled("XR_KHR_D3D11_enable")) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrGetD3D11GraphicsRequirementsKHR-extension-notenabled",
                                                "xrGetD3D11GraphicsRequirementsKHR",
                                                "The XR_KHR_D3D11_enable extension has not been enabled prior to calling xrGetD3D11GraphicsRequirementsKHR");
        return XR_ERROR_FUNCTION_UNSUPPORTED;
    }

    return dispatch_table->GetD3D11GraphicsRequirementsKHR(instance, systemId, graphicsRequirements);
}
XRLOADER_ABI_CATCH_FALLBACK

#endif // defined(XR_USE_GRAPHICS_API_D3D11)


// ---- XR_KHR_D3D12_enable extension commands
#if defined(XR_USE_GRAPHICS_API_D3D12)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrGetD3D12GraphicsRequirementsKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrGraphicsRequirementsD3D12KHR*             graphicsRequirements) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrGetD3D12GraphicsRequirementsKHR-instance-parameter",
                "xrGetD3D12GraphicsRequirementsKHR",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    if (!loader_instance->ExtensionIsEnabled("XR_KHR_D3D12_enable")) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrGetD3D12GraphicsRequirementsKHR-extension-notenabled",
                                                "xrGetD3D12GraphicsRequirementsKHR",
                                                "The XR_KHR_D3D12_enable extension has not been enabled prior to calling xrGetD3D12GraphicsRequirementsKHR");
        return XR_ERROR_FUNCTION_UNSUPPORTED;
    }

    return dispatch_table->GetD3D12GraphicsRequirementsKHR(instance, systemId, graphicsRequirements);
}
XRLOADER_ABI_CATCH_FALLBACK

#endif // defined(XR_USE_GRAPHICS_API_D3D12)


// ---- XR_KHR_visibility_mask extension commands
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrGetVisibilityMaskKHR(
    XrSession                                   session,
    XrViewConfigurationType                     viewConfigurationType,
    uint32_t                                    viewIndex,
    XrVisibilityMaskTypeKHR                     visibilityMaskType,
    XrVisibilityMaskKHR*                        visibilityMask) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrGetVisibilityMaskKHR-session-parameter",
                "xrGetVisibilityMaskKHR",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    if (!loader_instance->ExtensionIsEnabled("XR_KHR_visibility_mask")) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrGetVisibilityMaskKHR-extension-notenabled",
                                                "xrGetVisibilityMaskKHR",
                                                "The XR_KHR_visibility_mask extension has not been enabled prior to calling xrGetVisibilityMaskKHR");
        return XR_ERROR_FUNCTION_UNSUPPORTED;
    }

    return dispatch_table->GetVisibilityMaskKHR(session, viewConfigurationType, viewIndex, visibilityMaskType, visibilityMask);
}
XRLOADER_ABI_CATCH_FALLBACK



// ---- XR_KHR_win32_convert_performance_counter_time extension commands
#if defined(XR_USE_PLATFORM_WIN32)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrConvertWin32PerformanceCounterToTimeKHR(
    XrInstance                                  instance,
    const LARGE_INTEGER*                        performanceCounter,
    XrTime*                                     time) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrConvertWin32PerformanceCounterToTimeKHR-instance-parameter",
                "xrConvertWin32PerformanceCounterToTimeKHR",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    if (!loader_instance->ExtensionIsEnabled("XR_KHR_win32_convert_performance_counter_time")) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrConvertWin32PerformanceCounterToTimeKHR-extension-notenabled",
                                                "xrConvertWin32PerformanceCounterToTimeKHR",
                                                "The XR_KHR_win32_convert_performance_counter_time extension has not been enabled prior to calling xrConvertWin32PerformanceCounterToTimeKHR");
        return XR_ERROR_FUNCTION_UNSUPPORTED;
    }

    return dispatch_table->ConvertWin32PerformanceCounterToTimeKHR(instance, performanceCounter, time);
}
XRLOADER_ABI_CATCH_FALLBACK

#endif // defined(XR_USE_PLATFORM_WIN32)

#if defined(XR_USE_PLATFORM_WIN32)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrConvertTimeToWin32PerformanceCounterKHR(
    XrInstance                                  instance,
    XrTime                                      time,
    LARGE_INTEGER*                              performanceCounter) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrConvertTimeToWin32PerformanceCounterKHR-instance-parameter",
                "xrConvertTimeToWin32PerformanceCounterKHR",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    if (!loader_instance->ExtensionIsEnabled("XR_KHR_win32_convert_performance_counter_time")) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrConvertTimeToWin32PerformanceCounterKHR-extension-notenabled",
                                                "xrConvertTimeToWin32PerformanceCounterKHR",
                                                "The XR_KHR_win32_convert_performance_counter_time extension has not been enabled prior to calling xrConvertTimeToWin32PerformanceCounterKHR");
        return XR_ERROR_FUNCTION_UNSUPPORTED;
    }

    return dispatch_table->ConvertTimeToWin32PerformanceCounterKHR(instance, time, performanceCounter);
}
XRLOADER_ABI_CATCH_FALLBACK

#endif // defined(XR_USE_PLATFORM_WIN32)


// ---- XR_KHR_convert_timespec_time extension commands
#if defined(XR_USE_TIMESPEC)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrConvertTimespecTimeToTimeKHR(
    XrInstance                                  instance,
    const struct timespec*                      timespecTime,
    XrTime*                                     time) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrConvertTimespecTimeToTimeKHR-instance-parameter",
                "xrConvertTimespecTimeToTimeKHR",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    if (!loader_instance->ExtensionIsEnabled("XR_KHR_convert_timespec_time")) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrConvertTimespecTimeToTimeKHR-extension-notenabled",
                                                "xrConvertTimespecTimeToTimeKHR",
                                                "The XR_KHR_convert_timespec_time extension has not been enabled prior to calling xrConvertTimespecTimeToTimeKHR");
        return XR_ERROR_FUNCTION_UNSUPPORTED;
    }

    return dispatch_table->ConvertTimespecTimeToTimeKHR(instance, timespecTime, time);
}
XRLOADER_ABI_CATCH_FALLBACK

#endif // defined(XR_USE_TIMESPEC)

#if defined(XR_USE_TIMESPEC)
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrConvertTimeToTimespecTimeKHR(
    XrInstance                                  instance,
    XrTime                                      time,
    struct timespec*                            timespecTime) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrConvertTimeToTimespecTimeKHR-instance-parameter",
                "xrConvertTimeToTimespecTimeKHR",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    if (!loader_instance->ExtensionIsEnabled("XR_KHR_convert_timespec_time")) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrConvertTimeToTimespecTimeKHR-extension-notenabled",
                                                "xrConvertTimeToTimespecTimeKHR",
                                                "The XR_KHR_convert_timespec_time extension has not been enabled prior to calling xrConvertTimeToTimespecTimeKHR");
        return XR_ERROR_FUNCTION_UNSUPPORTED;
    }

    return dispatch_table->ConvertTimeToTimespecTimeKHR(instance, time, timespecTime);
}
XRLOADER_ABI_CATCH_FALLBACK

#endif // defined(XR_USE_TIMESPEC)


// ---- XR_EXT_performance_settings extension commands
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrPerfSettingsSetPerformanceLevelEXT(
    XrSession                                   session,
    XrPerfSettingsDomainEXT                     domain,
    XrPerfSettingsLevelEXT                      level) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrPerfSettingsSetPerformanceLevelEXT-session-parameter",
                "xrPerfSettingsSetPerformanceLevelEXT",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    if (!loader_instance->ExtensionIsEnabled("XR_EXT_performance_settings")) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrPerfSettingsSetPerformanceLevelEXT-extension-notenabled",
                                                "xrPerfSettingsSetPerformanceLevelEXT",
                                                "The XR_EXT_performance_settings extension has not been enabled prior to calling xrPerfSettingsSetPerformanceLevelEXT");
        return XR_ERROR_FUNCTION_UNSUPPORTED;
    }

    return dispatch_table->PerfSettingsSetPerformanceLevelEXT(session, domain, level);
}
XRLOADER_ABI_CATCH_FALLBACK



// ---- XR_EXT_thermal_query extension commands
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrThermalGetTemperatureTrendEXT(
    XrSession                                   session,
    XrPerfSettingsDomainEXT                     domain,
    XrPerfSettingsNotificationLevelEXT*         notificationLevel,
    float*                                      tempHeadroom,
    float*                                      tempSlope) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrThermalGetTemperatureTrendEXT-session-parameter",
                "xrThermalGetTemperatureTrendEXT",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    if (!loader_instance->ExtensionIsEnabled("XR_EXT_thermal_query")) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrThermalGetTemperatureTrendEXT-extension-notenabled",
                                                "xrThermalGetTemperatureTrendEXT",
                                                "The XR_EXT_thermal_query extension has not been enabled prior to calling xrThermalGetTemperatureTrendEXT");
        return XR_ERROR_FUNCTION_UNSUPPORTED;
    }

    return dispatch_table->ThermalGetTemperatureTrendEXT(session, domain, notificationLevel, tempHeadroom, tempSlope);
}
XRLOADER_ABI_CATCH_FALLBACK



// ---- XR_EXT_debug_utils extension commands
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrSetDebugUtilsObjectNameEXT(
    XrInstance                                  instance,
    const XrDebugUtilsObjectNameInfoEXT*        nameInfo) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrSetDebugUtilsObjectNameEXT-instance-parameter",
                "xrSetDebugUtilsObjectNameEXT",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    if (!loader_instance->ExtensionIsEnabled("XR_EXT_debug_utils")) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrSetDebugUtilsObjectNameEXT-extension-notenabled",
                                                "xrSetDebugUtilsObjectNameEXT",
                                                "The XR_EXT_debug_utils extension has not been enabled prior to calling xrSetDebugUtilsObjectNameEXT");
        return XR_ERROR_FUNCTION_UNSUPPORTED;
    }

    return dispatch_table->SetDebugUtilsObjectNameEXT(instance, nameInfo);
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" XRAPI_ATTR XrResult                                    XRAPI_CALL xrSubmitDebugUtilsMessageEXT(
    XrInstance                                  instance,
    XrDebugUtilsMessageSeverityFlagsEXT         messageSeverity,
    XrDebugUtilsMessageTypeFlagsEXT             messageTypes,
    const XrDebugUtilsMessengerCallbackDataEXT* callbackData) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_instance_map.Get(instance);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrSubmitDebugUtilsMessageEXT-instance-parameter",
                "xrSubmitDebugUtilsMessageEXT",
                "instance is not a valid XrInstance",
                {XrSdkLogObjectInfo{instance, XR_OBJECT_TYPE_INSTANCE}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
    if (!loader_instance->ExtensionIsEnabled("XR_EXT_debug_utils")) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrSubmitDebugUtilsMessageEXT-extension-notenabled",
                                                "xrSubmitDebugUtilsMessageEXT",
                                                "The XR_EXT_debug_utils extension has not been enabled prior to calling xrSubmitDebugUtilsMessageEXT");
        return XR_ERROR_FUNCTION_UNSUPPORTED;
    }

    return dispatch_table->SubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, callbackData);
}
XRLOADER_ABI_CATCH_FALLBACK



// ---- XR_MSFT_spatial_anchor extension commands
extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrCreateSpatialAnchorMSFT(
    XrSession                                   session,
    const XrSpatialAnchorCreateInfoMSFT*        createInfo,
    XrSpatialAnchorMSFT*                        anchor) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrCreateSpatialAnchorMSFT-session-parameter",
                "xrCreateSpatialAnchorMSFT",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
    if (!loader_instance->ExtensionIsEnabled("XR_MSFT_spatial_anchor")) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrCreateSpatialAnchorMSFT-extension-notenabled",
                                                "xrCreateSpatialAnchorMSFT",
                                                "The XR_MSFT_spatial_anchor extension has not been enabled prior to calling xrCreateSpatialAnchorMSFT");
        return XR_ERROR_FUNCTION_UNSUPPORTED;
    }

    result = dispatch_table->CreateSpatialAnchorMSFT(session, createInfo, anchor);
        if (XR_SUCCESS == result && nullptr != anchor) {
            XrResult insert_result = g_spatialanchormsft_map.Insert(*anchor, *loader_instance);
            if (XR_FAILED(insert_result)) {
                LoaderLogger::LogErrorMessage(
                    "xrCreateSpatialAnchorMSFT",
                    "Failed inserting new spatialanchormsft into map: may be null or not unique");
            }
        }
    return result;
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrCreateSpatialAnchorSpaceMSFT(
    XrSession                                   session,
    const XrSpatialAnchorSpaceCreateInfoMSFT*   createInfo,
    XrSpace*                                    space) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_session_map.Get(session);
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrCreateSpatialAnchorSpaceMSFT-session-parameter",
                "xrCreateSpatialAnchorSpaceMSFT",
                "session is not a valid XrSession",
                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
    if (!loader_instance->ExtensionIsEnabled("XR_MSFT_spatial_anchor")) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrCreateSpatialAnchorSpaceMSFT-extension-notenabled",
                                                "xrCreateSpatialAnchorSpaceMSFT",
                                                "The XR_MSFT_spatial_anchor extension has not been enabled prior to calling xrCreateSpatialAnchorSpaceMSFT");
        return XR_ERROR_FUNCTION_UNSUPPORTED;
    }

    result = dispatch_table->CreateSpatialAnchorSpaceMSFT(session, createInfo, space);
        if (XR_SUCCESS == result && nullptr != space) {
            XrResult insert_result = g_space_map.Insert(*space, *loader_instance);
            if (XR_FAILED(insert_result)) {
                LoaderLogger::LogErrorMessage(
                    "xrCreateSpatialAnchorSpaceMSFT",
                    "Failed inserting new space into map: may be null or not unique");
            }
        }
    return result;
}
XRLOADER_ABI_CATCH_FALLBACK


extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrDestroySpatialAnchorMSFT(
    XrSpatialAnchorMSFT                         anchor) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance = g_spatialanchormsft_map.Get(anchor);
    // Destroy the mapping entry for this item if it was valid.
    if (nullptr != loader_instance) {
            g_spatialanchormsft_map.Erase(anchor);
    }
    if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage(
                "VUID-xrDestroySpatialAnchorMSFT-anchor-parameter",
                "xrDestroySpatialAnchorMSFT",
                "anchor is not a valid XrSpatialAnchorMSFT",
                {XrSdkLogObjectInfo{anchor, XR_OBJECT_TYPE_SPATIAL_ANCHOR_MSFT}});
        return XR_ERROR_HANDLE_INVALID;
    }
    const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
    if (!loader_instance->ExtensionIsEnabled("XR_MSFT_spatial_anchor")) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrDestroySpatialAnchorMSFT-extension-notenabled",
                                                "xrDestroySpatialAnchorMSFT",
                                                "The XR_MSFT_spatial_anchor extension has not been enabled prior to calling xrDestroySpatialAnchorMSFT");
        return XR_ERROR_FUNCTION_UNSUPPORTED;
    }

    result = dispatch_table->DestroySpatialAnchorMSFT(anchor);
    return result;
}
XRLOADER_ABI_CATCH_FALLBACK



LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetInstanceProcAddr(XrInstance instance, const char* name,
                                                                   PFN_xrVoidFunction* function) XRLOADER_ABI_TRY {
    if (nullptr == function) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrGetInstanceProcAddr-function-parameter",
                                                "xrGetInstanceProcAddr", "Invalid Function pointer");
            return XR_ERROR_VALIDATION_FAILURE;
    }
    if (nullptr == name) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrGetInstanceProcAddr-function-parameter",
                                                "xrGetInstanceProcAddr", "Invalid Name pointer");
            return XR_ERROR_VALIDATION_FAILURE;
    }
    // Initialize the function to nullptr in case it does not get caught in a known case
    *function = nullptr;
    if (name[0] == 'x' && name[1] == 'r') {
        std::string func_name = &name[2];
        LoaderInstance * const loader_instance = g_instance_map.Get(instance);
        if (instance == XR_NULL_HANDLE) {
            // Null instance is allowed for 3 specific API entry points, otherwise return error
            if (!((func_name == "CreateInstance") ||
                  (func_name == "EnumerateApiLayerProperties") ||
                  (func_name == "EnumerateInstanceExtensionProperties"))) {
                std::string error_str = "XR_NULL_HANDLE for instance but query for ";
                error_str += name;
                error_str += " requires a valid instance";
                LoaderLogger::LogValidationErrorMessage("VUID-xrGetInstanceProcAddr-instance-parameter",
                                                        "xrGetInstanceProcAddr", error_str);
                return XR_ERROR_HANDLE_INVALID;
            }
        }
        else if (loader_instance == nullptr) {
            std::string error_str = "Invalid handle for instance (query for ";
            error_str += name;
            error_str += " )";
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetInstanceProcAddr-instance-parameter",
                                                    "xrGetInstanceProcAddr", error_str);
            return XR_ERROR_HANDLE_INVALID;
        }

        // ---- Core 1.0 commands

        if (func_name == "GetInstanceProcAddr") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetInstanceProcAddr);
        } else if (func_name == "EnumerateApiLayerProperties") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrEnumerateApiLayerProperties);
        } else if (func_name == "EnumerateInstanceExtensionProperties") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrEnumerateInstanceExtensionProperties);
        } else if (func_name == "CreateInstance") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrCreateInstance);
        } else if (func_name == "DestroyInstance") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrDestroyInstance);
        } else if (func_name == "GetInstanceProperties") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetInstanceProperties);
        } else if (func_name == "PollEvent") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrPollEvent);
        } else if (func_name == "ResultToString") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrResultToString);
        } else if (func_name == "StructureTypeToString") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrStructureTypeToString);
        } else if (func_name == "GetSystem") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetSystem);
        } else if (func_name == "GetSystemProperties") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetSystemProperties);
        } else if (func_name == "EnumerateEnvironmentBlendModes") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrEnumerateEnvironmentBlendModes);
        } else if (func_name == "CreateSession") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrCreateSession);
        } else if (func_name == "DestroySession") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->DestroySession);
        } else if (func_name == "EnumerateReferenceSpaces") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->EnumerateReferenceSpaces);
        } else if (func_name == "CreateReferenceSpace") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->CreateReferenceSpace);
        } else if (func_name == "GetReferenceSpaceBoundsRect") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->GetReferenceSpaceBoundsRect);
        } else if (func_name == "CreateActionSpace") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->CreateActionSpace);
        } else if (func_name == "LocateSpace") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->LocateSpace);
        } else if (func_name == "DestroySpace") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->DestroySpace);
        } else if (func_name == "EnumerateViewConfigurations") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrEnumerateViewConfigurations);
        } else if (func_name == "GetViewConfigurationProperties") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetViewConfigurationProperties);
        } else if (func_name == "EnumerateViewConfigurationViews") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrEnumerateViewConfigurationViews);
        } else if (func_name == "EnumerateSwapchainFormats") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->EnumerateSwapchainFormats);
        } else if (func_name == "CreateSwapchain") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->CreateSwapchain);
        } else if (func_name == "DestroySwapchain") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->DestroySwapchain);
        } else if (func_name == "EnumerateSwapchainImages") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->EnumerateSwapchainImages);
        } else if (func_name == "AcquireSwapchainImage") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->AcquireSwapchainImage);
        } else if (func_name == "WaitSwapchainImage") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->WaitSwapchainImage);
        } else if (func_name == "ReleaseSwapchainImage") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->ReleaseSwapchainImage);
        } else if (func_name == "BeginSession") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->BeginSession);
        } else if (func_name == "EndSession") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->EndSession);
        } else if (func_name == "RequestExitSession") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->RequestExitSession);
        } else if (func_name == "WaitFrame") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->WaitFrame);
        } else if (func_name == "BeginFrame") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->BeginFrame);
        } else if (func_name == "EndFrame") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->EndFrame);
        } else if (func_name == "LocateViews") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->LocateViews);
        } else if (func_name == "StringToPath") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrStringToPath);
        } else if (func_name == "PathToString") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrPathToString);
        } else if (func_name == "CreateActionSet") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrCreateActionSet);
        } else if (func_name == "DestroyActionSet") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->DestroyActionSet);
        } else if (func_name == "CreateAction") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->CreateAction);
        } else if (func_name == "DestroyAction") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->DestroyAction);
        } else if (func_name == "SuggestInteractionProfileBindings") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrSuggestInteractionProfileBindings);
        } else if (func_name == "AttachSessionActionSets") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->AttachSessionActionSets);
        } else if (func_name == "GetCurrentInteractionProfile") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->GetCurrentInteractionProfile);
        } else if (func_name == "GetActionStateBoolean") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->GetActionStateBoolean);
        } else if (func_name == "GetActionStateFloat") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->GetActionStateFloat);
        } else if (func_name == "GetActionStateVector2f") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->GetActionStateVector2f);
        } else if (func_name == "GetActionStatePose") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->GetActionStatePose);
        } else if (func_name == "SyncActions") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->SyncActions);
        } else if (func_name == "EnumerateBoundSourcesForAction") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->EnumerateBoundSourcesForAction);
        } else if (func_name == "GetInputSourceLocalizedName") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->GetInputSourceLocalizedName);
        } else if (func_name == "ApplyHapticFeedback") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->ApplyHapticFeedback);
        } else if (func_name == "StopHapticFeedback") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->StopHapticFeedback);

        // ---- XR_KHR_android_thread_settings extension commands

#if defined(XR_USE_PLATFORM_ANDROID)
        } else if (func_name == "SetAndroidApplicationThreadKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_android_thread_settings")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->SetAndroidApplicationThreadKHR);
            }
#endif // defined(XR_USE_PLATFORM_ANDROID)

        // ---- XR_KHR_android_surface_swapchain extension commands

#if defined(XR_USE_PLATFORM_ANDROID)
        } else if (func_name == "CreateSwapchainAndroidSurfaceKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_android_surface_swapchain")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->CreateSwapchainAndroidSurfaceKHR);
            }
#endif // defined(XR_USE_PLATFORM_ANDROID)

        // ---- XR_KHR_opengl_enable extension commands

#if defined(XR_USE_GRAPHICS_API_OPENGL)
        } else if (func_name == "GetOpenGLGraphicsRequirementsKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_opengl_enable")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetOpenGLGraphicsRequirementsKHR);
            }
#endif // defined(XR_USE_GRAPHICS_API_OPENGL)

        // ---- XR_KHR_opengl_es_enable extension commands

#if defined(XR_USE_GRAPHICS_API_OPENGL_ES)
        } else if (func_name == "GetOpenGLESGraphicsRequirementsKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_opengl_es_enable")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetOpenGLESGraphicsRequirementsKHR);
            }
#endif // defined(XR_USE_GRAPHICS_API_OPENGL_ES)

        // ---- XR_KHR_vulkan_enable extension commands

#if defined(XR_USE_GRAPHICS_API_VULKAN)
        } else if (func_name == "GetVulkanInstanceExtensionsKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_vulkan_enable")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetVulkanInstanceExtensionsKHR);
            }
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
        } else if (func_name == "GetVulkanDeviceExtensionsKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_vulkan_enable")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetVulkanDeviceExtensionsKHR);
            }
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
        } else if (func_name == "GetVulkanGraphicsDeviceKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_vulkan_enable")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetVulkanGraphicsDeviceKHR);
            }
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
        } else if (func_name == "GetVulkanGraphicsRequirementsKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_vulkan_enable")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetVulkanGraphicsRequirementsKHR);
            }
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)

        // ---- XR_KHR_D3D11_enable extension commands

#if defined(XR_USE_GRAPHICS_API_D3D11)
        } else if (func_name == "GetD3D11GraphicsRequirementsKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_D3D11_enable")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetD3D11GraphicsRequirementsKHR);
            }
#endif // defined(XR_USE_GRAPHICS_API_D3D11)

        // ---- XR_KHR_D3D12_enable extension commands

#if defined(XR_USE_GRAPHICS_API_D3D12)
        } else if (func_name == "GetD3D12GraphicsRequirementsKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_D3D12_enable")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetD3D12GraphicsRequirementsKHR);
            }
#endif // defined(XR_USE_GRAPHICS_API_D3D12)

        // ---- XR_KHR_visibility_mask extension commands

        } else if (func_name == "GetVisibilityMaskKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_visibility_mask")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->GetVisibilityMaskKHR);
            }

        // ---- XR_KHR_win32_convert_performance_counter_time extension commands

#if defined(XR_USE_PLATFORM_WIN32)
        } else if (func_name == "ConvertWin32PerformanceCounterToTimeKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_win32_convert_performance_counter_time")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrConvertWin32PerformanceCounterToTimeKHR);
            }
#endif // defined(XR_USE_PLATFORM_WIN32)
#if defined(XR_USE_PLATFORM_WIN32)
        } else if (func_name == "ConvertTimeToWin32PerformanceCounterKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_win32_convert_performance_counter_time")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrConvertTimeToWin32PerformanceCounterKHR);
            }
#endif // defined(XR_USE_PLATFORM_WIN32)

        // ---- XR_KHR_convert_timespec_time extension commands

#if defined(XR_USE_TIMESPEC)
        } else if (func_name == "ConvertTimespecTimeToTimeKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_convert_timespec_time")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrConvertTimespecTimeToTimeKHR);
            }
#endif // defined(XR_USE_TIMESPEC)
#if defined(XR_USE_TIMESPEC)
        } else if (func_name == "ConvertTimeToTimespecTimeKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_convert_timespec_time")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrConvertTimeToTimespecTimeKHR);
            }
#endif // defined(XR_USE_TIMESPEC)

        // ---- XR_EXT_performance_settings extension commands

        } else if (func_name == "PerfSettingsSetPerformanceLevelEXT") {
            if (loader_instance->ExtensionIsEnabled("XR_EXT_performance_settings")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->PerfSettingsSetPerformanceLevelEXT);
            }

        // ---- XR_EXT_thermal_query extension commands

        } else if (func_name == "ThermalGetTemperatureTrendEXT") {
            if (loader_instance->ExtensionIsEnabled("XR_EXT_thermal_query")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->ThermalGetTemperatureTrendEXT);
            }

        // ---- XR_EXT_debug_utils extension commands

        } else if (func_name == "SetDebugUtilsObjectNameEXT") {
            if (loader_instance->ExtensionIsEnabled("XR_EXT_debug_utils")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrSetDebugUtilsObjectNameEXT);
            }
        } else if (func_name == "CreateDebugUtilsMessengerEXT") {
            if (loader_instance->ExtensionIsEnabled("XR_EXT_debug_utils")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrCreateDebugUtilsMessengerEXT);
            }
        } else if (func_name == "DestroyDebugUtilsMessengerEXT") {
            if (loader_instance->ExtensionIsEnabled("XR_EXT_debug_utils")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrDestroyDebugUtilsMessengerEXT);
            }
        } else if (func_name == "SubmitDebugUtilsMessageEXT") {
            if (loader_instance->ExtensionIsEnabled("XR_EXT_debug_utils")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrSubmitDebugUtilsMessageEXT);
            }
        } else if (func_name == "SessionBeginDebugUtilsLabelRegionEXT") {
            if (loader_instance->ExtensionIsEnabled("XR_EXT_debug_utils")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrSessionBeginDebugUtilsLabelRegionEXT);
            }
        } else if (func_name == "SessionEndDebugUtilsLabelRegionEXT") {
            if (loader_instance->ExtensionIsEnabled("XR_EXT_debug_utils")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrSessionEndDebugUtilsLabelRegionEXT);
            }
        } else if (func_name == "SessionInsertDebugUtilsLabelEXT") {
            if (loader_instance->ExtensionIsEnabled("XR_EXT_debug_utils")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrSessionInsertDebugUtilsLabelEXT);
            }

        // ---- XR_MSFT_spatial_anchor extension commands

        } else if (func_name == "CreateSpatialAnchorMSFT") {
            if (loader_instance->ExtensionIsEnabled("XR_MSFT_spatial_anchor")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->CreateSpatialAnchorMSFT);
            }
        } else if (func_name == "CreateSpatialAnchorSpaceMSFT") {
            if (loader_instance->ExtensionIsEnabled("XR_MSFT_spatial_anchor")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->CreateSpatialAnchorSpaceMSFT);
            }
        } else if (func_name == "DestroySpatialAnchorMSFT") {
            if (loader_instance->ExtensionIsEnabled("XR_MSFT_spatial_anchor")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->DestroySpatialAnchorMSFT);
            }
        }
    }
    if (*function == nullptr) {
        return XR_ERROR_FUNCTION_UNSUPPORTED;
    }
    return XR_SUCCESS;
}
XRLOADER_ABI_CATCH_FALLBACK

// Terminator GetInstanceProcAddr function
XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermGetInstanceProcAddr(XrInstance instance, const char* name,
                                                               PFN_xrVoidFunction* function) XRLOADER_ABI_TRY {

    // A few instance commands need to go through a loader terminator.
    // Otherwise, go directly to the runtime version of the command if it exists.
    // But first set the function pointer to NULL so that the fall-through below actually works.
    *function = nullptr;

    if (0 == strcmp(name, "xrGetInstanceProcAddr")) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(LoaderXrTermGetInstanceProcAddr);
    } else if (0 == strcmp(name, "xrCreateInstance")) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(LoaderXrTermCreateInstance);
    } else if (0 == strcmp(name, "xrDestroyInstance")) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(LoaderXrTermDestroyInstance);
    } else if (0 == strcmp(name, "xrSetDebugUtilsObjectNameEXT")) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(LoaderXrTermSetDebugUtilsObjectNameEXT);
    } else if (0 == strcmp(name, "xrCreateDebugUtilsMessengerEXT")) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(LoaderXrTermCreateDebugUtilsMessengerEXT);
    } else if (0 == strcmp(name, "xrDestroyDebugUtilsMessengerEXT")) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(LoaderXrTermDestroyDebugUtilsMessengerEXT);
    } else if (0 == strcmp(name, "xrSubmitDebugUtilsMessageEXT")) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(LoaderXrTermSubmitDebugUtilsMessageEXT);
    } else if (0 == strcmp(name, "xrCreateApiLayerInstance")) {
        // Special layer version of xrCreateInstance terminator.  If we get called this by a layer,
        // we simply re-direct the information back into the standard xrCreateInstance terminator.
        *function = reinterpret_cast<PFN_xrVoidFunction>(LoaderXrTermCreateApiLayerInstance);
    }
    if (nullptr != *function) {
        return XR_SUCCESS;
    }
    return RuntimeInterface::GetInstanceProcAddr(instance, name, function);
}
XRLOADER_ABI_CATCH_FALLBACK

// Instance Init Dispatch Table (put all terminators in first)
void LoaderGenInitInstanceDispatchTable(XrInstance instance, std::unique_ptr<XrGeneratedDispatchTable>& table) {

    // ---- Core 1_0 commands
    LoaderXrTermGetInstanceProcAddr(instance, "xrGetInstanceProcAddr", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetInstanceProcAddr));
    table->EnumerateApiLayerProperties = nullptr;
    table->EnumerateInstanceExtensionProperties = nullptr;
    LoaderXrTermGetInstanceProcAddr(instance, "xrCreateInstance", reinterpret_cast<PFN_xrVoidFunction*>(&table->CreateInstance));
    LoaderXrTermGetInstanceProcAddr(instance, "xrDestroyInstance", reinterpret_cast<PFN_xrVoidFunction*>(&table->DestroyInstance));
    LoaderXrTermGetInstanceProcAddr(instance, "xrGetInstanceProperties", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetInstanceProperties));
    LoaderXrTermGetInstanceProcAddr(instance, "xrPollEvent", reinterpret_cast<PFN_xrVoidFunction*>(&table->PollEvent));
    LoaderXrTermGetInstanceProcAddr(instance, "xrResultToString", reinterpret_cast<PFN_xrVoidFunction*>(&table->ResultToString));
    LoaderXrTermGetInstanceProcAddr(instance, "xrStructureTypeToString", reinterpret_cast<PFN_xrVoidFunction*>(&table->StructureTypeToString));
    LoaderXrTermGetInstanceProcAddr(instance, "xrGetSystem", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetSystem));
    LoaderXrTermGetInstanceProcAddr(instance, "xrGetSystemProperties", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetSystemProperties));
    LoaderXrTermGetInstanceProcAddr(instance, "xrEnumerateEnvironmentBlendModes", reinterpret_cast<PFN_xrVoidFunction*>(&table->EnumerateEnvironmentBlendModes));
    LoaderXrTermGetInstanceProcAddr(instance, "xrCreateSession", reinterpret_cast<PFN_xrVoidFunction*>(&table->CreateSession));
    LoaderXrTermGetInstanceProcAddr(instance, "xrDestroySession", reinterpret_cast<PFN_xrVoidFunction*>(&table->DestroySession));
    LoaderXrTermGetInstanceProcAddr(instance, "xrEnumerateReferenceSpaces", reinterpret_cast<PFN_xrVoidFunction*>(&table->EnumerateReferenceSpaces));
    LoaderXrTermGetInstanceProcAddr(instance, "xrCreateReferenceSpace", reinterpret_cast<PFN_xrVoidFunction*>(&table->CreateReferenceSpace));
    LoaderXrTermGetInstanceProcAddr(instance, "xrGetReferenceSpaceBoundsRect", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetReferenceSpaceBoundsRect));
    LoaderXrTermGetInstanceProcAddr(instance, "xrCreateActionSpace", reinterpret_cast<PFN_xrVoidFunction*>(&table->CreateActionSpace));
    LoaderXrTermGetInstanceProcAddr(instance, "xrLocateSpace", reinterpret_cast<PFN_xrVoidFunction*>(&table->LocateSpace));
    LoaderXrTermGetInstanceProcAddr(instance, "xrDestroySpace", reinterpret_cast<PFN_xrVoidFunction*>(&table->DestroySpace));
    LoaderXrTermGetInstanceProcAddr(instance, "xrEnumerateViewConfigurations", reinterpret_cast<PFN_xrVoidFunction*>(&table->EnumerateViewConfigurations));
    LoaderXrTermGetInstanceProcAddr(instance, "xrGetViewConfigurationProperties", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetViewConfigurationProperties));
    LoaderXrTermGetInstanceProcAddr(instance, "xrEnumerateViewConfigurationViews", reinterpret_cast<PFN_xrVoidFunction*>(&table->EnumerateViewConfigurationViews));
    LoaderXrTermGetInstanceProcAddr(instance, "xrEnumerateSwapchainFormats", reinterpret_cast<PFN_xrVoidFunction*>(&table->EnumerateSwapchainFormats));
    LoaderXrTermGetInstanceProcAddr(instance, "xrCreateSwapchain", reinterpret_cast<PFN_xrVoidFunction*>(&table->CreateSwapchain));
    LoaderXrTermGetInstanceProcAddr(instance, "xrDestroySwapchain", reinterpret_cast<PFN_xrVoidFunction*>(&table->DestroySwapchain));
    LoaderXrTermGetInstanceProcAddr(instance, "xrEnumerateSwapchainImages", reinterpret_cast<PFN_xrVoidFunction*>(&table->EnumerateSwapchainImages));
    LoaderXrTermGetInstanceProcAddr(instance, "xrAcquireSwapchainImage", reinterpret_cast<PFN_xrVoidFunction*>(&table->AcquireSwapchainImage));
    LoaderXrTermGetInstanceProcAddr(instance, "xrWaitSwapchainImage", reinterpret_cast<PFN_xrVoidFunction*>(&table->WaitSwapchainImage));
    LoaderXrTermGetInstanceProcAddr(instance, "xrReleaseSwapchainImage", reinterpret_cast<PFN_xrVoidFunction*>(&table->ReleaseSwapchainImage));
    LoaderXrTermGetInstanceProcAddr(instance, "xrBeginSession", reinterpret_cast<PFN_xrVoidFunction*>(&table->BeginSession));
    LoaderXrTermGetInstanceProcAddr(instance, "xrEndSession", reinterpret_cast<PFN_xrVoidFunction*>(&table->EndSession));
    LoaderXrTermGetInstanceProcAddr(instance, "xrRequestExitSession", reinterpret_cast<PFN_xrVoidFunction*>(&table->RequestExitSession));
    LoaderXrTermGetInstanceProcAddr(instance, "xrWaitFrame", reinterpret_cast<PFN_xrVoidFunction*>(&table->WaitFrame));
    LoaderXrTermGetInstanceProcAddr(instance, "xrBeginFrame", reinterpret_cast<PFN_xrVoidFunction*>(&table->BeginFrame));
    LoaderXrTermGetInstanceProcAddr(instance, "xrEndFrame", reinterpret_cast<PFN_xrVoidFunction*>(&table->EndFrame));
    LoaderXrTermGetInstanceProcAddr(instance, "xrLocateViews", reinterpret_cast<PFN_xrVoidFunction*>(&table->LocateViews));
    LoaderXrTermGetInstanceProcAddr(instance, "xrStringToPath", reinterpret_cast<PFN_xrVoidFunction*>(&table->StringToPath));
    LoaderXrTermGetInstanceProcAddr(instance, "xrPathToString", reinterpret_cast<PFN_xrVoidFunction*>(&table->PathToString));
    LoaderXrTermGetInstanceProcAddr(instance, "xrCreateActionSet", reinterpret_cast<PFN_xrVoidFunction*>(&table->CreateActionSet));
    LoaderXrTermGetInstanceProcAddr(instance, "xrDestroyActionSet", reinterpret_cast<PFN_xrVoidFunction*>(&table->DestroyActionSet));
    LoaderXrTermGetInstanceProcAddr(instance, "xrCreateAction", reinterpret_cast<PFN_xrVoidFunction*>(&table->CreateAction));
    LoaderXrTermGetInstanceProcAddr(instance, "xrDestroyAction", reinterpret_cast<PFN_xrVoidFunction*>(&table->DestroyAction));
    LoaderXrTermGetInstanceProcAddr(instance, "xrSuggestInteractionProfileBindings", reinterpret_cast<PFN_xrVoidFunction*>(&table->SuggestInteractionProfileBindings));
    LoaderXrTermGetInstanceProcAddr(instance, "xrAttachSessionActionSets", reinterpret_cast<PFN_xrVoidFunction*>(&table->AttachSessionActionSets));
    LoaderXrTermGetInstanceProcAddr(instance, "xrGetCurrentInteractionProfile", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetCurrentInteractionProfile));
    LoaderXrTermGetInstanceProcAddr(instance, "xrGetActionStateBoolean", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetActionStateBoolean));
    LoaderXrTermGetInstanceProcAddr(instance, "xrGetActionStateFloat", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetActionStateFloat));
    LoaderXrTermGetInstanceProcAddr(instance, "xrGetActionStateVector2f", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetActionStateVector2f));
    LoaderXrTermGetInstanceProcAddr(instance, "xrGetActionStatePose", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetActionStatePose));
    LoaderXrTermGetInstanceProcAddr(instance, "xrSyncActions", reinterpret_cast<PFN_xrVoidFunction*>(&table->SyncActions));
    LoaderXrTermGetInstanceProcAddr(instance, "xrEnumerateBoundSourcesForAction", reinterpret_cast<PFN_xrVoidFunction*>(&table->EnumerateBoundSourcesForAction));
    LoaderXrTermGetInstanceProcAddr(instance, "xrGetInputSourceLocalizedName", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetInputSourceLocalizedName));
    LoaderXrTermGetInstanceProcAddr(instance, "xrApplyHapticFeedback", reinterpret_cast<PFN_xrVoidFunction*>(&table->ApplyHapticFeedback));
    LoaderXrTermGetInstanceProcAddr(instance, "xrStopHapticFeedback", reinterpret_cast<PFN_xrVoidFunction*>(&table->StopHapticFeedback));

    // ---- XR_KHR_android_thread_settings extension commands
#if defined(XR_USE_PLATFORM_ANDROID)
    LoaderXrTermGetInstanceProcAddr(instance, "xrSetAndroidApplicationThreadKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->SetAndroidApplicationThreadKHR));
#endif // defined(XR_USE_PLATFORM_ANDROID)

    // ---- XR_KHR_android_surface_swapchain extension commands
#if defined(XR_USE_PLATFORM_ANDROID)
    LoaderXrTermGetInstanceProcAddr(instance, "xrCreateSwapchainAndroidSurfaceKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->CreateSwapchainAndroidSurfaceKHR));
#endif // defined(XR_USE_PLATFORM_ANDROID)

    // ---- XR_KHR_opengl_enable extension commands
#if defined(XR_USE_GRAPHICS_API_OPENGL)
    LoaderXrTermGetInstanceProcAddr(instance, "xrGetOpenGLGraphicsRequirementsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetOpenGLGraphicsRequirementsKHR));
#endif // defined(XR_USE_GRAPHICS_API_OPENGL)

    // ---- XR_KHR_opengl_es_enable extension commands
#if defined(XR_USE_GRAPHICS_API_OPENGL_ES)
    LoaderXrTermGetInstanceProcAddr(instance, "xrGetOpenGLESGraphicsRequirementsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetOpenGLESGraphicsRequirementsKHR));
#endif // defined(XR_USE_GRAPHICS_API_OPENGL_ES)

    // ---- XR_KHR_vulkan_enable extension commands
#if defined(XR_USE_GRAPHICS_API_VULKAN)
    LoaderXrTermGetInstanceProcAddr(instance, "xrGetVulkanInstanceExtensionsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetVulkanInstanceExtensionsKHR));
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
    LoaderXrTermGetInstanceProcAddr(instance, "xrGetVulkanDeviceExtensionsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetVulkanDeviceExtensionsKHR));
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
    LoaderXrTermGetInstanceProcAddr(instance, "xrGetVulkanGraphicsDeviceKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetVulkanGraphicsDeviceKHR));
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
    LoaderXrTermGetInstanceProcAddr(instance, "xrGetVulkanGraphicsRequirementsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetVulkanGraphicsRequirementsKHR));
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)

    // ---- XR_KHR_D3D11_enable extension commands
#if defined(XR_USE_GRAPHICS_API_D3D11)
    LoaderXrTermGetInstanceProcAddr(instance, "xrGetD3D11GraphicsRequirementsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetD3D11GraphicsRequirementsKHR));
#endif // defined(XR_USE_GRAPHICS_API_D3D11)

    // ---- XR_KHR_D3D12_enable extension commands
#if defined(XR_USE_GRAPHICS_API_D3D12)
    LoaderXrTermGetInstanceProcAddr(instance, "xrGetD3D12GraphicsRequirementsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetD3D12GraphicsRequirementsKHR));
#endif // defined(XR_USE_GRAPHICS_API_D3D12)

    // ---- XR_KHR_visibility_mask extension commands
    LoaderXrTermGetInstanceProcAddr(instance, "xrGetVisibilityMaskKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetVisibilityMaskKHR));

    // ---- XR_KHR_win32_convert_performance_counter_time extension commands
#if defined(XR_USE_PLATFORM_WIN32)
    LoaderXrTermGetInstanceProcAddr(instance, "xrConvertWin32PerformanceCounterToTimeKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->ConvertWin32PerformanceCounterToTimeKHR));
#endif // defined(XR_USE_PLATFORM_WIN32)
#if defined(XR_USE_PLATFORM_WIN32)
    LoaderXrTermGetInstanceProcAddr(instance, "xrConvertTimeToWin32PerformanceCounterKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->ConvertTimeToWin32PerformanceCounterKHR));
#endif // defined(XR_USE_PLATFORM_WIN32)

    // ---- XR_KHR_convert_timespec_time extension commands
#if defined(XR_USE_TIMESPEC)
    LoaderXrTermGetInstanceProcAddr(instance, "xrConvertTimespecTimeToTimeKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->ConvertTimespecTimeToTimeKHR));
#endif // defined(XR_USE_TIMESPEC)
#if defined(XR_USE_TIMESPEC)
    LoaderXrTermGetInstanceProcAddr(instance, "xrConvertTimeToTimespecTimeKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->ConvertTimeToTimespecTimeKHR));
#endif // defined(XR_USE_TIMESPEC)

    // ---- XR_EXT_performance_settings extension commands
    LoaderXrTermGetInstanceProcAddr(instance, "xrPerfSettingsSetPerformanceLevelEXT", reinterpret_cast<PFN_xrVoidFunction*>(&table->PerfSettingsSetPerformanceLevelEXT));

    // ---- XR_EXT_thermal_query extension commands
    LoaderXrTermGetInstanceProcAddr(instance, "xrThermalGetTemperatureTrendEXT", reinterpret_cast<PFN_xrVoidFunction*>(&table->ThermalGetTemperatureTrendEXT));

    // ---- XR_EXT_debug_utils extension commands
    LoaderXrTermGetInstanceProcAddr(instance, "xrSetDebugUtilsObjectNameEXT", reinterpret_cast<PFN_xrVoidFunction*>(&table->SetDebugUtilsObjectNameEXT));
    LoaderXrTermGetInstanceProcAddr(instance, "xrCreateDebugUtilsMessengerEXT", reinterpret_cast<PFN_xrVoidFunction*>(&table->CreateDebugUtilsMessengerEXT));
    LoaderXrTermGetInstanceProcAddr(instance, "xrDestroyDebugUtilsMessengerEXT", reinterpret_cast<PFN_xrVoidFunction*>(&table->DestroyDebugUtilsMessengerEXT));
    LoaderXrTermGetInstanceProcAddr(instance, "xrSubmitDebugUtilsMessageEXT", reinterpret_cast<PFN_xrVoidFunction*>(&table->SubmitDebugUtilsMessageEXT));
    LoaderXrTermGetInstanceProcAddr(instance, "xrSessionBeginDebugUtilsLabelRegionEXT", reinterpret_cast<PFN_xrVoidFunction*>(&table->SessionBeginDebugUtilsLabelRegionEXT));
    LoaderXrTermGetInstanceProcAddr(instance, "xrSessionEndDebugUtilsLabelRegionEXT", reinterpret_cast<PFN_xrVoidFunction*>(&table->SessionEndDebugUtilsLabelRegionEXT));
    LoaderXrTermGetInstanceProcAddr(instance, "xrSessionInsertDebugUtilsLabelEXT", reinterpret_cast<PFN_xrVoidFunction*>(&table->SessionInsertDebugUtilsLabelEXT));

    // ---- XR_MSFT_spatial_anchor extension commands
    LoaderXrTermGetInstanceProcAddr(instance, "xrCreateSpatialAnchorMSFT", reinterpret_cast<PFN_xrVoidFunction*>(&table->CreateSpatialAnchorMSFT));
    LoaderXrTermGetInstanceProcAddr(instance, "xrCreateSpatialAnchorSpaceMSFT", reinterpret_cast<PFN_xrVoidFunction*>(&table->CreateSpatialAnchorSpaceMSFT));
    LoaderXrTermGetInstanceProcAddr(instance, "xrDestroySpatialAnchorMSFT", reinterpret_cast<PFN_xrVoidFunction*>(&table->DestroySpatialAnchorMSFT));
}

// Instance Update Dispatch Table with an API Layer Interface
void ApiLayerInterface::GenUpdateInstanceDispatchTable(XrInstance instance, std::unique_ptr<XrGeneratedDispatchTable>& table) {
    PFN_xrVoidFunction cur_func_ptr;

    // ---- Core 1_0 commands
    table->GetInstanceProcAddr = _get_instance_proc_addr;
    _get_instance_proc_addr(instance, "xrCreateInstance", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->CreateInstance = reinterpret_cast<PFN_xrCreateInstance>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrDestroyInstance", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->DestroyInstance = reinterpret_cast<PFN_xrDestroyInstance>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrGetInstanceProperties", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetInstanceProperties = reinterpret_cast<PFN_xrGetInstanceProperties>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrPollEvent", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->PollEvent = reinterpret_cast<PFN_xrPollEvent>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrResultToString", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->ResultToString = reinterpret_cast<PFN_xrResultToString>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrStructureTypeToString", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->StructureTypeToString = reinterpret_cast<PFN_xrStructureTypeToString>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrGetSystem", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetSystem = reinterpret_cast<PFN_xrGetSystem>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrGetSystemProperties", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetSystemProperties = reinterpret_cast<PFN_xrGetSystemProperties>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrEnumerateEnvironmentBlendModes", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->EnumerateEnvironmentBlendModes = reinterpret_cast<PFN_xrEnumerateEnvironmentBlendModes>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrCreateSession", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->CreateSession = reinterpret_cast<PFN_xrCreateSession>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrDestroySession", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->DestroySession = reinterpret_cast<PFN_xrDestroySession>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrEnumerateReferenceSpaces", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->EnumerateReferenceSpaces = reinterpret_cast<PFN_xrEnumerateReferenceSpaces>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrCreateReferenceSpace", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->CreateReferenceSpace = reinterpret_cast<PFN_xrCreateReferenceSpace>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrGetReferenceSpaceBoundsRect", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetReferenceSpaceBoundsRect = reinterpret_cast<PFN_xrGetReferenceSpaceBoundsRect>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrCreateActionSpace", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->CreateActionSpace = reinterpret_cast<PFN_xrCreateActionSpace>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrLocateSpace", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->LocateSpace = reinterpret_cast<PFN_xrLocateSpace>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrDestroySpace", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->DestroySpace = reinterpret_cast<PFN_xrDestroySpace>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrEnumerateViewConfigurations", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->EnumerateViewConfigurations = reinterpret_cast<PFN_xrEnumerateViewConfigurations>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrGetViewConfigurationProperties", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetViewConfigurationProperties = reinterpret_cast<PFN_xrGetViewConfigurationProperties>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrEnumerateViewConfigurationViews", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->EnumerateViewConfigurationViews = reinterpret_cast<PFN_xrEnumerateViewConfigurationViews>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrEnumerateSwapchainFormats", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->EnumerateSwapchainFormats = reinterpret_cast<PFN_xrEnumerateSwapchainFormats>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrCreateSwapchain", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->CreateSwapchain = reinterpret_cast<PFN_xrCreateSwapchain>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrDestroySwapchain", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->DestroySwapchain = reinterpret_cast<PFN_xrDestroySwapchain>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrEnumerateSwapchainImages", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->EnumerateSwapchainImages = reinterpret_cast<PFN_xrEnumerateSwapchainImages>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrAcquireSwapchainImage", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->AcquireSwapchainImage = reinterpret_cast<PFN_xrAcquireSwapchainImage>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrWaitSwapchainImage", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->WaitSwapchainImage = reinterpret_cast<PFN_xrWaitSwapchainImage>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrReleaseSwapchainImage", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->ReleaseSwapchainImage = reinterpret_cast<PFN_xrReleaseSwapchainImage>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrBeginSession", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->BeginSession = reinterpret_cast<PFN_xrBeginSession>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrEndSession", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->EndSession = reinterpret_cast<PFN_xrEndSession>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrRequestExitSession", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->RequestExitSession = reinterpret_cast<PFN_xrRequestExitSession>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrWaitFrame", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->WaitFrame = reinterpret_cast<PFN_xrWaitFrame>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrBeginFrame", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->BeginFrame = reinterpret_cast<PFN_xrBeginFrame>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrEndFrame", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->EndFrame = reinterpret_cast<PFN_xrEndFrame>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrLocateViews", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->LocateViews = reinterpret_cast<PFN_xrLocateViews>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrStringToPath", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->StringToPath = reinterpret_cast<PFN_xrStringToPath>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrPathToString", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->PathToString = reinterpret_cast<PFN_xrPathToString>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrCreateActionSet", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->CreateActionSet = reinterpret_cast<PFN_xrCreateActionSet>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrDestroyActionSet", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->DestroyActionSet = reinterpret_cast<PFN_xrDestroyActionSet>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrCreateAction", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->CreateAction = reinterpret_cast<PFN_xrCreateAction>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrDestroyAction", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->DestroyAction = reinterpret_cast<PFN_xrDestroyAction>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrSuggestInteractionProfileBindings", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->SuggestInteractionProfileBindings = reinterpret_cast<PFN_xrSuggestInteractionProfileBindings>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrAttachSessionActionSets", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->AttachSessionActionSets = reinterpret_cast<PFN_xrAttachSessionActionSets>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrGetCurrentInteractionProfile", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetCurrentInteractionProfile = reinterpret_cast<PFN_xrGetCurrentInteractionProfile>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrGetActionStateBoolean", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetActionStateBoolean = reinterpret_cast<PFN_xrGetActionStateBoolean>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrGetActionStateFloat", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetActionStateFloat = reinterpret_cast<PFN_xrGetActionStateFloat>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrGetActionStateVector2f", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetActionStateVector2f = reinterpret_cast<PFN_xrGetActionStateVector2f>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrGetActionStatePose", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetActionStatePose = reinterpret_cast<PFN_xrGetActionStatePose>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrSyncActions", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->SyncActions = reinterpret_cast<PFN_xrSyncActions>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrEnumerateBoundSourcesForAction", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->EnumerateBoundSourcesForAction = reinterpret_cast<PFN_xrEnumerateBoundSourcesForAction>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrGetInputSourceLocalizedName", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetInputSourceLocalizedName = reinterpret_cast<PFN_xrGetInputSourceLocalizedName>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrApplyHapticFeedback", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->ApplyHapticFeedback = reinterpret_cast<PFN_xrApplyHapticFeedback>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrStopHapticFeedback", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->StopHapticFeedback = reinterpret_cast<PFN_xrStopHapticFeedback>(cur_func_ptr);
    }

    // ---- XR_KHR_android_thread_settings extension commands
#if defined(XR_USE_PLATFORM_ANDROID)
    _get_instance_proc_addr(instance, "xrSetAndroidApplicationThreadKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->SetAndroidApplicationThreadKHR = reinterpret_cast<PFN_xrSetAndroidApplicationThreadKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_PLATFORM_ANDROID)

    // ---- XR_KHR_android_surface_swapchain extension commands
#if defined(XR_USE_PLATFORM_ANDROID)
    _get_instance_proc_addr(instance, "xrCreateSwapchainAndroidSurfaceKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->CreateSwapchainAndroidSurfaceKHR = reinterpret_cast<PFN_xrCreateSwapchainAndroidSurfaceKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_PLATFORM_ANDROID)

    // ---- XR_KHR_opengl_enable extension commands
#if defined(XR_USE_GRAPHICS_API_OPENGL)
    _get_instance_proc_addr(instance, "xrGetOpenGLGraphicsRequirementsKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetOpenGLGraphicsRequirementsKHR = reinterpret_cast<PFN_xrGetOpenGLGraphicsRequirementsKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_GRAPHICS_API_OPENGL)

    // ---- XR_KHR_opengl_es_enable extension commands
#if defined(XR_USE_GRAPHICS_API_OPENGL_ES)
    _get_instance_proc_addr(instance, "xrGetOpenGLESGraphicsRequirementsKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetOpenGLESGraphicsRequirementsKHR = reinterpret_cast<PFN_xrGetOpenGLESGraphicsRequirementsKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_GRAPHICS_API_OPENGL_ES)

    // ---- XR_KHR_vulkan_enable extension commands
#if defined(XR_USE_GRAPHICS_API_VULKAN)
    _get_instance_proc_addr(instance, "xrGetVulkanInstanceExtensionsKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetVulkanInstanceExtensionsKHR = reinterpret_cast<PFN_xrGetVulkanInstanceExtensionsKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
    _get_instance_proc_addr(instance, "xrGetVulkanDeviceExtensionsKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetVulkanDeviceExtensionsKHR = reinterpret_cast<PFN_xrGetVulkanDeviceExtensionsKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
    _get_instance_proc_addr(instance, "xrGetVulkanGraphicsDeviceKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetVulkanGraphicsDeviceKHR = reinterpret_cast<PFN_xrGetVulkanGraphicsDeviceKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
    _get_instance_proc_addr(instance, "xrGetVulkanGraphicsRequirementsKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetVulkanGraphicsRequirementsKHR = reinterpret_cast<PFN_xrGetVulkanGraphicsRequirementsKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)

    // ---- XR_KHR_D3D11_enable extension commands
#if defined(XR_USE_GRAPHICS_API_D3D11)
    _get_instance_proc_addr(instance, "xrGetD3D11GraphicsRequirementsKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetD3D11GraphicsRequirementsKHR = reinterpret_cast<PFN_xrGetD3D11GraphicsRequirementsKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_GRAPHICS_API_D3D11)

    // ---- XR_KHR_D3D12_enable extension commands
#if defined(XR_USE_GRAPHICS_API_D3D12)
    _get_instance_proc_addr(instance, "xrGetD3D12GraphicsRequirementsKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetD3D12GraphicsRequirementsKHR = reinterpret_cast<PFN_xrGetD3D12GraphicsRequirementsKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_GRAPHICS_API_D3D12)

    // ---- XR_KHR_visibility_mask extension commands
    _get_instance_proc_addr(instance, "xrGetVisibilityMaskKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetVisibilityMaskKHR = reinterpret_cast<PFN_xrGetVisibilityMaskKHR>(cur_func_ptr);
    }

    // ---- XR_KHR_win32_convert_performance_counter_time extension commands
#if defined(XR_USE_PLATFORM_WIN32)
    _get_instance_proc_addr(instance, "xrConvertWin32PerformanceCounterToTimeKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->ConvertWin32PerformanceCounterToTimeKHR = reinterpret_cast<PFN_xrConvertWin32PerformanceCounterToTimeKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_PLATFORM_WIN32)
#if defined(XR_USE_PLATFORM_WIN32)
    _get_instance_proc_addr(instance, "xrConvertTimeToWin32PerformanceCounterKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->ConvertTimeToWin32PerformanceCounterKHR = reinterpret_cast<PFN_xrConvertTimeToWin32PerformanceCounterKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_PLATFORM_WIN32)

    // ---- XR_KHR_convert_timespec_time extension commands
#if defined(XR_USE_TIMESPEC)
    _get_instance_proc_addr(instance, "xrConvertTimespecTimeToTimeKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->ConvertTimespecTimeToTimeKHR = reinterpret_cast<PFN_xrConvertTimespecTimeToTimeKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_TIMESPEC)
#if defined(XR_USE_TIMESPEC)
    _get_instance_proc_addr(instance, "xrConvertTimeToTimespecTimeKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->ConvertTimeToTimespecTimeKHR = reinterpret_cast<PFN_xrConvertTimeToTimespecTimeKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_TIMESPEC)

    // ---- XR_EXT_performance_settings extension commands
    _get_instance_proc_addr(instance, "xrPerfSettingsSetPerformanceLevelEXT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->PerfSettingsSetPerformanceLevelEXT = reinterpret_cast<PFN_xrPerfSettingsSetPerformanceLevelEXT>(cur_func_ptr);
    }

    // ---- XR_EXT_thermal_query extension commands
    _get_instance_proc_addr(instance, "xrThermalGetTemperatureTrendEXT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->ThermalGetTemperatureTrendEXT = reinterpret_cast<PFN_xrThermalGetTemperatureTrendEXT>(cur_func_ptr);
    }

    // ---- XR_EXT_debug_utils extension commands
    _get_instance_proc_addr(instance, "xrSetDebugUtilsObjectNameEXT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->SetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_xrSetDebugUtilsObjectNameEXT>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrCreateDebugUtilsMessengerEXT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->CreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_xrCreateDebugUtilsMessengerEXT>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrDestroyDebugUtilsMessengerEXT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->DestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_xrDestroyDebugUtilsMessengerEXT>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrSubmitDebugUtilsMessageEXT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->SubmitDebugUtilsMessageEXT = reinterpret_cast<PFN_xrSubmitDebugUtilsMessageEXT>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrSessionBeginDebugUtilsLabelRegionEXT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->SessionBeginDebugUtilsLabelRegionEXT = reinterpret_cast<PFN_xrSessionBeginDebugUtilsLabelRegionEXT>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrSessionEndDebugUtilsLabelRegionEXT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->SessionEndDebugUtilsLabelRegionEXT = reinterpret_cast<PFN_xrSessionEndDebugUtilsLabelRegionEXT>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrSessionInsertDebugUtilsLabelEXT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->SessionInsertDebugUtilsLabelEXT = reinterpret_cast<PFN_xrSessionInsertDebugUtilsLabelEXT>(cur_func_ptr);
    }

    // ---- XR_MSFT_spatial_anchor extension commands
    _get_instance_proc_addr(instance, "xrCreateSpatialAnchorMSFT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->CreateSpatialAnchorMSFT = reinterpret_cast<PFN_xrCreateSpatialAnchorMSFT>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrCreateSpatialAnchorSpaceMSFT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->CreateSpatialAnchorSpaceMSFT = reinterpret_cast<PFN_xrCreateSpatialAnchorSpaceMSFT>(cur_func_ptr);
    }
    _get_instance_proc_addr(instance, "xrDestroySpatialAnchorMSFT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->DestroySpatialAnchorMSFT = reinterpret_cast<PFN_xrDestroySpatialAnchorMSFT>(cur_func_ptr);
    }
}

