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
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

#include "platform_utils.hpp"
#include "manifest_file.hpp"
#include "xr_generated_dispatch_table.h"
#include "api_layer_interface.hpp"
#include "loader_interfaces.h"
#include "loader_logger.hpp"

#define OPENXR_ENABLE_LAYERS_ENV_VAR "XR_ENABLE_API_LAYERS"

// Add any layers defined in the loader layer environment variable.
static void AddEnvironmentApiLayers(const std::string& openxr_command, std::vector<std::string>& enabled_layers) {
    try {
        char* layer_environment_variable = PlatformUtilsGetEnv(OPENXR_ENABLE_LAYERS_ENV_VAR);
        if (nullptr != layer_environment_variable) {
            std::string layers = layer_environment_variable;
            PlatformUtilsFreeEnv(layer_environment_variable);

            std::size_t last_found = 0;
            std::size_t found = layers.find_first_of(PATH_SEPARATOR);
            std::string cur_search;

            // Handle any path listings in the string (separated by the appropriate path separator)
            while (found != std::string::npos) {
                cur_search = layers.substr(last_found, found);
                enabled_layers.push_back(cur_search);
                last_found = found + 1;
                found = layers.find_first_of(PATH_SEPARATOR, last_found);
            }

            // If there's something remaining in the string, copy it over
            if (last_found < layers.size()) {
                cur_search = layers.substr(last_found);
                enabled_layers.push_back(cur_search);
            }
        }
    } catch (...) {
        LoaderLogger::LogErrorMessage(openxr_command, "AddEnvironmentApiLayers - unknown error occurred");
        throw;
    }
}

XrResult ApiLayerInterface::GetApiLayerProperties(const std::string& openxr_command, uint32_t incoming_count,
                                                  uint32_t* outgoing_count, XrApiLayerProperties* api_layer_properties) {
    try {
        std::vector<std::unique_ptr<ApiLayerManifestFile>> manifest_files;
        uint32_t manifest_count = 0;

        // Find any implicit layers which we may need to report information for.
        XrResult result = ApiLayerManifestFile::FindManifestFiles(MANIFEST_TYPE_IMPLICIT_API_LAYER, manifest_files);
        if (XR_SUCCESS == result) {
            // Find any explicit layers which we may need to report information for.
            result = ApiLayerManifestFile::FindManifestFiles(MANIFEST_TYPE_EXPLICIT_API_LAYER, manifest_files);
        }
        if (XR_SUCCESS != result) {
            LoaderLogger::LogErrorMessage(
                openxr_command, "ApiLayerInterface::GetApiLayerProperties - failed searching for API layer manifest files");
            return result;
        }

        manifest_count = static_cast<uint32_t>(manifest_files.size());
        if (0 == incoming_count) {
            *outgoing_count = manifest_count;
        } else if (nullptr != api_layer_properties) {
            if (incoming_count < manifest_count && nullptr != api_layer_properties) {
                LoaderLogger::LogErrorMessage(
                    "xrEnumerateInstanceExtensionProperties",
                    "VUID-xrEnumerateApiLayerProperties-propertyCapacityInput-parameter: insufficient space in array");
                *outgoing_count = manifest_count;
                return XR_ERROR_SIZE_INSUFFICIENT;
            }

            uint32_t prop = 0;
            bool properties_valid = true;
            for (; prop < incoming_count && prop < manifest_count; ++prop) {
                if (XR_TYPE_API_LAYER_PROPERTIES != api_layer_properties[prop].type) {
                    LoaderLogger::LogErrorMessage(openxr_command,
                                                  "VUID-XrApiLayerProperties-type-type: unknown type in api_layer_properties");
                    properties_valid = false;
                }
                if (nullptr != api_layer_properties[prop].next) {
                    LoaderLogger::LogErrorMessage(openxr_command, "VUID-XrApiLayerProperties-next-next: expected NULL");
                    properties_valid = false;
                }
                if (properties_valid) {
                    api_layer_properties[prop] = manifest_files[prop]->GetApiLayerProperties();
                }
            }
            if (!properties_valid) {
                LoaderLogger::LogErrorMessage(openxr_command,
                                              "VUID-xrEnumerateApiLayerProperties-properties-parameter: invalid properties");
                return XR_ERROR_VALIDATION_FAILURE;
            } else if (nullptr != outgoing_count) {
                *outgoing_count = prop;
            }
        }
        return XR_SUCCESS;
    } catch (...) {
        LoaderLogger::LogErrorMessage(openxr_command, "ApiLayerInterface::GetApiLayerProperties - unknown error occurred");
        return XR_ERROR_VALIDATION_FAILURE;
    }
}

