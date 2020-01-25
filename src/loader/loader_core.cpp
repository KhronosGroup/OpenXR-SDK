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
// Author: Dave Houlton <daveh@lunarg.com>
//

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif  // defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)

#include "api_layer_interface.hpp"
#include "exception_handling.hpp"
#include "hex_and_handles.h"
#include "loader_instance.hpp"
#include "loader_logger_recorders.hpp"
#include "loader_logger.hpp"
#include "loader_platform.hpp"
#include "runtime_interface.hpp"
#include "xr_generated_dispatch_table.h"
#include "xr_generated_loader.hpp"

#include <openxr/openxr.h>

#include <cstring>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
// Global lock to prevent reading JSON manifest files at the same time.
static std::mutex g_loader_json_mutex;

// Global lock to prevent simultaneous instance creation/destruction
static std::mutex g_instance_create_destroy_mutex;

// Terminal functions needed by xrCreateInstance.
XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermGetInstanceProcAddr(XrInstance, const char *, PFN_xrVoidFunction *);
XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermCreateInstance(const XrInstanceCreateInfo *, XrInstance *);
XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermCreateApiLayerInstance(const XrInstanceCreateInfo *, const struct XrApiLayerCreateInfo *,
                                                                  XrInstance *);
XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermSetDebugUtilsObjectNameEXT(XrInstance, const XrDebugUtilsObjectNameInfoEXT *);
XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermCreateDebugUtilsMessengerEXT(XrInstance, const XrDebugUtilsMessengerCreateInfoEXT *,
                                                                        XrDebugUtilsMessengerEXT *);
XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermDestroyDebugUtilsMessengerEXT(XrDebugUtilsMessengerEXT);
XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermSubmitDebugUtilsMessageEXT(XrInstance instance,
                                                                      XrDebugUtilsMessageSeverityFlagsEXT messageSeverity,
                                                                      XrDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                                      const XrDebugUtilsMessengerCallbackDataEXT *callbackData);

// Utility template function meant to validate if a fixed size string contains
// a null-terminator.
template <size_t max_length>
inline bool IsMissingNullTerminator(const char (&str)[max_length]) {
    for (size_t index = 0; index < max_length; ++index) {
        if (str[index] == '\0') {
            return false;
        }
    }
    return true;
}

