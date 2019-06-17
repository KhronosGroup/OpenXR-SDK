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

#ifdef XR_OS_WINDOWS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <cstring>
#include <string>
#include <mutex>
#include <memory>

#include "xr_dependencies.h"
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include "loader_logger.hpp"
#include "loader_instance.hpp"
#include "xr_generated_loader.cpp"

// Flag to cause the one time to init to only occur one time.
std::once_flag g_one_time_init_flag;

// Global lock to prevent reading JSON manifest files at the same time.
static std::mutex g_loader_json_mutex;

// Global lock to prevent simultaneous instance creation/destruction
static std::mutex g_loader_instance_mutex;

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

extern "C" {

// ---- Core 0.1 manual loader trampoline functions

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateApiLayerProperties(uint32_t propertyCapacityInput,
                                                                           uint32_t *propertyCountOutput,
                                                                           XrApiLayerProperties *properties) {
    try {
        LoaderLogger::LogVerboseMessage("xrEnumerateApiLayerProperties", "Entering loader trampoline");

        // Make sure only one thread is attempting to read the JSON files at a time.
        std::unique_lock<std::mutex> json_lock(g_loader_json_mutex);

        XrResult result = ApiLayerInterface::GetApiLayerProperties("xrEnumerateApiLayerProperties", propertyCapacityInput,
                                                                   propertyCountOutput, properties);
        if (XR_SUCCESS != result) {
            LoaderLogger::LogErrorMessage("xrEnumerateApiLayerProperties", "Failed ApiLayerInterface::GetApiLayerProperties");
        }

        return result;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrEnumerateApiLayerProperties", "Unknown error occurred");
        return XR_ERROR_VALIDATION_FAILURE;
    }

    return XR_SUCCESS;
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateInstanceExtensionProperties(const char *layerName,
                                                                                    uint32_t propertyCapacityInput,
                                                                                    uint32_t *propertyCountOutput,
                                                                                    XrExtensionProperties *properties) {
    try {
        bool just_layer_properties = false;
        LoaderLogger::LogVerboseMessage("xrEnumerateInstanceExtensionProperties", "Entering loader trampoline");

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
            if (XR_SUCCESS == result && !just_layer_properties) {
                // If not specific to a layer, get the runtime extension properties
                result = RuntimeInterface::LoadRuntime("xrEnumerateInstanceExtensionProperties");
                if (XR_SUCCESS == result) {
                    RuntimeInterface::GetRuntime().GetInstanceExtensionProperties(extension_properties);
                    RuntimeInterface::UnloadRuntime("xrEnumerateInstanceExtensionProperties");
                } else {
                    LoaderLogger::LogErrorMessage("xrEnumerateInstanceExtensionProperties",
                                                  "Failed to find default runtime with RuntimeInterface::LoadRuntime()");
                }
            }
        }

        if (XR_SUCCESS != result) {
            LoaderLogger::LogErrorMessage("xrEnumerateInstanceExtensionProperties", "Failed querying extension properties");
            return result;
        }

        // If this is not in reference to a specific layer, then add the loader-specific extension properties as well.
        // These are extensions that the loader directly supports.
        if (!just_layer_properties) {
            auto loader_extension_props = LoaderInstance::LoaderSpecificExtensions();
            for (XrExtensionProperties &loader_prop : loader_extension_props) {
                bool found_prop = false;
                for (XrExtensionProperties &existing_prop : extension_properties) {
                    if (0 == strcmp(existing_prop.extensionName, loader_prop.extensionName)) {
                        found_prop = true;
                        // Use the loader version if it is newer
                        if (existing_prop.specVersion < loader_prop.specVersion) {
                            existing_prop.specVersion = loader_prop.specVersion;
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

        uint32_t num_extension_properties = static_cast<uint32_t>(extension_properties.size());
        if (propertyCapacityInput == 0) {
            *propertyCountOutput = num_extension_properties;
        } else if (nullptr != properties) {
            if (propertyCapacityInput < num_extension_properties) {
                *propertyCountOutput = num_extension_properties;
                LoaderLogger::LogErrorMessage(
                    "xrEnumerateInstanceExtensionProperties",
                    "VUID-xrEnumerateInstanceExtensionProperties-propertyCountOutput-parameter: insufficient space in array");
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
                    LoaderLogger::LogErrorMessage("xrEnumerateInstanceExtensionProperties",
                                                  "VUID-XrExtensionProperties-type-type: unknown type in properties");
                }
                if (nullptr != properties[prop].next) {
                    LoaderLogger::LogErrorMessage("xrEnumerateInstanceExtensionProperties",
                                                  "VUID-XrExtensionProperties-next-next: expected NULL");
                    properties_valid = false;
                }
                if (properties_valid) {
                    properties[prop] = extension_properties[prop];
                }
            }
            if (!properties_valid) {
                LoaderLogger::LogErrorMessage(
                    "xrEnumerateInstanceExtensionProperties",
                    "VUID-xrEnumerateInstanceExtensionProperties-properties-parameter: invalid properties");
                return XR_ERROR_VALIDATION_FAILURE;
            } else if (nullptr != propertyCountOutput) {
                *propertyCountOutput = num_to_copy;
            }
        }
        LoaderLogger::LogVerboseMessage("xrEnumerateInstanceExtensionProperties", "Completed loader trampoline");
        return XR_SUCCESS;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrEnumerateInstanceExtensionProperties", "Unknown error occurred");
        return XR_ERROR_INITIALIZATION_FAILED;
    }
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrCreateInstance(const XrInstanceCreateInfo *info, XrInstance *instance) {
    bool runtime_loaded = false;

    try {
        LoaderLogger::LogVerboseMessage("xrCreateInstance", "Entering loader trampoline");
        if (nullptr == info) {
            LoaderLogger::LogErrorMessage("xrCreateInstance", "VUID-xrCreateInstance-info-parameter: must be non-NULL");
            return XR_ERROR_VALIDATION_FAILURE;
        } else {
            // If application requested OpenXR API version is higher than the loader version, then we need to throw
            // an error.
            uint16_t app_major = XR_VERSION_MAJOR(info->applicationInfo.apiVersion);
            uint16_t app_minor = XR_VERSION_MINOR(info->applicationInfo.apiVersion);
            uint16_t loader_major = XR_VERSION_MAJOR(XR_CURRENT_API_VERSION);
            uint16_t loader_minor = XR_VERSION_MINOR(XR_CURRENT_API_VERSION);
            if (app_major > loader_major || (app_major == loader_major && app_minor > loader_minor)) {
                std::string error_message = "xrCreateInstance called with invalid API version ";
                error_message += std::to_string(app_major);
                error_message += ".";
                error_message += std::to_string(app_minor);
                error_message += ".  Max supported version is ";
                error_message += std::to_string(loader_major);
                error_message += ".";
                error_message += std::to_string(loader_minor);
                LoaderLogger::LogErrorMessage("xrCreateInstance", error_message);
                return XR_ERROR_DRIVER_INCOMPATIBLE;
            }
        }
        if (nullptr == instance) {
            LoaderLogger::LogErrorMessage("xrCreateInstance", "VUID-xrCreateInstance-instance-parameter: must be non-NULL");
            return XR_ERROR_HANDLE_INVALID;
        }

        std::vector<std::unique_ptr<ApiLayerInterface>> api_layer_interfaces;

        // Make sure only one thread is attempting to read the JSON files and use the instance.
        XrResult result;
        {
            std::unique_lock<std::mutex> json_lock(g_loader_json_mutex);
            // Load the available runtime
            result = RuntimeInterface::LoadRuntime("xrCreateInstance");
            if (XR_SUCCESS != result) {
                LoaderLogger::LogErrorMessage("xrCreateInstance", "Failed loading runtime information");
            } else {
                runtime_loaded = true;
                // Load the appropriate layers
                result = ApiLayerInterface::LoadApiLayers("xrCreateInstance", info->enabledApiLayerCount,
                                                          info->enabledApiLayerNames, api_layer_interfaces);
                if (XR_SUCCESS != result) {
                    LoaderLogger::LogErrorMessage("xrCreateInstance", "Failed loading layer information");
                }
            }
        }

        if (XR_SUCCESS != result) {
            if (runtime_loaded) {
                RuntimeInterface::UnloadRuntime("xrCreateInstance");
            }
            return result;
        }

        std::unique_lock<std::mutex> instance_lock(g_loader_instance_mutex);

        // Create the loader instance (only send down first runtime interface)
        XrInstance created_instance = XR_NULL_HANDLE;
        result = LoaderInstance::CreateInstance(api_layer_interfaces, info, &created_instance);

        if (XR_SUCCESS == result) {
            *instance = created_instance;

            LoaderInstance *loader_instance;
            {
                std::unique_lock<std::mutex> lock(g_instance_mutex);
                loader_instance = g_instance_map[created_instance];
            }

            // Create a debug utils messenger if the create structure is in the "next" chain
            const XrBaseInStructure *next_header = reinterpret_cast<const XrBaseInStructure *>(info->next);
            const XrDebugUtilsMessengerCreateInfoEXT *dbg_utils_create_info = nullptr;
            while (next_header != nullptr) {
                if (next_header->type == XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT) {
                    LoaderLogger::LogInfoMessage("xrCreateInstance", "Found XrDebugUtilsMessengerCreateInfoEXT in \'next\' chain.");
                    dbg_utils_create_info = reinterpret_cast<const XrDebugUtilsMessengerCreateInfoEXT *>(next_header);
                    XrDebugUtilsMessengerEXT messenger;
                    result = xrCreateDebugUtilsMessengerEXT(*instance, dbg_utils_create_info, &messenger);
                    if (XR_SUCCESS != result) {
                        return XR_ERROR_VALIDATION_FAILURE;
                    }
                    loader_instance->SetDefaultDebugUtilsMessenger(messenger);
                    break;
                }
                next_header = reinterpret_cast<const XrBaseInStructure *>(next_header->next);
            }
        }

        LoaderLogger::LogVerboseMessage("xrCreateInstance", "Completed loader trampoline");
        return result;
    } catch (std::bad_alloc &) {
        if (runtime_loaded) {
            RuntimeInterface::UnloadRuntime("xrCreateInstance");
        }
        LoaderLogger::LogErrorMessage("xrCreateInstance", "Failed to allocate memory");
        return XR_ERROR_OUT_OF_MEMORY;
    } catch (...) {
        if (runtime_loaded) {
            RuntimeInterface::UnloadRuntime("xrCreateInstance");
        }
        LoaderLogger::LogErrorMessage("xrCreateInstance", "Unknown error occurred");
        return XR_ERROR_INITIALIZATION_FAILED;
    }
}

LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrDestroyInstance(XrInstance instance) {
    try {
        LoaderLogger::LogVerboseMessage("xrDestroyInstance", "Entering loader trampoline");
        // XR_NULL_HANDLE is ignored, but valid, in a delete operation.
        if (XR_NULL_HANDLE == instance) {
            return XR_SUCCESS;
        }

        LoaderInstance *const loader_instance = TryLookupLoaderInstance(instance);
        if (loader_instance == nullptr) {
            LoaderLogger::LogErrorMessage("xrDestroyInstance", "VUID-xrDestroyInstance-instance-parameter: invalid instance");
            return XR_ERROR_HANDLE_INVALID;
        }

        const std::unique_ptr<XrGeneratedDispatchTable> &dispatch_table = loader_instance->DispatchTable();

        // If we allocated a default debug utils messenger, free it
        XrDebugUtilsMessengerEXT messenger = loader_instance->DefaultDebugUtilsMessenger();
        if (messenger != XR_NULL_HANDLE) {
            xrDestroyDebugUtilsMessengerEXT(messenger);
        }

        // Now destroy the instance
        if (XR_SUCCESS != dispatch_table->DestroyInstance(instance)) {
            LoaderLogger::LogErrorMessage("xrDestroyInstance", "Unknown error occurred calling down chain");
        }

        // Cleanup any map entries that may still be using this instance
        LoaderCleanUpMapsForInstance(loader_instance);

        // Lock the instance create/destroy mutex
        std::unique_lock<std::mutex> loader_instance_lock(g_loader_instance_mutex);
        delete loader_instance;
        LoaderLogger::LogVerboseMessage("xrDestroyInstance", "Completed loader trampoline");
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrDestroyInstance", "Unknown error occurred");
    }

    // Finally, unload the runtime if necessary
    RuntimeInterface::UnloadRuntime("xrDestroyInstance");

    return XR_SUCCESS;
}

// ---- Core 0.1 manual loader terminator functions

// Validate that the XrInstanceCreateInfo 'next' chain is valid.
static bool ValidateInstanceCreateInfoNextChain(LoaderInstance *loader_instance, const XrInstanceCreateInfo *info) {
    // See if there is a debug utils create structure in the "next" chain
    const XrBaseInStructure *next_header = reinterpret_cast<const XrBaseInStructure *>(info->next);
    while (next_header != nullptr) {
        if (next_header->type != XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT) {
            return false;
        }
        next_header = reinterpret_cast<const XrBaseInStructure *>(next_header->next);
    }
    return true;
}

// Validate that the applicationInfo structure in the XrInstanceCreateInfo is valid.
static bool ValidateApplicationInfo(LoaderInstance *loader_instance, const XrApplicationInfo &info) {
    if (IsMissingNullTerminator<XR_MAX_APPLICATION_NAME_SIZE>(info.applicationName)) {
        LoaderLogger::LogErrorMessage(
            "xrCreateInstance", "VUID-XrApplicationInfo-applicationName-parameter: application name missing NULL terminator.");
        return false;
    }
    if (IsMissingNullTerminator<XR_MAX_ENGINE_NAME_SIZE>(info.engineName)) {
        LoaderLogger::LogErrorMessage("xrCreateInstance",
                                      "VUID-XrApplicationInfo-engineName-parameter: engine name missing NULL terminator.");
        return false;
    }
    return true;
}

// Validate that the XrInstanceCreateInfo is valid
static bool ValidateInstanceCreateInfo(LoaderInstance *loader_instance, const XrInstanceCreateInfo *info) {
    // Should have a valid 'type'
    if (XR_TYPE_INSTANCE_CREATE_INFO != info->type) {
        LoaderLogger::LogErrorMessage("xrCreateInstance",
                                      "VUID-XrInstanceCreateInfo-type-type: expected XR_TYPE_INSTANCE_CREATE_INFO.");
        return false;
    }
    // Should have a valid 'next' chain
    if (!ValidateInstanceCreateInfoNextChain(loader_instance, info)) {
        LoaderLogger::LogErrorMessage("xrCreateInstance",
                                      "VUID-XrInstanceCreateInfo-next-next: unexpected struct in \'next\' chain.");
        return false;
    }
    // Flags must be 0
    if (0 != info->createFlags) {
        LoaderLogger::LogErrorMessage("xrCreateInstance", "VUID-XrInstanceCreateInfo-createFlags-zerobitmask: flags must be 0.");
        return false;
    }
    // ApplicationInfo struct must be valid
    if (!ValidateApplicationInfo(loader_instance, info->applicationInfo)) {
        LoaderLogger::LogErrorMessage("xrCreateInstance",
                                      "VUID-XrInstanceCreateInfo-applicationInfo-parameter: info->applicationInfo is not valid.");
        return false;
    }
    // VUID-XrInstanceCreateInfo-enabledApiLayerNames-parameter already tested in LoadApiLayers()
    if (info->enabledExtensionCount && nullptr == info->enabledExtensionNames) {
        LoaderLogger::LogErrorMessage(
            "xrCreateInstance",
            "VUID-XrInstanceCreateInfo-enabledExtensionNames-parameter: enabledExtensionCount is non-0 but array is NULL");
        return false;
    }
    return true;
}

XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermCreateInstance(const XrInstanceCreateInfo *info, XrInstance *instance) {
    LoaderLogger::LogVerboseMessage("xrCreateInstance", "Entering loader terminator");
    LoaderInstance *loader_instance = reinterpret_cast<LoaderInstance *>(*instance);
    if (!ValidateInstanceCreateInfo(loader_instance, info)) {
        LoaderLogger::LogErrorMessage("xrCreateInstance",
                                      "VUID-xrCreateInstance-info-parameter: something wrong with XrInstanceCreateInfo contents");
        return XR_ERROR_VALIDATION_FAILURE;
    }
    XrResult result = RuntimeInterface::GetRuntime().CreateInstance(info, instance);
    loader_instance->SetRuntimeInstance(*instance);
    LoaderLogger::LogVerboseMessage("xrCreateInstance", "Completed loader terminator");
    return result;
}

XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermCreateApiLayerInstance(const XrInstanceCreateInfo *info,
                                                                  const struct XrApiLayerCreateInfo *apiLayerInfo,
                                                                  XrInstance *instance) {
    return LoaderXrTermCreateInstance(info, instance);
}

XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermDestroyInstance(XrInstance instance) {
    XrResult result;
    LoaderLogger::LogVerboseMessage("xrDestroyInstance", "Entering loader terminator");
    result = RuntimeInterface::GetRuntime().DestroyInstance(instance);
    LoaderLogger::LogVerboseMessage("xrDestroyInstance", "Completed loader terminator");
    return result;
}

// ---- Extension manual loader trampoline functions

XRAPI_ATTR XrResult XRAPI_CALL xrCreateDebugUtilsMessengerEXT(XrInstance instance,
                                                              const XrDebugUtilsMessengerCreateInfoEXT *createInfo,
                                                              XrDebugUtilsMessengerEXT *messenger) {
    try {
        LoaderLogger::LogVerboseMessage("xrCreateDebugUtilsMessengerEXT", "Entering loader trampoline");

        LoaderInstance *loader_instance;
        {
            std::unique_lock<std::mutex> lock(g_instance_mutex);
            loader_instance = g_instance_map[instance];
        }

        if (!loader_instance->ExtensionIsEnabled(XR_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
            std::string error_str = "The ";
            error_str += XR_EXT_DEBUG_UTILS_EXTENSION_NAME;
            error_str += " extension has not been enabled prior to calling xrCreateDebugUtilsMessengerEXT";
            LoaderLogger::LogValidationErrorMessage("VUID-xrCreateDebugUtilsMessengerEXT-extension-notenabled",
                                                    "xrCreateDebugUtilsMessengerEXT", error_str);
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        const std::unique_ptr<XrGeneratedDispatchTable> &dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
        result = dispatch_table->CreateDebugUtilsMessengerEXT(instance, createInfo, messenger);
        if (XR_SUCCESS == result && nullptr != messenger) {
            auto exists = g_debugutilsmessengerext_map.find(*messenger);
            if (exists == g_debugutilsmessengerext_map.end()) {
                std::unique_lock<std::mutex> debugutilsmessengerlock(g_debugutilsmessengerext_mutex);
                g_debugutilsmessengerext_map[*messenger] = loader_instance;
            }
        }
        LoaderLogger::LogVerboseMessage("xrCreateDebugUtilsMessengerEXT", "Completed loader trampoline");
        return result;
    } catch (std::bad_alloc &) {
        LoaderLogger::LogErrorMessage("xrCreateDebugUtilsMessengerEXT",
                                      "xrCreateDebugUtilsMessengerEXT trampoline failed allocating memory");
        return XR_ERROR_OUT_OF_MEMORY;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrCreateDebugUtilsMessengerEXT",
                                      "xrCreateDebugUtilsMessengerEXT trampoline encountered an unknown error");
        return XR_ERROR_INITIALIZATION_FAILED;
    }
}

XRAPI_ATTR XrResult XRAPI_CALL xrDestroyDebugUtilsMessengerEXT(XrDebugUtilsMessengerEXT messenger) {
    // TODO: get instance from messenger in loader
    // Also, is the loader really doing all this every call?
    try {
        LoaderLogger::LogVerboseMessage("xrDestroyDebugUtilsMessengerEXT", "Entering loader trampoline");

        if (XR_NULL_HANDLE == messenger) {
            return XR_SUCCESS;
        }

        auto exists = g_debugutilsmessengerext_map.find(messenger);
        if (exists == g_debugutilsmessengerext_map.end()) {
            return XR_ERROR_DEBUG_UTILS_MESSENGER_INVALID_EXT;
        }

        LoaderInstance *loader_instance;
        {
            std::unique_lock<std::mutex> lock(g_debugutilsmessengerext_mutex);
            loader_instance = g_debugutilsmessengerext_map[messenger];
        }

        if (!loader_instance->ExtensionIsEnabled(XR_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
            std::string error_str = "The ";
            error_str += XR_EXT_DEBUG_UTILS_EXTENSION_NAME;
            error_str += " extension has not been enabled prior to calling xrDestroyDebugUtilsMessengerEXT";
            LoaderLogger::LogValidationErrorMessage("VUID-xrDestroyDebugUtilsMessengerEXT-extension-notenabled",
                                                    "xrDestroyDebugUtilsMessengerEXT", error_str);
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        const std::unique_ptr<XrGeneratedDispatchTable> &dispatch_table = loader_instance->DispatchTable();
        XrResult result = dispatch_table->DestroyDebugUtilsMessengerEXT(messenger);
        LoaderLogger::LogVerboseMessage("xrDestroyDebugUtilsMessengerEXT", "Completed loader trampoline");
        return result;
    } catch (...) {
        std::string error_message =
            "xrDestroyDebugUtilsMessengerEXT trampoline encountered an unknown error.  Likely XrDebugUtilsMessengerEXT 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void *>(messenger);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrDestroyDebugUtilsMessengerEXT", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}

// ---- Extension manual loader terminator functions

XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermCreateDebugUtilsMessengerEXT(XrInstance instance,
                                                                        const XrDebugUtilsMessengerCreateInfoEXT *createInfo,
                                                                        XrDebugUtilsMessengerEXT *messenger) {
    try {
        LoaderLogger::LogVerboseMessage("xrCreateDebugUtilsMessengerEXT", "Entering loader terminator");
        if (nullptr == messenger) {
            return XR_ERROR_VALIDATION_FAILURE;
        }
        const XrGeneratedDispatchTable *dispatch_table = RuntimeInterface::GetRuntime().GetDispatchTable(instance);
        XrResult result = XR_SUCCESS;
        // This extension is supported entirely by the loader which means the runtime may or may not support it.
        if (nullptr != dispatch_table->CreateDebugUtilsMessengerEXT) {
            result = dispatch_table->CreateDebugUtilsMessengerEXT(instance, createInfo, messenger);
        } else {
            // Just allocate a character so we have a unique value
            char *temp_mess_ptr = new char;
            *messenger = reinterpret_cast<XrDebugUtilsMessengerEXT>(temp_mess_ptr);
        }
        if (XR_SUCCESS == result) {
            std::unique_ptr<LoaderLogRecorder> base_recorder(new DebugUtilsLogRecorder(createInfo, *messenger));
            LoaderLogger::GetInstance().AddLogRecorder(base_recorder);
            RuntimeInterface::GetRuntime().TrackDebugMessenger(instance, *messenger);
        }
        LoaderLogger::LogVerboseMessage("xrCreateDebugUtilsMessengerEXT", "Completed loader terminator");
        return result;
    } catch (std::bad_alloc &) {
        LoaderLogger::LogErrorMessage("xrCreateDebugUtilsMessengerEXT", "Terminator failed allocating memory");
        return XR_ERROR_OUT_OF_MEMORY;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrCreateDebugUtilsMessengerEXT", "Terminator encountered an unknown error");
        return XR_ERROR_INITIALIZATION_FAILED;
    }
}

XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermDestroyDebugUtilsMessengerEXT(XrDebugUtilsMessengerEXT messenger) {
    try {
        LoaderLogger::LogVerboseMessage("xrDestroyDebugUtilsMessengerEXT", "Entering loader terminator");
        const XrGeneratedDispatchTable *dispatch_table =
            RuntimeInterface::GetRuntime().GetDebugUtilsMessengerDispatchTable(messenger);
        XrResult result = XR_SUCCESS;
        // This extension is supported entirely by the loader which means the runtime may or may not support it.
        if (nullptr != dispatch_table->DestroyDebugUtilsMessengerEXT) {
            result = dispatch_table->DestroyDebugUtilsMessengerEXT(messenger);
        } else {
            // Delete the character we would've created
            delete (reinterpret_cast<char *>(reinterpret_cast<uint64_t &>(messenger)));
        }
        LoaderLogger::LogVerboseMessage("xrDestroyDebugUtilsMessengerEXT", "Completed loader terminator");
        LoaderLogger::GetInstance().RemoveLogRecorder(reinterpret_cast<uint64_t &>(messenger));
        RuntimeInterface::GetRuntime().ForgetDebugMessenger(messenger);
        return result;
    } catch (...) {
        std::string error_message = "Terminator encountered an unknown error.  Likely XrDebugUtilsMessengerEXT 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void *>(messenger);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrDestroyDebugUtilsMessengerEXT", error_message);
        return XR_ERROR_DEBUG_UTILS_MESSENGER_INVALID_EXT;
    }
}

XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermSubmitDebugUtilsMessageEXT(XrInstance instance,
                                                                      XrDebugUtilsMessageSeverityFlagsEXT messageSeverity,
                                                                      XrDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                                      const XrDebugUtilsMessengerCallbackDataEXT *callbackData) {
    try {
        LoaderLogger::LogVerboseMessage("xrSubmitDebugUtilsMessageEXT", "Entering loader terminator");
        const XrGeneratedDispatchTable *dispatch_table = RuntimeInterface::GetRuntime().GetDispatchTable(instance);
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
    } catch (...) {
        std::string error_message = "xrSubmitDebugUtilsMessageEXT terminator encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void *>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrSubmitDebugUtilsMessageEXT", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}

XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermSetDebugUtilsObjectNameEXT(XrInstance instance,
                                                                      const XrDebugUtilsObjectNameInfoEXT *nameInfo) {
    try {
        LoaderLogger::LogVerboseMessage("xrSetDebugUtilsObjectNameEXT", "Entering loader terminator");
        const XrGeneratedDispatchTable *dispatch_table = RuntimeInterface::GetRuntime().GetDispatchTable(instance);
        XrResult result = XR_SUCCESS;
        if (nullptr != dispatch_table->SetDebugUtilsObjectNameEXT) {
            result = dispatch_table->SetDebugUtilsObjectNameEXT(instance, nameInfo);
        }
        LoaderLogger::GetInstance().AddObjectName(nameInfo->objectHandle, nameInfo->objectType, nameInfo->objectName);
        LoaderLogger::LogVerboseMessage("xrSetDebugUtilsObjectNameEXT", "Completed loader terminator");
        return result;
    } catch (...) {
        std::string error_message = "xrSetDebugUtilsObjectNameEXT terminator encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void *>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrSetDebugUtilsObjectNameEXT", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}

XRAPI_ATTR XrResult XRAPI_CALL xrSessionBeginDebugUtilsLabelRegionEXT(XrSession session, const XrDebugUtilsLabelEXT *labelInfo) {
    try {
        LoaderInstance *loader_instance;
        {
            std::unique_lock<std::mutex> lock(g_session_mutex);
            loader_instance = g_session_map[session];
        }

        std::vector<XrLoaderLogObjectInfo> loader_objects;
        XrLoaderLogObjectInfo object_info = {};
        object_info.type = XR_OBJECT_TYPE_SESSION;
        object_info.handle = reinterpret_cast<uint64_t &>(session);
        loader_objects.push_back(object_info);
        if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrSessionBeginDebugUtilsLabelRegionEXT-session-parameter",
                                                    "xrSessionBeginDebugUtilsLabelRegionEXT", "session is not a valid XrSession",
                                                    loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        if (!loader_instance->ExtensionIsEnabled(XR_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
            LoaderLogger::LogValidationErrorMessage("TBD", "xrSessionBeginDebugUtilsLabelRegionEXT",
                                                    "Extension entrypoint called without enabling appropriate extension",
                                                    loader_objects);
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        } else if (nullptr == labelInfo) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrSessionBeginDebugUtilsLabelRegionEXT-labelInfo-parameter",
                                                    "xrSessionBeginDebugUtilsLabelRegionEXT", "labelInfo must be non-NULL",
                                                    loader_objects);
            return XR_ERROR_VALIDATION_FAILURE;
        }

        LoaderLogger::GetInstance().BeginLabelRegion(session, labelInfo);
        const std::unique_ptr<XrGeneratedDispatchTable> &dispatch_table = loader_instance->DispatchTable();
        if (nullptr != dispatch_table->SessionBeginDebugUtilsLabelRegionEXT) {
            return dispatch_table->SessionBeginDebugUtilsLabelRegionEXT(session, labelInfo);
        }
        return XR_SUCCESS;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrSessionBeginDebugUtilsLabelRegionEXT",
                                      "xrSessionBeginDebugUtilsLabelRegionEXT trampoline encountered an unknown error");
        return XR_ERROR_VALIDATION_FAILURE;
    }
}

XRAPI_ATTR XrResult XRAPI_CALL xrSessionEndDebugUtilsLabelRegionEXT(XrSession session) {
    try {
        LoaderInstance *loader_instance;
        {
            std::unique_lock<std::mutex> lock(g_session_mutex);
            loader_instance = g_session_map[session];
        }

        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t &>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrSessionEndDebugUtilsLabelRegionEXT-session-parameter",
                                                    "xrSessionEndDebugUtilsLabelRegionEXT", "session is not a valid XrSession",
                                                    loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        } else if (!loader_instance->ExtensionIsEnabled(XR_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }
        LoaderLogger::GetInstance().EndLabelRegion(session);
        const std::unique_ptr<XrGeneratedDispatchTable> &dispatch_table = loader_instance->DispatchTable();
        if (nullptr != dispatch_table->SessionBeginDebugUtilsLabelRegionEXT) {
            return dispatch_table->SessionEndDebugUtilsLabelRegionEXT(session);
        }
        return XR_SUCCESS;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrSessionEndDebugUtilsLabelRegionEXT",
                                      "xrSessionEndDebugUtilsLabelRegionEXT trampoline encountered an unknown error");
        return XR_ERROR_VALIDATION_FAILURE;
    }
}

XRAPI_ATTR XrResult XRAPI_CALL xrSessionInsertDebugUtilsLabelEXT(XrSession session, const XrDebugUtilsLabelEXT *labelInfo) {
    try {
        LoaderInstance *loader_instance;
        {
            std::unique_lock<std::mutex> lock(g_session_mutex);
            loader_instance = g_session_map[session];
        }

        XrLoaderLogObjectInfo object_info = {};
        object_info.type = XR_OBJECT_TYPE_SESSION;
        object_info.handle = reinterpret_cast<uint64_t &>(session);
        std::vector<XrLoaderLogObjectInfo> loader_objects;
        loader_objects.push_back(object_info);
        if (nullptr == loader_instance) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrSessionInsertDebugUtilsLabelEXT-session-parameter",
                                                    "xrSessionInsertDebugUtilsLabelEXT", "session is not a valid XrSession",
                                                    loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        if (!loader_instance->ExtensionIsEnabled(XR_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
            LoaderLogger::LogValidationErrorMessage("TBD", "xrSessionInsertDebugUtilsLabelEXT",
                                                    "Extension entrypoint called without enabling appropriate extension",
                                                    loader_objects);
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        } else if (nullptr == labelInfo) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrSessionInsertDebugUtilsLabelEXT-labelInfo-parameter",
                                                    "xrSessionInsertDebugUtilsLabelEXT", "labelInfo must be non-NULL",
                                                    loader_objects);
            return XR_ERROR_VALIDATION_FAILURE;
        }

        LoaderLogger::GetInstance().InsertLabel(session, labelInfo);
        const std::unique_ptr<XrGeneratedDispatchTable> &dispatch_table = loader_instance->DispatchTable();
        if (nullptr != dispatch_table->SessionInsertDebugUtilsLabelEXT) {
            return dispatch_table->SessionInsertDebugUtilsLabelEXT(session, labelInfo);
        }
        return XR_SUCCESS;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrSessionInsertDebugUtilsLabelEXT",
                                      "xrSessionInsertDebugUtilsLabelEXT trampoline encountered an unknown error");
        return XR_ERROR_VALIDATION_FAILURE;
    }
}

}  // extern "C"