XrResult ApiLayerInterface::GetInstanceExtensionProperties(const std::string& openxr_command, const char* layer_name,
                                                           std::vector<XrExtensionProperties>& extension_properties) {
    try {
        std::vector<std::unique_ptr<ApiLayerManifestFile>> manifest_files;

        // If a layer name is supplied, only use the information out of that one layer
        if (nullptr != layer_name && 0 != strlen(layer_name)) {
            XrResult result = ApiLayerManifestFile::FindManifestFiles(MANIFEST_TYPE_IMPLICIT_API_LAYER, manifest_files);
            if (XR_SUCCESS == result) {
                // Find any explicit layers which we may need to report information for.
                result = ApiLayerManifestFile::FindManifestFiles(MANIFEST_TYPE_EXPLICIT_API_LAYER, manifest_files);
                if (XR_SUCCESS != result) {
                    LoaderLogger::LogErrorMessage(
                        openxr_command,
                        "ApiLayerInterface::GetInstanceExtensionProperties - failed searching for API layer manifest files");
                    return result;
                }

                bool found = false;
                uint32_t num_files = static_cast<uint32_t>(manifest_files.size());
                for (uint32_t man_file = 0; man_file < num_files; ++man_file) {
                    // If a layer with the provided name exists, get it's instance extension information.
                    if (manifest_files[man_file]->LayerName() == layer_name) {
                        manifest_files[man_file]->GetInstanceExtensionProperties(extension_properties);
                        found = true;
                        break;
                    }
                }

                // If nothing found, report 0
                if (!found) {
                    return XR_ERROR_API_LAYER_NOT_PRESENT;
                }
            }
            // Otherwise, we want to add only implicit API layers and explicit API layers enabled using the environment variables
        } else {
            XrResult result = ApiLayerManifestFile::FindManifestFiles(MANIFEST_TYPE_IMPLICIT_API_LAYER, manifest_files);
            if (XR_SUCCESS == result) {
                // Find any environmentally enabled explicit layers.  If they're present, treat them like implicit layers
                // since we know that they're going to be enabled.
                std::vector<std::string> env_enabled_layers;
                AddEnvironmentApiLayers(openxr_command, env_enabled_layers);
                if (env_enabled_layers.size() > 0) {
                    std::vector<std::unique_ptr<ApiLayerManifestFile>> exp_layer_man_files = {};
                    result = ApiLayerManifestFile::FindManifestFiles(MANIFEST_TYPE_EXPLICIT_API_LAYER, exp_layer_man_files);
                    if (XR_SUCCESS == result) {
                        for (auto l_iter = exp_layer_man_files.begin();
                             exp_layer_man_files.size() > 0 && l_iter != exp_layer_man_files.end();
                             /* No iterate */) {
                            for (std::string& enabled_layer : env_enabled_layers) {
                                // If this is an enabled layer, transfer it over to the manifest list.
                                if (enabled_layer == (*l_iter)->LayerName()) {
                                    manifest_files.push_back(std::move(*l_iter));
                                    break;
                                }
                            }
                            exp_layer_man_files.erase(l_iter);
                        }
                    }
                }
            }

            // Grab the layer instance extensions information
            uint32_t num_files = static_cast<uint32_t>(manifest_files.size());
            for (uint32_t man_file = 0; man_file < num_files; ++man_file) {
                manifest_files[man_file]->GetInstanceExtensionProperties(extension_properties);
            }
        }
        return XR_SUCCESS;
    } catch (...) {
        LoaderLogger::LogErrorMessage(openxr_command, "ApiLayerInterface::GetInstanceExtensionProperties - unknown error occurred");
        return XR_ERROR_INITIALIZATION_FAILED;
    }
}