// ---- Core 1.0 manual loader trampoline functions

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateApiLayerProperties(uint32_t propertyCapacityInput,
                                                                           uint32_t *propertyCountOutput,
                                                                           XrApiLayerProperties *properties) XRLOADER_ABI_TRY {
    LoaderLogger::LogVerboseMessage("xrEnumerateApiLayerProperties", "Entering loader trampoline");

    // Make sure only one thread is attempting to read the JSON files at a time.
    std::unique_lock<std::mutex> json_lock(g_loader_json_mutex);

    XrResult result = ApiLayerInterface::GetApiLayerProperties("xrEnumerateApiLayerProperties", propertyCapacityInput,
                                                               propertyCountOutput, properties);
    if (XR_FAILED(result)) {
        LoaderLogger::LogErrorMessage("xrEnumerateApiLayerProperties", "Failed ApiLayerInterface::GetApiLayerProperties");
    }

    return result;
}
XRLOADER_ABI_CATCH_FALLBACK

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL
xrEnumerateInstanceExtensionProperties(const char *layerName, uint32_t propertyCapacityInput, uint32_t *propertyCountOutput,
                                       XrExtensionProperties *properties) XRLOADER_ABI_TRY {
    bool just_layer_properties = false;
    LoaderLogger::LogVerboseMessage("xrEnumerateInstanceExtensionProperties", "Entering loader trampoline");

    // "Independent of elementCapacityInput or elements parameters, elementCountOutput must be a valid pointer,
    // and the function sets elementCountOutput." - 2.11
    if (nullptr == propertyCountOutput) {
        return XR_ERROR_VALIDATION_FAILURE;
    }

    if (nullptr != layerName && 0 != strlen(layerName)) {
        // Application is only interested in layer's properties, not all of them.
        just_layer_properties = true;
    }

    std::vector<XrExtensionProperties> extension_properties = {};
    XrResult result;

    {
        // Make sure only one thread is attempting to read the JSON files at a time.
        std::unique_lock<std::mutex> json_lock(g_loader_json_mutex);

        // Get the layer extension properties
        result = ApiLayerInterface::GetInstanceExtensionProperties("xrEnumerateInstanceExtensionProperties", layerName,
                                                                   extension_properties);
        if (XR_SUCCEEDED(result) && !just_layer_properties) {
            // If not specific to a layer, get the runtime extension properties
            result = RuntimeInterface::LoadRuntime("xrEnumerateInstanceExtensionProperties");
            if (XR_SUCCEEDED(result)) {
                RuntimeInterface::GetRuntime().GetInstanceExtensionProperties(extension_properties);
                RuntimeInterface::UnloadRuntime("xrEnumerateInstanceExtensionProperties");
            } else {
                LoaderLogger::LogErrorMessage("xrEnumerateInstanceExtensionProperties",
                                              "Failed to find default runtime with RuntimeInterface::LoadRuntime()");
            }
        }
    }

    if (XR_FAILED(result)) {
        LoaderLogger::LogErrorMessage("xrEnumerateInstanceExtensionProperties", "Failed querying extension properties");
        return result;
    }

    // If this is not in reference to a specific layer, then add the loader-specific extension properties as well.
    // These are extensions that the loader directly supports.
    if (!just_layer_properties) {
        for (const XrExtensionProperties &loader_prop : LoaderInstance::LoaderSpecificExtensions()) {
            bool found_prop = false;
            for (XrExtensionProperties &existing_prop : extension_properties) {
                if (0 == strcmp(existing_prop.extensionName, loader_prop.extensionName)) {
                    found_prop = true;
                    // Use the loader version if it is newer
                    if (existing_prop.extensionVersion < loader_prop.extensionVersion) {
                        existing_prop.extensionVersion = loader_prop.extensionVersion;
                    }
                    break;
                }
            }
            // Only add extensions not supported by the loader
            if (!found_prop) {
                extension_properties.push_back(loader_prop);
            }
        }
    }

    auto num_extension_properties = static_cast<uint32_t>(extension_properties.size());
    if (propertyCapacityInput == 0) {
        *propertyCountOutput = num_extension_properties;
    } else if (nullptr != properties) {
        if (propertyCapacityInput < num_extension_properties) {
            *propertyCountOutput = num_extension_properties;
            LoaderLogger::LogValidationErrorMessage("VUID-xrEnumerateInstanceExtensionProperties-propertyCountOutput-parameter",
                                                    "xrEnumerateInstanceExtensionProperties", "insufficient space in array");
            return XR_ERROR_SIZE_INSUFFICIENT;
        }

        uint32_t num_to_copy = num_extension_properties;
        // Determine how many extension properties we can copy over
        if (propertyCapacityInput < num_to_copy) {
            num_to_copy = propertyCapacityInput;
        }
        bool properties_valid = true;
        for (uint32_t prop = 0; prop < propertyCapacityInput && prop < extension_properties.size(); ++prop) {
            if (XR_TYPE_EXTENSION_PROPERTIES != properties[prop].type) {
                properties_valid = false;
                LoaderLogger::LogValidationErrorMessage("VUID-XrExtensionProperties-type-type",
                                                        "xrEnumerateInstanceExtensionProperties", "unknown type in properties");
            }
            if (properties_valid) {
                properties[prop] = extension_properties[prop];
            }
        }
        if (!properties_valid) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrEnumerateInstanceExtensionProperties-properties-parameter",
                                                    "xrEnumerateInstanceExtensionProperties", "invalid properties");
            return XR_ERROR_VALIDATION_FAILURE;
        }
        if (nullptr != propertyCountOutput) {
            *propertyCountOutput = num_to_copy;
        }
    } else {
        // incoming_count is not 0 BUT the properties is NULL
        return XR_ERROR_VALIDATION_FAILURE;
    }
    LoaderLogger::LogVerboseMessage("xrEnumerateInstanceExtensionProperties", "Completed loader trampoline");
    return XR_SUCCESS;
}
XRLOADER_ABI_CATCH_FALLBACK

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrCreateInstance(const XrInstanceCreateInfo *info,
                                                              XrInstance *instance) XRLOADER_ABI_TRY {
    bool runtime_loaded = false;

    LoaderLogger::LogVerboseMessage("xrCreateInstance", "Entering loader trampoline");
    if (nullptr == info) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrCreateInstance-info-parameter", "xrCreateInstance", "must be non-NULL");
        return XR_ERROR_VALIDATION_FAILURE;
    }
    // If application requested OpenXR API version is higher than the loader version, then we need to throw
    // an error.
    uint16_t app_major = XR_VERSION_MAJOR(info->applicationInfo.apiVersion);  // NOLINT
    uint16_t app_minor = XR_VERSION_MINOR(info->applicationInfo.apiVersion);  // NOLINT
    uint16_t loader_major = XR_VERSION_MAJOR(XR_CURRENT_API_VERSION);         // NOLINT
    uint16_t loader_minor = XR_VERSION_MINOR(XR_CURRENT_API_VERSION);         // NOLINT
    if (app_major > loader_major || (app_major == loader_major && app_minor > loader_minor)) {
        std::ostringstream oss;
        oss << "xrCreateInstance called with invalid API version " << app_major << "." << app_minor
            << ".  Max supported version is " << loader_major << "." << loader_minor;
        LoaderLogger::LogErrorMessage("xrCreateInstance", oss.str());
        return XR_ERROR_API_VERSION_UNSUPPORTED;
    }

    if (nullptr == instance) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrCreateInstance-instance-parameter", "xrCreateInstance", "must be non-NULL");
        return XR_ERROR_VALIDATION_FAILURE;
    }

    std::unique_lock<std::mutex> instance_lock(g_instance_create_destroy_mutex);

    // Check if there is already an XrInstance that is alive. If so, another instance cannot be created.
    // The loader does not support multiple simultaneous instances because the loader is intended to be
    // usable by apps using future OpenXR APIs (through xrGetInstanceProcAddr). Because the loader would
    // not be aware of new handle types, it would not be able to look up the appropriate dispatch table
    // in some cases.
    if (ActiveLoaderInstance::IsAvailable()) {  // If there is an XrInstance already alive.
        LoaderLogger::LogErrorMessage("xrCreateInstance", "Loader does not support simultaneous XrInstances");
        return XR_ERROR_LIMIT_REACHED;
    }

    std::vector<std::unique_ptr<ApiLayerInterface>> api_layer_interfaces;
    XrResult result;

    // Make sure only one thread is attempting to read the JSON files and use the instance.
    {
        std::unique_lock<std::mutex> json_lock(g_loader_json_mutex);
        // Load the available runtime
        result = RuntimeInterface::LoadRuntime("xrCreateInstance");
        if (XR_FAILED(result)) {
            LoaderLogger::LogErrorMessage("xrCreateInstance", "Failed loading runtime information");
        } else {
            runtime_loaded = true;
            // Load the appropriate layers
            result = ApiLayerInterface::LoadApiLayers("xrCreateInstance", info->enabledApiLayerCount, info->enabledApiLayerNames,
                                                      api_layer_interfaces);
            if (XR_FAILED(result)) {
                LoaderLogger::LogErrorMessage("xrCreateInstance", "Failed loading layer information");
            }
        }
    }

    if (XR_FAILED(result)) {
        if (runtime_loaded) {
            RuntimeInterface::UnloadRuntime("xrCreateInstance");
        }
        return result;
    }

    // Create the loader instance (only send down first runtime interface)

    LoaderInstance *loader_instance = nullptr;
    {
        std::unique_ptr<LoaderInstance> owned_loader_instance;
        result = LoaderInstance::CreateInstance(LoaderXrTermGetInstanceProcAddr, LoaderXrTermCreateInstance,
                                                LoaderXrTermCreateApiLayerInstance, std::move(api_layer_interfaces), info,
                                                &owned_loader_instance);
        if (XR_SUCCEEDED(result)) {
            loader_instance = owned_loader_instance.get();
            result = ActiveLoaderInstance::Set(std::move(owned_loader_instance), "xrCreateInstance");
        }
    }

    if (XR_SUCCEEDED(result)) {
        // Create a debug utils messenger if the create structure is in the "next" chain
        const auto *next_header = reinterpret_cast<const XrBaseInStructure *>(info->next);
        const XrDebugUtilsMessengerCreateInfoEXT *dbg_utils_create_info = nullptr;
        while (next_header != nullptr) {
            if (next_header->type == XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT) {
                LoaderLogger::LogInfoMessage("xrCreateInstance", "Found XrDebugUtilsMessengerCreateInfoEXT in \'next\' chain.");
                dbg_utils_create_info = reinterpret_cast<const XrDebugUtilsMessengerCreateInfoEXT *>(next_header);
                XrDebugUtilsMessengerEXT messenger;
                result = xrCreateDebugUtilsMessengerEXT(loader_instance->GetInstanceHandle(), dbg_utils_create_info, &messenger);
                if (XR_FAILED(result)) {
                    return XR_ERROR_VALIDATION_FAILURE;
                }
                loader_instance->SetDefaultDebugUtilsMessenger(messenger);
                break;
            }
            next_header = reinterpret_cast<const XrBaseInStructure *>(next_header->next);
        }
    }

    if (XR_SUCCEEDED(result)) {
        *instance = loader_instance->GetInstanceHandle();
    } else {
        // Ensure the global loader instance is destroyed if something went wrong.
        ActiveLoaderInstance::Remove();
    }

    LoaderLogger::LogVerboseMessage("xrCreateInstance", "Completed loader trampoline");
    return result;
}
XRLOADER_ABI_CATCH_FALLBACK

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrDestroyInstance(XrInstance instance) XRLOADER_ABI_TRY {
    LoaderLogger::LogVerboseMessage("xrDestroyInstance", "Entering loader trampoline");
    // Runtimes may detect XR_NULL_HANDLE provided as a required handle parameter and return XR_ERROR_HANDLE_INVALID. - 2.9
    if (XR_NULL_HANDLE == instance) {
        LoaderLogger::LogErrorMessage("xrDestroyInstance", "Instance handle is XR_NULL_HANDLE.");
        return XR_ERROR_HANDLE_INVALID;
    }

    std::unique_lock<std::mutex> loader_instance_lock(g_instance_create_destroy_mutex);

    LoaderInstance *loader_instance;
    XrResult result = ActiveLoaderInstance::Get(&loader_instance, "xrDestroyInstance");
    if (XR_FAILED(result)) {
        return result;
    }

    const std::unique_ptr<XrGeneratedDispatchTable> &dispatch_table = loader_instance->DispatchTable();

    // If we allocated a default debug utils messenger, free it
    XrDebugUtilsMessengerEXT messenger = loader_instance->DefaultDebugUtilsMessenger();
    if (messenger != XR_NULL_HANDLE) {
        xrDestroyDebugUtilsMessengerEXT(messenger);
    }

    // Now destroy the instance
    if (XR_FAILED(dispatch_table->DestroyInstance(instance))) {
        LoaderLogger::LogErrorMessage("xrDestroyInstance", "Unknown error occurred calling down chain");
    }

    // Get rid of the loader instance. This will make it possible to create another instance in the future.
    ActiveLoaderInstance::Remove();

    // Lock the instance create/destroy mutex
    LoaderLogger::LogVerboseMessage("xrDestroyInstance", "Completed loader trampoline");

    // Finally, unload the runtime if necessary
    RuntimeInterface::UnloadRuntime("xrDestroyInstance");

    return XR_SUCCESS;
}
XRLOADER_ABI_CATCH_FALLBACK

