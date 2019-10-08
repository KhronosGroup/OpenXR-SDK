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

#include "runtime_interface.hpp"

#include "manifest_file.hpp"
#include "loader_interfaces.h"
#include "loader_logger.hpp"
#include "loader_platform.hpp"
#include "xr_generated_dispatch_table.h"

#include <openxr/openxr.h>

#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

std::unique_ptr<RuntimeInterface> RuntimeInterface::_single_runtime_interface;
uint32_t RuntimeInterface::_single_runtime_count = 0;

XrResult RuntimeInterface::LoadRuntime(const std::string& openxr_command) {
    XrResult last_error = XR_SUCCESS;
    bool any_loaded = false;

    // If something's already loaded, we're done here.
    if (_single_runtime_interface != nullptr) {
        _single_runtime_count++;
        return XR_SUCCESS;
    }

    std::vector<std::unique_ptr<RuntimeManifestFile>> runtime_manifest_files = {};

    // Find the available runtimes which we may need to report information for.
    last_error = RuntimeManifestFile::FindManifestFiles(MANIFEST_TYPE_RUNTIME, runtime_manifest_files);
    if (XR_FAILED(last_error)) {
        LoaderLogger::LogErrorMessage(openxr_command, "RuntimeInterface::LoadRuntimes - unknown error");
        last_error = XR_ERROR_FILE_ACCESS_ERROR;
    } else {
        for (std::unique_ptr<RuntimeManifestFile>& manifest_file : runtime_manifest_files) {
            LoaderPlatformLibraryHandle runtime_library = LoaderPlatformLibraryOpen(manifest_file->LibraryPath());
            if (nullptr == runtime_library) {
                if (!any_loaded) {
                    last_error = XR_ERROR_INSTANCE_LOST;
                }
                std::string library_message = LoaderPlatformLibraryOpenError(manifest_file->LibraryPath());
                std::string warning_message = "RuntimeInterface::LoadRuntime skipping manifest file ";
                warning_message += manifest_file->Filename();
                warning_message += ", failed to load with message \"";
                warning_message += library_message;
                warning_message += "\"";
                LoaderLogger::LogErrorMessage(openxr_command, warning_message);
                continue;
            }

            // Get and settle on an runtime interface version (using any provided name if required).
            std::string function_name = manifest_file->GetFunctionName("xrNegotiateLoaderRuntimeInterface");
            auto negotiate = reinterpret_cast<PFN_xrNegotiateLoaderRuntimeInterface>(
                LoaderPlatformLibraryGetProcAddr(runtime_library, function_name));

            // Loader info for negotiation
            XrNegotiateLoaderInfo loader_info = {};
            loader_info.structType = XR_LOADER_INTERFACE_STRUCT_LOADER_INFO;
            loader_info.structVersion = XR_LOADER_INFO_STRUCT_VERSION;
            loader_info.structSize = sizeof(XrNegotiateLoaderInfo);
            loader_info.minInterfaceVersion = 1;
            loader_info.maxInterfaceVersion = XR_CURRENT_LOADER_RUNTIME_VERSION;
            loader_info.minApiVersion = XR_MAKE_VERSION(1, 0, 0);
            loader_info.maxApiVersion = XR_MAKE_VERSION(1, 0x3ff, 0xfff);  // Maximum allowed version for this major version.

            // Set up the runtime return structure
            XrNegotiateRuntimeRequest runtime_info = {};
            runtime_info.structType = XR_LOADER_INTERFACE_STRUCT_RUNTIME_REQUEST;
            runtime_info.structVersion = XR_RUNTIME_INFO_STRUCT_VERSION;
            runtime_info.structSize = sizeof(XrNegotiateRuntimeRequest);

            // Skip calling the negotiate function and fail if the function pointer
            // could not get loaded
            XrResult res = XR_ERROR_RUNTIME_FAILURE;
            if (nullptr != negotiate) {
                res = negotiate(&loader_info, &runtime_info);
            }
            // If we supposedly succeeded, but got a nullptr for GetInstanceProcAddr
            // then something still went wrong, so return with an error.
            if (XR_SUCCEEDED(res)) {
                uint32_t runtime_major = XR_VERSION_MAJOR(runtime_info.runtimeApiVersion);
                uint32_t runtime_minor = XR_VERSION_MINOR(runtime_info.runtimeApiVersion);
                uint32_t loader_major = XR_VERSION_MAJOR(XR_CURRENT_API_VERSION);
                if (nullptr == runtime_info.getInstanceProcAddr) {
                    std::string error_message = "RuntimeInterface::LoadRuntime skipping manifest file ";
                    error_message += manifest_file->Filename();
                    error_message += ", negotiation succeeded but returned NULL getInstanceProcAddr";
                    LoaderLogger::LogErrorMessage(openxr_command, error_message);
                    res = XR_ERROR_FILE_CONTENTS_INVALID;
                } else if (0 >= runtime_info.runtimeInterfaceVersion ||
                           XR_CURRENT_LOADER_RUNTIME_VERSION < runtime_info.runtimeInterfaceVersion) {
                    std::string error_message = "RuntimeInterface::LoadRuntime skipping manifest file ";
                    error_message += manifest_file->Filename();
                    error_message += ", negotiation succeeded but returned invalid interface version";
                    LoaderLogger::LogErrorMessage(openxr_command, error_message);
                    res = XR_ERROR_FILE_CONTENTS_INVALID;
                } else if (runtime_major != loader_major || (runtime_major == 0 && runtime_minor == 0)) {
                    std::string error_message = "RuntimeInterface::LoadRuntime skipping manifest file ";
                    error_message += manifest_file->Filename();
                    error_message += ", OpenXR version returned not compatible with this loader";
                    LoaderLogger::LogErrorMessage(openxr_command, error_message);
                    res = XR_ERROR_FILE_CONTENTS_INVALID;
                }
            }
            if (XR_FAILED(res)) {
                if (!any_loaded) {
                    last_error = res;
                }
                std::string warning_message = "RuntimeInterface::LoadRuntime skipping manifest file ";
                warning_message += manifest_file->Filename();
                warning_message += ", negotiation failed with error ";
                warning_message += std::to_string(res);
                LoaderLogger::LogErrorMessage(openxr_command, warning_message);
                LoaderPlatformLibraryClose(runtime_library);
                continue;
            }

            std::string info_message = "RuntimeInterface::LoadRuntime succeeded loading runtime defined in manifest file ";
            info_message += manifest_file->Filename();
            info_message += " using interface version ";
            info_message += std::to_string(runtime_info.runtimeInterfaceVersion);
            info_message += " and OpenXR API version ";
            info_message += std::to_string(XR_VERSION_MAJOR(runtime_info.runtimeApiVersion));
            info_message += ".";
            info_message += std::to_string(XR_VERSION_MINOR(runtime_info.runtimeApiVersion));
            LoaderLogger::LogInfoMessage(openxr_command, info_message);

            // Use this runtime
            _single_runtime_interface.reset(new RuntimeInterface(runtime_library, runtime_info.getInstanceProcAddr));
            _single_runtime_count++;

            // Grab the list of extensions this runtime supports for easy filtering after the
            // xrCreateInstance call
            std::vector<std::string> supported_extensions;
            std::vector<XrExtensionProperties> extension_properties;
            _single_runtime_interface->GetInstanceExtensionProperties(extension_properties);
            supported_extensions.reserve(extension_properties.size());
            for (XrExtensionProperties ext_prop : extension_properties) {
                supported_extensions.emplace_back(ext_prop.extensionName);
            }
            _single_runtime_interface->SetSupportedExtensions(supported_extensions);

            // If we load one, clear all errors.
            any_loaded = true;
            last_error = XR_SUCCESS;
            break;
        }
    }

    // Always clear the manifest file list.  Either we use them or we don't.
    runtime_manifest_files.clear();

    // We found no valid runtimes, throw the initialization failed message
    if (!any_loaded) {
        LoaderLogger::LogErrorMessage(openxr_command, "RuntimeInterface::LoadRuntimes - failed to find a valid runtime");
        last_error = XR_ERROR_INSTANCE_LOST;
    }

    return last_error;
}

