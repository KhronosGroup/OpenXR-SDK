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

#ifdef _WIN32
// Disable Windows warning about using strncpy_s instead of strncpy
#define  _CRT_SECURE_NO_WARNINGS 1
#endif // _WIN32

#include "xr_generated_utilities.h"

#include <openxr/openxr.h>

#include <stdio.h>
#include <string.h>


#ifdef __cplusplus
extern "C" { 
#endif

XrResult GeneratedXrUtilitiesResultToString(XrResult result,
                                            char buffer[XR_MAX_RESULT_STRING_SIZE]) {
    if (NULL == buffer) {
        return XR_ERROR_VALIDATION_FAILURE;
    }
    XrResult int_result = XR_SUCCESS;
    switch (result) {
        case XR_SUCCESS:
            strncpy(buffer, "XR_SUCCESS", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_TIMEOUT_EXPIRED:
            strncpy(buffer, "XR_TIMEOUT_EXPIRED", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_SESSION_LOSS_PENDING:
            strncpy(buffer, "XR_SESSION_LOSS_PENDING", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_EVENT_UNAVAILABLE:
            strncpy(buffer, "XR_EVENT_UNAVAILABLE", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_SPACE_BOUNDS_UNAVAILABLE:
            strncpy(buffer, "XR_SPACE_BOUNDS_UNAVAILABLE", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_SESSION_NOT_FOCUSED:
            strncpy(buffer, "XR_SESSION_NOT_FOCUSED", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_FRAME_DISCARDED:
            strncpy(buffer, "XR_FRAME_DISCARDED", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_VALIDATION_FAILURE:
            strncpy(buffer, "XR_ERROR_VALIDATION_FAILURE", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_RUNTIME_FAILURE:
            strncpy(buffer, "XR_ERROR_RUNTIME_FAILURE", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_OUT_OF_MEMORY:
            strncpy(buffer, "XR_ERROR_OUT_OF_MEMORY", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_API_VERSION_UNSUPPORTED:
            strncpy(buffer, "XR_ERROR_API_VERSION_UNSUPPORTED", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_INITIALIZATION_FAILED:
            strncpy(buffer, "XR_ERROR_INITIALIZATION_FAILED", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_FUNCTION_UNSUPPORTED:
            strncpy(buffer, "XR_ERROR_FUNCTION_UNSUPPORTED", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_FEATURE_UNSUPPORTED:
            strncpy(buffer, "XR_ERROR_FEATURE_UNSUPPORTED", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_EXTENSION_NOT_PRESENT:
            strncpy(buffer, "XR_ERROR_EXTENSION_NOT_PRESENT", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_LIMIT_REACHED:
            strncpy(buffer, "XR_ERROR_LIMIT_REACHED", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_SIZE_INSUFFICIENT:
            strncpy(buffer, "XR_ERROR_SIZE_INSUFFICIENT", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_HANDLE_INVALID:
            strncpy(buffer, "XR_ERROR_HANDLE_INVALID", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_INSTANCE_LOST:
            strncpy(buffer, "XR_ERROR_INSTANCE_LOST", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_SESSION_RUNNING:
            strncpy(buffer, "XR_ERROR_SESSION_RUNNING", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_SESSION_NOT_RUNNING:
            strncpy(buffer, "XR_ERROR_SESSION_NOT_RUNNING", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_SESSION_LOST:
            strncpy(buffer, "XR_ERROR_SESSION_LOST", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_SYSTEM_INVALID:
            strncpy(buffer, "XR_ERROR_SYSTEM_INVALID", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_PATH_INVALID:
            strncpy(buffer, "XR_ERROR_PATH_INVALID", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_PATH_COUNT_EXCEEDED:
            strncpy(buffer, "XR_ERROR_PATH_COUNT_EXCEEDED", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_PATH_FORMAT_INVALID:
            strncpy(buffer, "XR_ERROR_PATH_FORMAT_INVALID", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_PATH_UNSUPPORTED:
            strncpy(buffer, "XR_ERROR_PATH_UNSUPPORTED", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_LAYER_INVALID:
            strncpy(buffer, "XR_ERROR_LAYER_INVALID", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_LAYER_LIMIT_EXCEEDED:
            strncpy(buffer, "XR_ERROR_LAYER_LIMIT_EXCEEDED", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_SWAPCHAIN_RECT_INVALID:
            strncpy(buffer, "XR_ERROR_SWAPCHAIN_RECT_INVALID", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_SWAPCHAIN_FORMAT_UNSUPPORTED:
            strncpy(buffer, "XR_ERROR_SWAPCHAIN_FORMAT_UNSUPPORTED", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_ACTION_TYPE_MISMATCH:
            strncpy(buffer, "XR_ERROR_ACTION_TYPE_MISMATCH", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_SESSION_NOT_READY:
            strncpy(buffer, "XR_ERROR_SESSION_NOT_READY", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_SESSION_NOT_STOPPING:
            strncpy(buffer, "XR_ERROR_SESSION_NOT_STOPPING", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_TIME_INVALID:
            strncpy(buffer, "XR_ERROR_TIME_INVALID", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_REFERENCE_SPACE_UNSUPPORTED:
            strncpy(buffer, "XR_ERROR_REFERENCE_SPACE_UNSUPPORTED", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_FILE_ACCESS_ERROR:
            strncpy(buffer, "XR_ERROR_FILE_ACCESS_ERROR", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_FILE_CONTENTS_INVALID:
            strncpy(buffer, "XR_ERROR_FILE_CONTENTS_INVALID", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_FORM_FACTOR_UNSUPPORTED:
            strncpy(buffer, "XR_ERROR_FORM_FACTOR_UNSUPPORTED", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_FORM_FACTOR_UNAVAILABLE:
            strncpy(buffer, "XR_ERROR_FORM_FACTOR_UNAVAILABLE", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_API_LAYER_NOT_PRESENT:
            strncpy(buffer, "XR_ERROR_API_LAYER_NOT_PRESENT", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_CALL_ORDER_INVALID:
            strncpy(buffer, "XR_ERROR_CALL_ORDER_INVALID", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_GRAPHICS_DEVICE_INVALID:
            strncpy(buffer, "XR_ERROR_GRAPHICS_DEVICE_INVALID", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_POSE_INVALID:
            strncpy(buffer, "XR_ERROR_POSE_INVALID", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_INDEX_OUT_OF_RANGE:
            strncpy(buffer, "XR_ERROR_INDEX_OUT_OF_RANGE", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_VIEW_CONFIGURATION_TYPE_UNSUPPORTED:
            strncpy(buffer, "XR_ERROR_VIEW_CONFIGURATION_TYPE_UNSUPPORTED", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_ENVIRONMENT_BLEND_MODE_UNSUPPORTED:
            strncpy(buffer, "XR_ERROR_ENVIRONMENT_BLEND_MODE_UNSUPPORTED", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_NAME_DUPLICATED:
            strncpy(buffer, "XR_ERROR_NAME_DUPLICATED", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_NAME_INVALID:
            strncpy(buffer, "XR_ERROR_NAME_INVALID", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_ACTIONSET_NOT_ATTACHED:
            strncpy(buffer, "XR_ERROR_ACTIONSET_NOT_ATTACHED", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_ACTIONSETS_ALREADY_ATTACHED:
            strncpy(buffer, "XR_ERROR_ACTIONSETS_ALREADY_ATTACHED", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_LOCALIZED_NAME_DUPLICATED:
            strncpy(buffer, "XR_ERROR_LOCALIZED_NAME_DUPLICATED", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_LOCALIZED_NAME_INVALID:
            strncpy(buffer, "XR_ERROR_LOCALIZED_NAME_INVALID", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_ANDROID_THREAD_SETTINGS_ID_INVALID_KHR:
            strncpy(buffer, "XR_ERROR_ANDROID_THREAD_SETTINGS_ID_INVALID_KHR", XR_MAX_RESULT_STRING_SIZE);
            break;
        case XR_ERROR_ANDROID_THREAD_SETTINGS_FAILURE_KHR:
            strncpy(buffer, "XR_ERROR_ANDROID_THREAD_SETTINGS_FAILURE_KHR", XR_MAX_RESULT_STRING_SIZE);
            break;
        default:
            // Unknown result type
            if (XR_SUCCEEDED(result)) {
                snprintf(buffer, XR_MAX_RESULT_STRING_SIZE, "XR_UNKNOWN_SUCCESS_%d", result);
            } else {
                snprintf(buffer, XR_MAX_RESULT_STRING_SIZE, "XR_UNKNOWN_FAILURE_%d", result);
            }
            int_result = XR_ERROR_VALIDATION_FAILURE;
            break;
    }
    return int_result;
}

XrResult GeneratedXrUtilitiesStructureTypeToString(XrStructureType struct_type,
                                            char buffer[XR_MAX_STRUCTURE_NAME_SIZE]) {
    if (NULL == buffer) {
        return XR_ERROR_VALIDATION_FAILURE;
    }
    XrResult int_result = XR_SUCCESS;
    switch (struct_type) {
        case XR_TYPE_UNKNOWN:
            strncpy(buffer, "XR_TYPE_UNKNOWN", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_API_LAYER_PROPERTIES:
            strncpy(buffer, "XR_TYPE_API_LAYER_PROPERTIES", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_EXTENSION_PROPERTIES:
            strncpy(buffer, "XR_TYPE_EXTENSION_PROPERTIES", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_INSTANCE_CREATE_INFO:
            strncpy(buffer, "XR_TYPE_INSTANCE_CREATE_INFO", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_SYSTEM_GET_INFO:
            strncpy(buffer, "XR_TYPE_SYSTEM_GET_INFO", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_SYSTEM_PROPERTIES:
            strncpy(buffer, "XR_TYPE_SYSTEM_PROPERTIES", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_VIEW_LOCATE_INFO:
            strncpy(buffer, "XR_TYPE_VIEW_LOCATE_INFO", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_VIEW:
            strncpy(buffer, "XR_TYPE_VIEW", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_SESSION_CREATE_INFO:
            strncpy(buffer, "XR_TYPE_SESSION_CREATE_INFO", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_SWAPCHAIN_CREATE_INFO:
            strncpy(buffer, "XR_TYPE_SWAPCHAIN_CREATE_INFO", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_SESSION_BEGIN_INFO:
            strncpy(buffer, "XR_TYPE_SESSION_BEGIN_INFO", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_VIEW_STATE:
            strncpy(buffer, "XR_TYPE_VIEW_STATE", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_FRAME_END_INFO:
            strncpy(buffer, "XR_TYPE_FRAME_END_INFO", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_HAPTIC_VIBRATION:
            strncpy(buffer, "XR_TYPE_HAPTIC_VIBRATION", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_EVENT_DATA_BUFFER:
            strncpy(buffer, "XR_TYPE_EVENT_DATA_BUFFER", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
            strncpy(buffer, "XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
            strncpy(buffer, "XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_ACTION_STATE_BOOLEAN:
            strncpy(buffer, "XR_TYPE_ACTION_STATE_BOOLEAN", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_ACTION_STATE_FLOAT:
            strncpy(buffer, "XR_TYPE_ACTION_STATE_FLOAT", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_ACTION_STATE_VECTOR2F:
            strncpy(buffer, "XR_TYPE_ACTION_STATE_VECTOR2F", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_ACTION_STATE_POSE:
            strncpy(buffer, "XR_TYPE_ACTION_STATE_POSE", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_ACTION_SET_CREATE_INFO:
            strncpy(buffer, "XR_TYPE_ACTION_SET_CREATE_INFO", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_ACTION_CREATE_INFO:
            strncpy(buffer, "XR_TYPE_ACTION_CREATE_INFO", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_INSTANCE_PROPERTIES:
            strncpy(buffer, "XR_TYPE_INSTANCE_PROPERTIES", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_FRAME_WAIT_INFO:
            strncpy(buffer, "XR_TYPE_FRAME_WAIT_INFO", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_COMPOSITION_LAYER_PROJECTION:
            strncpy(buffer, "XR_TYPE_COMPOSITION_LAYER_PROJECTION", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_COMPOSITION_LAYER_QUAD:
            strncpy(buffer, "XR_TYPE_COMPOSITION_LAYER_QUAD", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_REFERENCE_SPACE_CREATE_INFO:
            strncpy(buffer, "XR_TYPE_REFERENCE_SPACE_CREATE_INFO", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_ACTION_SPACE_CREATE_INFO:
            strncpy(buffer, "XR_TYPE_ACTION_SPACE_CREATE_INFO", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING:
            strncpy(buffer, "XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_VIEW_CONFIGURATION_VIEW:
            strncpy(buffer, "XR_TYPE_VIEW_CONFIGURATION_VIEW", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_SPACE_LOCATION:
            strncpy(buffer, "XR_TYPE_SPACE_LOCATION", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_SPACE_VELOCITY:
            strncpy(buffer, "XR_TYPE_SPACE_VELOCITY", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_FRAME_STATE:
            strncpy(buffer, "XR_TYPE_FRAME_STATE", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_VIEW_CONFIGURATION_PROPERTIES:
            strncpy(buffer, "XR_TYPE_VIEW_CONFIGURATION_PROPERTIES", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_FRAME_BEGIN_INFO:
            strncpy(buffer, "XR_TYPE_FRAME_BEGIN_INFO", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW:
            strncpy(buffer, "XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_EVENT_DATA_EVENTS_LOST:
            strncpy(buffer, "XR_TYPE_EVENT_DATA_EVENTS_LOST", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING:
            strncpy(buffer, "XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
            strncpy(buffer, "XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_INTERACTION_PROFILE_STATE:
            strncpy(buffer, "XR_TYPE_INTERACTION_PROFILE_STATE", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO:
            strncpy(buffer, "XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO:
            strncpy(buffer, "XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO:
            strncpy(buffer, "XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_ACTION_STATE_GET_INFO:
            strncpy(buffer, "XR_TYPE_ACTION_STATE_GET_INFO", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_HAPTIC_ACTION_INFO:
            strncpy(buffer, "XR_TYPE_HAPTIC_ACTION_INFO", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO:
            strncpy(buffer, "XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_ACTIONS_SYNC_INFO:
            strncpy(buffer, "XR_TYPE_ACTIONS_SYNC_INFO", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_BOUND_SOURCES_FOR_ACTION_ENUMERATE_INFO:
            strncpy(buffer, "XR_TYPE_BOUND_SOURCES_FOR_ACTION_ENUMERATE_INFO", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_INPUT_SOURCE_LOCALIZED_NAME_GET_INFO:
            strncpy(buffer, "XR_TYPE_INPUT_SOURCE_LOCALIZED_NAME_GET_INFO", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_COMPOSITION_LAYER_CUBE_KHR:
            strncpy(buffer, "XR_TYPE_COMPOSITION_LAYER_CUBE_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_INSTANCE_CREATE_INFO_ANDROID_KHR:
            strncpy(buffer, "XR_TYPE_INSTANCE_CREATE_INFO_ANDROID_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR:
            strncpy(buffer, "XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_VULKAN_SWAPCHAIN_FORMAT_LIST_CREATE_INFO_KHR:
            strncpy(buffer, "XR_TYPE_VULKAN_SWAPCHAIN_FORMAT_LIST_CREATE_INFO_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_EVENT_DATA_PERF_SETTINGS_EXT:
            strncpy(buffer, "XR_TYPE_EVENT_DATA_PERF_SETTINGS_EXT", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_COMPOSITION_LAYER_CYLINDER_KHR:
            strncpy(buffer, "XR_TYPE_COMPOSITION_LAYER_CYLINDER_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_COMPOSITION_LAYER_EQUIRECT_KHR:
            strncpy(buffer, "XR_TYPE_COMPOSITION_LAYER_EQUIRECT_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT:
            strncpy(buffer, "XR_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT:
            strncpy(buffer, "XR_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT:
            strncpy(buffer, "XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_DEBUG_UTILS_LABEL_EXT:
            strncpy(buffer, "XR_TYPE_DEBUG_UTILS_LABEL_EXT", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR:
            strncpy(buffer, "XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR:
            strncpy(buffer, "XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_GRAPHICS_BINDING_OPENGL_XCB_KHR:
            strncpy(buffer, "XR_TYPE_GRAPHICS_BINDING_OPENGL_XCB_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_GRAPHICS_BINDING_OPENGL_WAYLAND_KHR:
            strncpy(buffer, "XR_TYPE_GRAPHICS_BINDING_OPENGL_WAYLAND_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR:
            strncpy(buffer, "XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR:
            strncpy(buffer, "XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_GRAPHICS_BINDING_OPENGL_ES_ANDROID_KHR:
            strncpy(buffer, "XR_TYPE_GRAPHICS_BINDING_OPENGL_ES_ANDROID_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_ES_KHR:
            strncpy(buffer, "XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_ES_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_ES_KHR:
            strncpy(buffer, "XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_ES_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR:
            strncpy(buffer, "XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR:
            strncpy(buffer, "XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR:
            strncpy(buffer, "XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_GRAPHICS_BINDING_D3D11_KHR:
            strncpy(buffer, "XR_TYPE_GRAPHICS_BINDING_D3D11_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR:
            strncpy(buffer, "XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR:
            strncpy(buffer, "XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_GRAPHICS_BINDING_D3D12_KHR:
            strncpy(buffer, "XR_TYPE_GRAPHICS_BINDING_D3D12_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_SWAPCHAIN_IMAGE_D3D12_KHR:
            strncpy(buffer, "XR_TYPE_SWAPCHAIN_IMAGE_D3D12_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_GRAPHICS_REQUIREMENTS_D3D12_KHR:
            strncpy(buffer, "XR_TYPE_GRAPHICS_REQUIREMENTS_D3D12_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_VISIBILITY_MASK_KHR:
            strncpy(buffer, "XR_TYPE_VISIBILITY_MASK_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        case XR_TYPE_EVENT_DATA_VISIBILITY_MASK_CHANGED_KHR:
            strncpy(buffer, "XR_TYPE_EVENT_DATA_VISIBILITY_MASK_CHANGED_KHR", XR_MAX_STRUCTURE_NAME_SIZE);
            break;
        default:
            // Unknown structure type
            snprintf(buffer, XR_MAX_STRUCTURE_NAME_SIZE, "XR_UNKNOWN_STRUCTURE_TYPE_%d", struct_type);
            int_result = XR_ERROR_VALIDATION_FAILURE;
            break;
    }
    return int_result;
}

#ifdef __cplusplus
} // extern "C"
#endif