// ---- Core 1.0 manual loader terminator functions

// Validate that the applicationInfo structure in the XrInstanceCreateInfo is valid.
static XrResult ValidateApplicationInfo(const XrApplicationInfo &info) {
    if (IsMissingNullTerminator<XR_MAX_APPLICATION_NAME_SIZE>(info.applicationName)) {
        LoaderLogger::LogValidationErrorMessage("VUID-XrApplicationInfo-applicationName-parameter", "xrCreateInstance",
                                                "application name missing NULL terminator.");
        return XR_ERROR_NAME_INVALID;
    }
    if (IsMissingNullTerminator<XR_MAX_ENGINE_NAME_SIZE>(info.engineName)) {
        LoaderLogger::LogValidationErrorMessage("VUID-XrApplicationInfo-engineName-parameter", "xrCreateInstance",
                                                "engine name missing NULL terminator.");
        return XR_ERROR_NAME_INVALID;
    }
    if (strlen(info.applicationName) == 0) {
        LoaderLogger::LogErrorMessage("xrCreateInstance",
                                      "VUID-XrApplicationInfo-engineName-parameter: application name can not be empty.");
        return XR_ERROR_NAME_INVALID;
    }
    return XR_SUCCESS;
}

// Validate that the XrInstanceCreateInfo is valid
static XrResult ValidateInstanceCreateInfo(const XrInstanceCreateInfo *info) {
    // Should have a valid 'type'
    if (XR_TYPE_INSTANCE_CREATE_INFO != info->type) {
        LoaderLogger::LogValidationErrorMessage("VUID-XrInstanceCreateInfo-type-type", "xrCreateInstance",
                                                "expected XR_TYPE_INSTANCE_CREATE_INFO.");
        return XR_ERROR_VALIDATION_FAILURE;
    }
    // Flags must be 0
    if (0 != info->createFlags) {
        LoaderLogger::LogValidationErrorMessage("VUID-XrInstanceCreateInfo-createFlags-zerobitmask", "xrCreateInstance",
                                                "flags must be 0.");
        return XR_ERROR_VALIDATION_FAILURE;
    }
    // ApplicationInfo struct must be valid
    XrResult result = ValidateApplicationInfo(info->applicationInfo);
    if (XR_FAILED(result)) {
        LoaderLogger::LogValidationErrorMessage("VUID-XrInstanceCreateInfo-applicationInfo-parameter", "xrCreateInstance",
                                                "info->applicationInfo is not valid.");
        return result;
    }
    // VUID-XrInstanceCreateInfo-enabledApiLayerNames-parameter already tested in LoadApiLayers()
    if ((info->enabledExtensionCount != 0u) && nullptr == info->enabledExtensionNames) {
        LoaderLogger::LogValidationErrorMessage("VUID-XrInstanceCreateInfo-enabledExtensionNames-parameter", "xrCreateInstance",
                                                "enabledExtensionCount is non-0 but array is NULL");
        return XR_ERROR_VALIDATION_FAILURE;
    }
    return XR_SUCCESS;
}

XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermCreateInstance(const XrInstanceCreateInfo *createInfo,
                                                          XrInstance *instance) XRLOADER_ABI_TRY {
    LoaderLogger::LogVerboseMessage("xrCreateInstance", "Entering loader terminator");
    XrResult result = ValidateInstanceCreateInfo(createInfo);
    if (XR_FAILED(result)) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrCreateInstance-info-parameter", "xrCreateInstance",
                                                "something wrong with XrInstanceCreateInfo contents");
        return result;
    }
    result = RuntimeInterface::GetRuntime().CreateInstance(createInfo, instance);
    LoaderLogger::LogVerboseMessage("xrCreateInstance", "Completed loader terminator");
    return result;
}
XRLOADER_ABI_CATCH_FALLBACK

XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermCreateApiLayerInstance(const XrInstanceCreateInfo *info,
                                                                  const struct XrApiLayerCreateInfo * /*apiLayerInfo*/,
                                                                  XrInstance *instance) {
    return LoaderXrTermCreateInstance(info, instance);
}

XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermDestroyInstance(XrInstance instance) XRLOADER_ABI_TRY {
    LoaderLogger::LogVerboseMessage("xrDestroyInstance", "Entering loader terminator");
    XrResult result = RuntimeInterface::GetRuntime().DestroyInstance(instance);
    LoaderLogger::LogVerboseMessage("xrDestroyInstance", "Completed loader terminator");
    return result;
}
XRLOADER_ABI_CATCH_FALLBACK

XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermGetInstanceProcAddr(XrInstance instance, const char *name,
                                                               PFN_xrVoidFunction *function) XRLOADER_ABI_TRY {
    // A few instance commands need to go through a loader terminator.
    // Otherwise, go directly to the runtime version of the command if it exists.
    // But first set the function pointer to NULL so that the fall-through below actually works.
    *function = nullptr;

    // NOTE: ActiveLoaderInstance cannot be used in this function because it is called before an instance is made active.

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

// ---- Extension manual loader trampoline functions

XRAPI_ATTR XrResult XRAPI_CALL xrCreateDebugUtilsMessengerEXT(XrInstance instance,
                                                              const XrDebugUtilsMessengerCreateInfoEXT *createInfo,
                                                              XrDebugUtilsMessengerEXT *messenger) XRLOADER_ABI_TRY {
    LoaderLogger::LogVerboseMessage("xrCreateDebugUtilsMessengerEXT", "Entering loader trampoline");

    if (instance == XR_NULL_HANDLE) {
        LoaderLogger::LogErrorMessage("xrCreateDebugUtilsMessengerEXT", "Instance handle is XR_NULL_HANDLE.");
        return XR_ERROR_HANDLE_INVALID;
    }

    LoaderInstance *loader_instance;
    XrResult result = ActiveLoaderInstance::Get(&loader_instance, "xrCreateDebugUtilsMessengerEXT");
    if (XR_FAILED(result)) {
        return result;
    }

    result = loader_instance->DispatchTable()->CreateDebugUtilsMessengerEXT(instance, createInfo, messenger);
    LoaderLogger::LogVerboseMessage("xrCreateDebugUtilsMessengerEXT", "Completed loader trampoline");
    return result;
}
XRLOADER_ABI_CATCH_BAD_ALLOC_OOM XRLOADER_ABI_CATCH_FALLBACK

    XRAPI_ATTR XrResult XRAPI_CALL
    xrDestroyDebugUtilsMessengerEXT(XrDebugUtilsMessengerEXT messenger) XRLOADER_ABI_TRY {
    // TODO: get instance from messenger in loader
    // Also, is the loader really doing all this every call?
    LoaderLogger::LogVerboseMessage("xrDestroyDebugUtilsMessengerEXT", "Entering loader trampoline");

    if (messenger == XR_NULL_HANDLE) {
        LoaderLogger::LogErrorMessage("xrDestroyDebugUtilsMessengerEXT", "Messenger handle is XR_NULL_HANDLE.");
        return XR_ERROR_HANDLE_INVALID;
    }

    LoaderInstance *loader_instance;
    XrResult result = ActiveLoaderInstance::Get(&loader_instance, "xrDestroyDebugUtilsMessengerEXT");
    if (XR_FAILED(result)) {
        return result;
    }

    result = loader_instance->DispatchTable()->DestroyDebugUtilsMessengerEXT(messenger);
    LoaderLogger::LogVerboseMessage("xrDestroyDebugUtilsMessengerEXT", "Completed loader trampoline");
    return result;
}
XRLOADER_ABI_CATCH_FALLBACK

XRAPI_ATTR XrResult XRAPI_CALL xrSessionBeginDebugUtilsLabelRegionEXT(XrSession session,
                                                                      const XrDebugUtilsLabelEXT *labelInfo) XRLOADER_ABI_TRY {
    if (session == XR_NULL_HANDLE) {
        LoaderLogger::LogErrorMessage("xrSessionBeginDebugUtilsLabelRegionEXT", "Session handle is XR_NULL_HANDLE.");
        return XR_ERROR_HANDLE_INVALID;
    }

    if (nullptr == labelInfo) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrSessionBeginDebugUtilsLabelRegionEXT-labelInfo-parameter",
                                                "xrSessionBeginDebugUtilsLabelRegionEXT", "labelInfo must be non-NULL",
                                                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_VALIDATION_FAILURE;
    }

    LoaderInstance *loader_instance;
    XrResult result = ActiveLoaderInstance::Get(&loader_instance, "xrSessionBeginDebugUtilsLabelRegionEXT");
    if (XR_FAILED(result)) {
        return result;
    }
    LoaderLogger::GetInstance().BeginLabelRegion(session, labelInfo);
    const std::unique_ptr<XrGeneratedDispatchTable> &dispatch_table = loader_instance->DispatchTable();
    if (nullptr != dispatch_table->SessionBeginDebugUtilsLabelRegionEXT) {
        return dispatch_table->SessionBeginDebugUtilsLabelRegionEXT(session, labelInfo);
    }
    return XR_SUCCESS;
}
XRLOADER_ABI_CATCH_FALLBACK