void RuntimeInterface::UnloadRuntime(const std::string& openxr_command) {
    if (_single_runtime_count == 1) {
        _single_runtime_count = 0;
        _single_runtime_interface.reset();
    } else if (_single_runtime_count > 0) {
        --_single_runtime_count;
    }
    LoaderLogger::LogInfoMessage(openxr_command, "RuntimeInterface being unloaded.");
}

XrResult RuntimeInterface::GetInstanceProcAddr(XrInstance instance, const char* name, PFN_xrVoidFunction* function) {
    return _single_runtime_interface->_get_instance_proc_addr(instance, name, function);
}

const XrGeneratedDispatchTable* RuntimeInterface::GetDispatchTable(XrInstance instance) {
    XrGeneratedDispatchTable* table = nullptr;
    std::lock_guard<std::mutex> mlock(_single_runtime_interface->_dispatch_table_mutex);
    auto it = _single_runtime_interface->_dispatch_table_map.find(instance);
    if (it != _single_runtime_interface->_dispatch_table_map.end()) {
        table = it->second.get();
    }
    return table;
}

const XrGeneratedDispatchTable* RuntimeInterface::GetDebugUtilsMessengerDispatchTable(XrDebugUtilsMessengerEXT messenger) {
    XrInstance runtime_instance = XR_NULL_HANDLE;
    {
        std::lock_guard<std::mutex> mlock(_single_runtime_interface->_messenger_to_instance_mutex);
        auto it = _single_runtime_interface->_messenger_to_instance_map.find(messenger);
        if (it != _single_runtime_interface->_messenger_to_instance_map.end()) {
            runtime_instance = it->second;
        }
    }
    return GetDispatchTable(runtime_instance);
}