XrResult ApiLayerInterface::LoadApiLayers(const std::string& openxr_command, uint32_t enabled_api_layer_count,
                                          const char* const* enabled_api_layer_names,
                                          std::vector<std::unique_ptr<ApiLayerInterface>>& api_layer_interfaces) {
    XrResult last_error = XR_SUCCESS;
    try {
        bool any_loaded = false;
        std::vector<bool> layer_found;
        std::vector<std::unique_ptr<ApiLayerManifestFile>> layer_manifest_files = {};

        // Find any implicit layers which we may need to report information for.
        XrResult result = ApiLayerManifestFile::FindManifestFiles(MANIFEST_TYPE_IMPLICIT_API_LAYER, layer_manifest_files);
        if (XR_SUCCESS == result) {
            // Find any explicit layers which we may need to report information for.
            result = ApiLayerManifestFile::FindManifestFiles(MANIFEST_TYPE_EXPLICIT_API_LAYER, layer_manifest_files);
        }

        // Put all the enabled layers into a string vector
        std::vector<std::string> enabled_api_layers = {};
        AddEnvironmentApiLayers(openxr_command, enabled_api_layers);
        if (enabled_api_layer_count > 0) {
            if (nullptr == enabled_api_layer_names) {
                LoaderLogger::LogErrorMessage(
                    "xrCreateInstance",
                    "VUID-XrInstanceCreateInfo-enabledApiLayerNames-parameter: enabledApiLayerCount is non-0 but array is NULL");
                LoaderLogger::LogErrorMessage(
                    "xrCreateInstance", "VUID-xrCreateInstance-info-parameter: something wrong with XrInstanceCreateInfo contents");
                return XR_ERROR_VALIDATION_FAILURE;
            }
            uint32_t num_env_api_layers = static_cast<uint32_t>(enabled_api_layers.size());
            uint32_t total_api_layers = num_env_api_layers + enabled_api_layer_count;
            enabled_api_layers.resize(total_api_layers);
            for (uint32_t layer = 0; layer < enabled_api_layer_count; ++layer) {
                enabled_api_layers[num_env_api_layers + layer] = enabled_api_layer_names[layer];
            }
        }

        // Initialize the layer found vector to false
        layer_found.resize(enabled_api_layers.size());
        for (uint32_t layer = 0; layer < layer_found.size(); ++layer) {
            layer_found[layer] = false;
        }

        for (std::unique_ptr<ApiLayerManifestFile>& manifest_file : layer_manifest_files) {
            bool enabled = false;

            // Always add implicit layers.  They would only be in this list if they were enabled
            // (i.e. the disable environment variable is not set).
            if (manifest_file->Type() == MANIFEST_TYPE_IMPLICIT_API_LAYER) {
                enabled = true;
            } else {
                // Only add explicit layers if they are called out by the application
                for (uint32_t layer = 0; layer < enabled_api_layers.size(); ++layer) {
                    if (enabled_api_layers[layer] == manifest_file->LayerName()) {
                        layer_found[layer] = true;
                        enabled = true;
                        break;
                    }
                }
            }

            // If this layer isn't enabled, skip it.
            if (!enabled) {
                continue;
            }

            LoaderPlatformLibraryHandle layer_library = LoaderPlatformLibraryOpen(manifest_file->LibraryPath());
            if (nullptr == layer_library) {
                if (!any_loaded) {
                    last_error = XR_ERROR_FILE_ACCESS_ERROR;
                }
                std::string library_message = LoaderPlatformLibraryOpenError(manifest_file->LibraryPath());
                std::string warning_message = "ApiLayerInterface::LoadApiLayers skipping layer ";
                warning_message += manifest_file->LayerName();
                warning_message += ", failed to load with message \"";
                warning_message += library_message;
                warning_message += "\"";
                LoaderLogger::LogWarningMessage(openxr_command, warning_message);
                continue;
            }

            // Get and settle on an layer interface version (using any provided name if required).
            std::string function_name = manifest_file->GetFunctionName("xrNegotiateLoaderApiLayerInterface");
            PFN_xrNegotiateLoaderApiLayerInterface negotiate = reinterpret_cast<PFN_xrNegotiateLoaderApiLayerInterface>(
                LoaderPlatformLibraryGetProcAddr(layer_library, function_name));

            // Loader info for negotiation
            XrNegotiateLoaderInfo loader_info = {};
            loader_info.structType = XR_LOADER_INTERFACE_STRUCT_LOADER_INFO;
            loader_info.structVersion = XR_LOADER_INFO_STRUCT_VERSION;
            loader_info.structSize = sizeof(XrNegotiateLoaderInfo);
            loader_info.minInterfaceVersion = 1;
            loader_info.maxInterfaceVersion = XR_CURRENT_LOADER_API_LAYER_VERSION;
            loader_info.minXrVersion = XR_MAKE_VERSION(0, 1, 0);
            loader_info.maxXrVersion = XR_MAKE_VERSION(1, 0, 0);

            // Set up the layer return structure
            XrNegotiateApiLayerRequest api_layer_info = {};
            api_layer_info.structType = XR_LOADER_INTERFACE_STRUCT_API_LAYER_REQUEST;
            api_layer_info.structVersion = XR_API_LAYER_INFO_STRUCT_VERSION;
            api_layer_info.structSize = sizeof(XrNegotiateApiLayerRequest);

            XrResult res = negotiate(&loader_info, manifest_file->LayerName().c_str(), &api_layer_info);
            // If we supposedly succeeded, but got a nullptr for getInstanceProcAddr
            // then something still went wrong, so return with an error.
            if (XR_SUCCESS == res && nullptr == api_layer_info.getInstanceProcAddr) {
                std::string warning_message = "ApiLayerInterface::LoadApiLayers skipping layer ";
                warning_message += manifest_file->LayerName();
                warning_message += ", negotiation did not return a valid getInstanceProcAddr";
                LoaderLogger::LogWarningMessage(openxr_command, warning_message);
                res = XR_ERROR_FILE_CONTENTS_INVALID;
            }
            if (XR_SUCCESS != res) {
                if (!any_loaded) {
                    last_error = res;
                }
                std::string warning_message = "ApiLayerInterface::LoadApiLayers skipping layer ";
                warning_message += manifest_file->LayerName();
                warning_message += " due to failed negotiation with error ";
                warning_message += std::to_string(res);
                LoaderLogger::LogWarningMessage(openxr_command, warning_message);
                LoaderPlatformLibraryClose(layer_library);
                continue;
            }

            std::string info_message = "ApiLayerInterface::LoadApiLayers succeeded loading layer ";
            info_message += manifest_file->LayerName();
            info_message += " using interface version ";
            info_message += std::to_string(api_layer_info.layerInterfaceVersion);
            info_message += " and OpenXR API version ";
            info_message += std::to_string(XR_VERSION_MAJOR(api_layer_info.layerXrVersion));
            info_message += ".";
            info_message += std::to_string(XR_VERSION_MINOR(api_layer_info.layerXrVersion));
            LoaderLogger::LogInfoMessage(openxr_command, info_message);

            // Grab the list of extensions this layer supports for easy filtering after the
            // xrCreateInstance call
            std::vector<std::string> supported_extensions;
            std::vector<XrExtensionProperties> extension_properties;
            manifest_file->GetInstanceExtensionProperties(extension_properties);
            for (XrExtensionProperties& ext_prop : extension_properties) {
                supported_extensions.push_back(ext_prop.extensionName);
            }

            // Add this runtime to the vector
            api_layer_interfaces.emplace_back(new ApiLayerInterface(manifest_file->LayerName(), layer_library, supported_extensions,
                                                                    api_layer_info.getInstanceProcAddr,
                                                                    api_layer_info.createApiLayerInstance));

            // If we load one, clear all errors.
            any_loaded = true;
            last_error = XR_SUCCESS;
        }

        // If even one of the layers wasn't found, we want to return an error
        for (uint32_t layer = 0; layer < layer_found.size(); ++layer) {
            if (!layer_found[layer]) {
                std::string error_message = "ApiLayerInterface::LoadApiLayers - failed to find layer ";
                error_message += enabled_api_layers[layer];
                LoaderLogger::LogErrorMessage(openxr_command, error_message);
                last_error = XR_ERROR_API_LAYER_NOT_PRESENT;
            }
        }

        // Always clear the manifest file list.  Either we use them or we don't.
        layer_manifest_files.clear();
    } catch (std::bad_alloc&) {
        LoaderLogger::LogErrorMessage(openxr_command, "ApiLayerInterface::LoadApiLayers - failed to allocate memory");
        last_error = XR_ERROR_OUT_OF_MEMORY;
    } catch (...) {
        LoaderLogger::LogErrorMessage(openxr_command, "ApiLayerInterface::LoadApiLayers - unknown error");
        last_error = XR_ERROR_FILE_ACCESS_ERROR;
    }

    // If we failed catastrophically for some reason, clean up everything.
    if (XR_SUCCESS != last_error) {
        api_layer_interfaces.clear();
    }

    return last_error;
}

ApiLayerInterface::ApiLayerInterface(std::string layer_name, LoaderPlatformLibraryHandle layer_library,
                                     std::vector<std::string>& supported_extensions,
                                     PFN_xrGetInstanceProcAddr get_instant_proc_addr,
                                     PFN_xrCreateApiLayerInstance create_api_layer_instance)
    : _layer_name(layer_name),
      _layer_library(layer_library),
      _get_instant_proc_addr(get_instant_proc_addr),
      _create_api_layer_instance(create_api_layer_instance),
      _supported_extensions(supported_extensions) {}

ApiLayerInterface::~ApiLayerInterface() {
    std::string info_message = "ApiLayerInterface being destroyed for layer ";
    info_message += _layer_name;
    LoaderLogger::LogInfoMessage("", info_message);
    LoaderPlatformLibraryClose(_layer_library);
}

bool ApiLayerInterface::SupportsExtension(const std::string& extension_name) {
    bool found_prop = false;
    try {
        for (std::string supported_extension : _supported_extensions) {
            if (supported_extension == extension_name) {
                found_prop = true;
                break;
            }
        }
    } catch (...) {
    }
    return found_prop;
}