XRAPI_ATTR XrResult XRAPI_CALL xrSessionEndDebugUtilsLabelRegionEXT(XrSession session) XRLOADER_ABI_TRY {
    if (session == XR_NULL_HANDLE) {
        LoaderLogger::LogErrorMessage("xrSessionEndDebugUtilsLabelRegionEXT", "Session handle is XR_NULL_HANDLE.");
        return XR_ERROR_HANDLE_INVALID;
    }

    LoaderInstance *loader_instance;
    XrResult result = ActiveLoaderInstance::Get(&loader_instance, "xrSessionEndDebugUtilsLabelRegionEXT");
    if (XR_FAILED(result)) {
        return result;
    }

    LoaderLogger::GetInstance().EndLabelRegion(session);
    const std::unique_ptr<XrGeneratedDispatchTable> &dispatch_table = loader_instance->DispatchTable();
    if (nullptr != dispatch_table->SessionEndDebugUtilsLabelRegionEXT) {
        return dispatch_table->SessionEndDebugUtilsLabelRegionEXT(session);
    }
    return XR_SUCCESS;
}
XRLOADER_ABI_CATCH_FALLBACK

XRAPI_ATTR XrResult XRAPI_CALL xrSessionInsertDebugUtilsLabelEXT(XrSession session,
                                                                 const XrDebugUtilsLabelEXT *labelInfo) XRLOADER_ABI_TRY {
    if (session == XR_NULL_HANDLE) {
        LoaderLogger::LogErrorMessage("xrSessionInsertDebugUtilsLabelEXT", "Session handle is XR_NULL_HANDLE.");
        return XR_ERROR_HANDLE_INVALID;
    }

    LoaderInstance *loader_instance;
    XrResult result = ActiveLoaderInstance::Get(&loader_instance, "xrSessionInsertDebugUtilsLabelEXT");
    if (XR_FAILED(result)) {
        return result;
    }

    if (nullptr == labelInfo) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrSessionInsertDebugUtilsLabelEXT-labelInfo-parameter",
                                                "xrSessionInsertDebugUtilsLabelEXT", "labelInfo must be non-NULL",
                                                {XrSdkLogObjectInfo{session, XR_OBJECT_TYPE_SESSION}});
        return XR_ERROR_VALIDATION_FAILURE;
    }

    LoaderLogger::GetInstance().InsertLabel(session, labelInfo);

    const std::unique_ptr<XrGeneratedDispatchTable> &dispatch_table = loader_instance->DispatchTable();
    if (nullptr != dispatch_table->SessionInsertDebugUtilsLabelEXT) {
        return dispatch_table->SessionInsertDebugUtilsLabelEXT(session, labelInfo);
    }

    return XR_SUCCESS;
}
XRLOADER_ABI_CATCH_FALLBACK

// No-op trampoline needed for xrGetInstanceProcAddr. Work done in terminator.
XRAPI_ATTR XrResult XRAPI_CALL xrSetDebugUtilsObjectNameEXT(XrInstance instance,
                                                            const XrDebugUtilsObjectNameInfoEXT *nameInfo) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance;
    XrResult result = ActiveLoaderInstance::Get(&loader_instance, "xrSetDebugUtilsObjectNameEXT");
    if (XR_SUCCEEDED(result)) {
        result = loader_instance->DispatchTable()->SetDebugUtilsObjectNameEXT(instance, nameInfo);
    }
    return result;
}
XRLOADER_ABI_CATCH_FALLBACK

// No-op trampoline needed for xrGetInstanceProcAddr. Work done in terminator.
XRAPI_ATTR XrResult XRAPI_CALL xrSubmitDebugUtilsMessageEXT(
    XrInstance instance, XrDebugUtilsMessageSeverityFlagsEXT messageSeverity, XrDebugUtilsMessageTypeFlagsEXT messageTypes,
    const XrDebugUtilsMessengerCallbackDataEXT *callbackData) XRLOADER_ABI_TRY {
    LoaderInstance *loader_instance;
    XrResult result = ActiveLoaderInstance::Get(&loader_instance, "xrSubmitDebugUtilsMessageEXT");
    if (XR_SUCCEEDED(result)) {
        result =
            loader_instance->DispatchTable()->SubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, callbackData);
    }
    return result;
}
XRLOADER_ABI_CATCH_FALLBACK