RuntimeInterface::RuntimeInterface(LoaderPlatformLibraryHandle runtime_library, PFN_xrGetInstanceProcAddr get_instant_proc_addr)
    : _runtime_library(runtime_library), _get_instance_proc_addr(get_instant_proc_addr) {}

RuntimeInterface::~RuntimeInterface() {
    std::string info_message = "RuntimeInterface being destroyed.";
    LoaderLogger::LogInfoMessage("", info_message);
    {
        std::lock_guard<std::mutex> mlock(_dispatch_table_mutex);
        _dispatch_table_map.clear();
    }
    LoaderPlatformLibraryClose(_runtime_library);
}

void RuntimeInterface::GetInstanceExtensionProperties(std::vector<XrExtensionProperties>& extension_properties) {
    std::vector<XrExtensionProperties> runtime_extension_properties;
    PFN_xrEnumerateInstanceExtensionProperties rt_xrEnumerateInstanceExtensionProperties;
    _get_instance_proc_addr(XR_NULL_HANDLE, "xrEnumerateInstanceExtensionProperties",
                            reinterpret_cast<PFN_xrVoidFunction*>(&rt_xrEnumerateInstanceExtensionProperties));
    uint32_t count = 0;
    uint32_t count_output = 0;
    // Get the count from the runtime
    rt_xrEnumerateInstanceExtensionProperties(nullptr, count, &count_output, nullptr);
    if (count_output > 0) {
        runtime_extension_properties.resize(count_output);
        count = count_output;
        for (XrExtensionProperties& ext_prop : runtime_extension_properties) {
            ext_prop.type = XR_TYPE_EXTENSION_PROPERTIES;
            ext_prop.next = nullptr;
        }
        rt_xrEnumerateInstanceExtensionProperties(nullptr, count, &count_output, runtime_extension_properties.data());
    }
    size_t ext_count = runtime_extension_properties.size();
    size_t props_count = extension_properties.size();
    for (size_t ext = 0; ext < ext_count; ++ext) {
        bool found = false;
        for (size_t prop = 0; prop < props_count; ++prop) {
            // If we find it, then make sure the spec version matches that of the runtime instead of the
            // layer.
            if (strcmp(extension_properties[prop].extensionName, runtime_extension_properties[ext].extensionName) == 0) {
                // Make sure the spec version used is the runtime's
                extension_properties[prop].extensionVersion = runtime_extension_properties[ext].extensionVersion;
                found = true;
                break;
            }
        }
        if (!found) {
            extension_properties.push_back(runtime_extension_properties[ext]);
        }
    }
}