// ---- Extension manual loader terminator functions

XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermCreateDebugUtilsMessengerEXT(XrInstance instance,
                                                                        const XrDebugUtilsMessengerCreateInfoEXT *createInfo,
                                                                        XrDebugUtilsMessengerEXT *messenger) XRLOADER_ABI_TRY {
    LoaderLogger::LogVerboseMessage("xrCreateDebugUtilsMessengerEXT", "Entering loader terminator");
    if (nullptr == messenger) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrCreateDebugUtilsMessengerEXT-messenger-parameter",
                                                "xrCreateDebugUtilsMessengerEXT", "invalid messenger pointer");
        return XR_ERROR_VALIDATION_FAILURE;
    }
    const XrGeneratedDispatchTable *dispatch_table = RuntimeInterface::GetDispatchTable(instance);
    XrResult result = XR_SUCCESS;
    // This extension is supported entirely by the loader which means the runtime may or may not support it.
    if (nullptr != dispatch_table->CreateDebugUtilsMessengerEXT) {
        result = dispatch_table->CreateDebugUtilsMessengerEXT(instance, createInfo, messenger);
    } else {
        // Just allocate a character so we have a unique value
        char *temp_mess_ptr = new char;
        *messenger = reinterpret_cast<XrDebugUtilsMessengerEXT>(temp_mess_ptr);
    }
    if (XR_SUCCEEDED(result)) {
        LoaderLogger::GetInstance().AddLogRecorder(MakeDebugUtilsLoaderLogRecorder(createInfo, *messenger));
        RuntimeInterface::GetRuntime().TrackDebugMessenger(instance, *messenger);
    }
    LoaderLogger::LogVerboseMessage("xrCreateDebugUtilsMessengerEXT", "Completed loader terminator");
    return result;
}
XRLOADER_ABI_CATCH_FALLBACK

XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermDestroyDebugUtilsMessengerEXT(XrDebugUtilsMessengerEXT messenger) XRLOADER_ABI_TRY {
    LoaderLogger::LogVerboseMessage("xrDestroyDebugUtilsMessengerEXT", "Entering loader terminator");
    const XrGeneratedDispatchTable *dispatch_table = RuntimeInterface::GetDebugUtilsMessengerDispatchTable(messenger);
    XrResult result = XR_SUCCESS;
    LoaderLogger::GetInstance().RemoveLogRecorder(MakeHandleGeneric(messenger));
    RuntimeInterface::GetRuntime().ForgetDebugMessenger(messenger);
    // This extension is supported entirely by the loader which means the runtime may or may not support it.
    if (nullptr != dispatch_table->DestroyDebugUtilsMessengerEXT) {
        result = dispatch_table->DestroyDebugUtilsMessengerEXT(messenger);
    } else {
        // Delete the character we would've created
        delete (reinterpret_cast<char *>(MakeHandleGeneric(messenger)));
    }
    LoaderLogger::LogVerboseMessage("xrDestroyDebugUtilsMessengerEXT", "Completed loader terminator");
    return result;
}
XRLOADER_ABI_CATCH_FALLBACK

XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermSubmitDebugUtilsMessageEXT(
    XrInstance instance, XrDebugUtilsMessageSeverityFlagsEXT messageSeverity, XrDebugUtilsMessageTypeFlagsEXT messageTypes,
    const XrDebugUtilsMessengerCallbackDataEXT *callbackData) XRLOADER_ABI_TRY {
    LoaderLogger::LogVerboseMessage("xrSubmitDebugUtilsMessageEXT", "Entering loader terminator");
    const XrGeneratedDispatchTable *dispatch_table = RuntimeInterface::GetDispatchTable(instance);
    XrResult result = XR_SUCCESS;
    if (nullptr != dispatch_table->SubmitDebugUtilsMessageEXT) {
        result = dispatch_table->SubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, callbackData);
    } else {
        // Only log the message from the loader if the runtime doesn't support this extension.  If we did,
        // then the user would receive multiple instances of the same message.
        LoaderLogger::GetInstance().LogDebugUtilsMessage(messageSeverity, messageTypes, callbackData);
    }
    LoaderLogger::LogVerboseMessage("xrSubmitDebugUtilsMessageEXT", "Completed loader terminator");
    return result;
}
XRLOADER_ABI_CATCH_FALLBACK

XRAPI_ATTR XrResult XRAPI_CALL
LoaderXrTermSetDebugUtilsObjectNameEXT(XrInstance instance, const XrDebugUtilsObjectNameInfoEXT *nameInfo) XRLOADER_ABI_TRY {
    LoaderLogger::LogVerboseMessage("xrSetDebugUtilsObjectNameEXT", "Entering loader terminator");
    const XrGeneratedDispatchTable *dispatch_table = RuntimeInterface::GetDispatchTable(instance);
    XrResult result = XR_SUCCESS;
    if (nullptr != dispatch_table->SetDebugUtilsObjectNameEXT) {
        result = dispatch_table->SetDebugUtilsObjectNameEXT(instance, nameInfo);
    }
    LoaderLogger::GetInstance().AddObjectName(nameInfo->objectHandle, nameInfo->objectType, nameInfo->objectName);
    LoaderLogger::LogVerboseMessage("xrSetDebugUtilsObjectNameEXT", "Completed loader terminator");
    return result;
}
XRLOADER_ABI_CATCH_FALLBACK

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetInstanceProcAddr(XrInstance instance, const char *name,
                                                                   PFN_xrVoidFunction *function) XRLOADER_ABI_TRY {
    // Initialize the function to nullptr in case it does not get caught in a known case
    *function = nullptr;

    if (nullptr == function) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrGetInstanceProcAddr-function-parameter", "xrGetInstanceProcAddr",
                                                "Invalid Function pointer");
        return XR_ERROR_VALIDATION_FAILURE;
    }

    if (nullptr == name) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrGetInstanceProcAddr-function-parameter", "xrGetInstanceProcAddr",
                                                "Invalid Name pointer");
        return XR_ERROR_VALIDATION_FAILURE;
    }

    if (instance == XR_NULL_HANDLE) {
        // Null instance is allowed for 3 specific API entry points, otherwise return error
        if (strcmp(name, "xrCreateInstance") != 0 && strcmp(name, "xrEnumerateApiLayerProperties") != 0 &&
            strcmp(name, "xrEnumerateInstanceExtensionProperties") != 0) {
            std::string error_str = "XR_NULL_HANDLE for instance but query for ";
            error_str += name;
            error_str += " requires a valid instance";
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetInstanceProcAddr-instance-parameter", "xrGetInstanceProcAddr",
                                                    error_str);
            return XR_ERROR_HANDLE_INVALID;
        }
    }

    // These functions must always go through the loader's implementation (trampoline).
    if (strcmp(name, "xrGetInstanceProcAddr") == 0) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetInstanceProcAddr);
        return XR_SUCCESS;
    } else if (strcmp(name, "xrEnumerateApiLayerProperties") == 0) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(xrEnumerateApiLayerProperties);
        return XR_SUCCESS;
    } else if (strcmp(name, "xrEnumerateInstanceExtensionProperties") == 0) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(xrEnumerateInstanceExtensionProperties);
        return XR_SUCCESS;
    } else if (strcmp(name, "xrCreateInstance") == 0) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(xrCreateInstance);
        return XR_SUCCESS;
    } else if (strcmp(name, "xrDestroyInstance") == 0) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(xrDestroyInstance);
        return XR_SUCCESS;
    }

    // Remainder of the functions require the LoaderInstance.
    LoaderInstance *loader_instance = nullptr;
    XrResult result = ActiveLoaderInstance::Get(&loader_instance, "xrGetInstanceProcAddr");
    if (XR_FAILED(result)) {
        return result;
    }

    // XR_EXT_debug_utils is built into the loader and handled partly through the xrGetInstanceProcAddress terminator,
    // but the check to see if the extension is enabled must be done here where ActiveLoaderInstance is safe to use.
    if (*function == nullptr) {
        if (strcmp(name, "xrCreateDebugUtilsMessengerEXT") == 0) {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrCreateDebugUtilsMessengerEXT);
        } else if (strcmp(name, "xrDestroyDebugUtilsMessengerEXT") == 0) {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrDestroyDebugUtilsMessengerEXT);
        } else if (strcmp(name, "xrSessionBeginDebugUtilsLabelRegionEXT") == 0) {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrSessionBeginDebugUtilsLabelRegionEXT);
        } else if (strcmp(name, "xrSessionEndDebugUtilsLabelRegionEXT") == 0) {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrSessionEndDebugUtilsLabelRegionEXT);
        } else if (strcmp(name, "xrSessionInsertDebugUtilsLabelEXT") == 0) {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrSessionInsertDebugUtilsLabelEXT);
        } else if (strcmp(name, "xrSetDebugUtilsObjectNameEXT") == 0) {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrSetDebugUtilsObjectNameEXT);
        } else if (strcmp(name, "xrSubmitDebugUtilsMessageEXT") == 0) {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrSubmitDebugUtilsMessageEXT);
        }

        if (*function != nullptr && !loader_instance->ExtensionIsEnabled("XR_EXT_debug_utils")) {
            // The function matches one of the XR_EXT_debug_utils functions but the extension is not enabled.
            *function = nullptr;
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }
    }

    if (*function != nullptr) {
        // The loader has a trampoline or implementation of this function.
        return XR_SUCCESS;
    }

    // If the function is not supported by the loader, call down to the next layer.
    return loader_instance->GetInstanceProcAddr(name, function);
}
XRLOADER_ABI_CATCH_FALLBACK