XrResult RuntimeInterface::CreateInstance(const XrInstanceCreateInfo* info, XrInstance* instance) {
    XrResult res = XR_SUCCESS;
    bool create_succeeded = false;
    PFN_xrCreateInstance rt_xrCreateInstance;
    _get_instance_proc_addr(XR_NULL_HANDLE, "xrCreateInstance", reinterpret_cast<PFN_xrVoidFunction*>(&rt_xrCreateInstance));
    res = rt_xrCreateInstance(info, instance);
    if (XR_SUCCEEDED(res)) {
        create_succeeded = true;
        std::unique_ptr<XrGeneratedDispatchTable> dispatch_table(new XrGeneratedDispatchTable());
        GeneratedXrPopulateDispatchTable(dispatch_table.get(), *instance, _get_instance_proc_addr);
        std::lock_guard<std::mutex> mlock(_dispatch_table_mutex);
        _dispatch_table_map[*instance] = std::move(dispatch_table);
    }

    // If the failure occurred during the populate, clean up the instance we had picked up from the runtime
    if (XR_FAILED(res) && create_succeeded) {
        PFN_xrDestroyInstance rt_xrDestroyInstance;
        _get_instance_proc_addr(*instance, "xrDestroyInstance", reinterpret_cast<PFN_xrVoidFunction*>(&rt_xrDestroyInstance));
        rt_xrDestroyInstance(*instance);
        *instance = XR_NULL_HANDLE;
    }

    return res;
}

XrResult RuntimeInterface::DestroyInstance(XrInstance instance) {
    if (XR_NULL_HANDLE != instance) {
        // Destroy the dispatch table for this instance first
        {
            std::lock_guard<std::mutex> mlock(_dispatch_table_mutex);
            auto map_iter = _dispatch_table_map.find(instance);
            if (map_iter != _dispatch_table_map.end()) {
                _dispatch_table_map.erase(map_iter);
            }
        }
        // Now delete the instance
        PFN_xrDestroyInstance rt_xrDestroyInstance;
        _get_instance_proc_addr(instance, "xrDestroyInstance", reinterpret_cast<PFN_xrVoidFunction*>(&rt_xrDestroyInstance));
        rt_xrDestroyInstance(instance);
    }
    return XR_SUCCESS;
}

bool RuntimeInterface::TrackDebugMessenger(XrInstance instance, XrDebugUtilsMessengerEXT messenger) {
    std::lock_guard<std::mutex> mlock(_messenger_to_instance_mutex);
    _messenger_to_instance_map[messenger] = instance;
    return true;
}

void RuntimeInterface::ForgetDebugMessenger(XrDebugUtilsMessengerEXT messenger) {
    if (XR_NULL_HANDLE != messenger) {
        std::lock_guard<std::mutex> mlock(_messenger_to_instance_mutex);
        _messenger_to_instance_map.erase(messenger);
    }
}

void RuntimeInterface::SetSupportedExtensions(std::vector<std::string>& supported_extensions) {
    _supported_extensions = supported_extensions;
}

bool RuntimeInterface::SupportsExtension(const std::string& extension_name) {
    bool found_prop = false;
    for (const std::string& supported_extension : _supported_extensions) {
        if (supported_extension == extension_name) {
            found_prop = true;
            break;
        }
    }
    return found_prop;
}
