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

#include "api_layer_interface.hpp"

#include "loader_interfaces.h"
#include "loader_logger.hpp"
#include "loader_platform.hpp"
#include "manifest_file.hpp"
#include "platform_utils.hpp"

#include <openxr/openxr.h>

#include <cstring>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#define OPENXR_ENABLE_LAYERS_ENV_VAR "XR_ENABLE_API_LAYERS"

// Add any layers defined in the loader layer environment variable.
static void AddEnvironmentApiLayers(std::vector<std::string>& enabled_layers) {
    std::string layers = PlatformUtilsGetEnv(OPENXR_ENABLE_LAYERS_ENV_VAR);

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

XrResult ApiLayerInterface::GetApiLayerProperties(const std::string& openxr_command, uint32_t incoming_count,
                                                  uint32_t* outgoing_count, XrApiLayerProperties* api_layer_properties) {
    std::vector<std::unique_ptr<ApiLayerManifestFile>> manifest_files;
    uint32_t manifest_count = 0;

    // Validate props struct before proceeding
    if (0 < incoming_count && nullptr != api_layer_properties) {
        for (uint32_t i = 0; i < incoming_count; i++) {
            if (XR_TYPE_API_LAYER_PROPERTIES != api_layer_properties[i].type) {
                LoaderLogger::LogErrorMessage(openxr_command,
                                              "VUID-XrApiLayerProperties-type-type: unknown type in api_layer_properties");
                return XR_ERROR_VALIDATION_FAILURE;
            }
        }
    }

    // "Independent of elementCapacityInput or elements parameters, elementCountOutput must be a valid pointer,
    // and the function sets elementCountOutput." - 2.11
    if (nullptr == outgoing_count) {
        return XR_ERROR_VALIDATION_FAILURE;
    }

    // Find any implicit layers which we may need to report information for.
    XrResult result = ApiLayerManifestFile::FindManifestFiles(MANIFEST_TYPE_IMPLICIT_API_LAYER, manifest_files);
    if (XR_SUCCEEDED(result)) {
        // Find any explicit layers which we may need to report information for.
        result = ApiLayerManifestFile::FindManifestFiles(MANIFEST_TYPE_EXPLICIT_API_LAYER, manifest_files);
    }
    if (XR_FAILED(result)) {
        LoaderLogger::LogErrorMessage(openxr_command,
                                      "ApiLayerInterface::GetApiLayerProperties - failed searching for API layer manifest files");
        return result;
    }

    manifest_count = static_cast<uint32_t>(manifest_files.size());
    if (nullptr == outgoing_count) {
        LoaderLogger::LogErrorMessage("xrEnumerateInstanceExtensionProperties",
                                      "VUID-xrEnumerateApiLayerProperties-propertyCountOutput-parameter: null propertyCountOutput");
        return XR_ERROR_VALIDATION_FAILURE;
    }

    *outgoing_count = manifest_count;
    if (0 == incoming_count) {
        // capacity check only
        return XR_SUCCESS;
    }
    if (nullptr == api_layer_properties) {
        // incoming_count is not 0 BUT the api_layer_properties is NULL
        LoaderLogger::LogErrorMessage("xrEnumerateInstanceExtensionProperties",
                                      "VUID-xrEnumerateApiLayerProperties-properties-parameter: non-zero capacity but null array");
        return XR_ERROR_VALIDATION_FAILURE;
    }
    if (incoming_count < manifest_count) {
        LoaderLogger::LogErrorMessage(
            "xrEnumerateInstanceExtensionProperties",
            "VUID-xrEnumerateApiLayerProperties-propertyCapacityInput-parameter: insufficient space in array");
        return XR_ERROR_SIZE_INSUFFICIENT;
    }

    for (uint32_t prop = 0; prop < incoming_count && prop < manifest_count; ++prop) {
        manifest_files[prop]->PopulateApiLayerProperties(api_layer_properties[prop]);
    }
    return XR_SUCCESS;
}

XrResult ApiLayerInterface::GetInstanceExtensionProperties(const std::string& openxr_command, const char* layer_name,
                                                           std::vector<XrExtensionProperties>& extension_properties) {
    std::vector<std::unique_ptr<ApiLayerManifestFile>> manifest_files;

    // If a layer name is supplied, only use the information out of that one layer
    if (nullptr != layer_name && 0 != strlen(layer_name)) {
        XrResult result = ApiLayerManifestFile::FindManifestFiles(MANIFEST_TYPE_IMPLICIT_API_LAYER, manifest_files);
        if (XR_SUCCEEDED(result)) {
            // Find any explicit layers which we may need to report information for.
            result = ApiLayerManifestFile::FindManifestFiles(MANIFEST_TYPE_EXPLICIT_API_LAYER, manifest_files);
            if (XR_FAILED(result)) {
                LoaderLogger::LogErrorMessage(
                    openxr_command,
                    "ApiLayerInterface::GetInstanceExtensionProperties - failed searching for API layer manifest files");
                return result;
            }

            bool found = false;
            auto num_files = static_cast<uint32_t>(manifest_files.size());
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
        if (XR_SUCCEEDED(result)) {
            // Find any environmentally enabled explicit layers.  If they're present, treat them like implicit layers
            // since we know that they're going to be enabled.
            std::vector<std::string> env_enabled_layers;
            AddEnvironmentApiLayers(env_enabled_layers);
            if (!env_enabled_layers.empty()) {
                std::vector<std::unique_ptr<ApiLayerManifestFile>> exp_layer_man_files = {};
                result = ApiLayerManifestFile::FindManifestFiles(MANIFEST_TYPE_EXPLICIT_API_LAYER, exp_layer_man_files);
                if (XR_SUCCEEDED(result)) {
                    for (auto& exp_layer_man_file : exp_layer_man_files) {
                        for (std::string& enabled_layer : env_enabled_layers) {
                            // If this is an enabled layer, transfer it over to the manifest list.
                            if (enabled_layer == exp_layer_man_file->LayerName()) {
                                manifest_files.push_back(std::move(exp_layer_man_file));
                                break;
                            }
                        }
                    }
                }
            }
        }

        // Grab the layer instance extensions information
        auto num_files = static_cast<uint32_t>(manifest_files.size());
        for (uint32_t man_file = 0; man_file < num_files; ++man_file) {
            manifest_files[man_file]->GetInstanceExtensionProperties(extension_properties);
        }
    }
    return XR_SUCCESS;
}

XrResult ApiLayerInterface::LoadApiLayers(const std::string& openxr_command, uint32_t enabled_api_layer_count,
                                          const char* const* enabled_api_layer_names,
                                          std::vector<std::unique_ptr<ApiLayerInterface>>& api_layer_interfaces) {
    XrResult last_error = XR_SUCCESS;
    bool any_loaded = false;
    std::vector<bool> layer_found;
    std::vector<std::unique_ptr<ApiLayerManifestFile>> layer_manifest_files = {};

    // Find any implicit layers which we may need to report information for.
    XrResult result = ApiLayerManifestFile::FindManifestFiles(MANIFEST_TYPE_IMPLICIT_API_LAYER, layer_manifest_files);
    if (XR_SUCCEEDED(result)) {
        // Find any explicit layers which we may need to report information for.
        result = ApiLayerManifestFile::FindManifestFiles(MANIFEST_TYPE_EXPLICIT_API_LAYER, layer_manifest_files);
    }

    // Put all the enabled layers into a string vector
    std::vector<std::string> enabled_api_layers = {};
    AddEnvironmentApiLayers(enabled_api_layers);
    if (enabled_api_layer_count > 0) {
        if (nullptr == enabled_api_layer_names) {
            LoaderLogger::LogErrorMessage(
                "xrCreateInstance",
                "VUID-XrInstanceCreateInfo-enabledApiLayerNames-parameter: enabledApiLayerCount is non-0 but array is NULL");
            LoaderLogger::LogErrorMessage(
                "xrCreateInstance", "VUID-xrCreateInstance-info-parameter: something wrong with XrInstanceCreateInfo contents");
            return XR_ERROR_VALIDATION_FAILURE;
        }
        auto num_env_api_layers = static_cast<uint32_t>(enabled_api_layers.size());
        uint32_t total_api_layers = num_env_api_layers + enabled_api_layer_count;
        enabled_api_layers.resize(total_api_layers);
        for (uint32_t layer = 0; layer < enabled_api_layer_count; ++layer) {
            enabled_api_layers[num_env_api_layers + layer] = enabled_api_layer_names[layer];
        }
    }

    // Initialize the layer found vector to false
    layer_found.resize(enabled_api_layers.size());
    for (auto&& layer : layer_found) {
        layer = false;
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
        auto negotiate = reinterpret_cast<PFN_xrNegotiateLoaderApiLayerInterface>(
            LoaderPlatformLibraryGetProcAddr(layer_library, function_name));

        if (nullptr == negotiate) {
            std::ostringstream oss;
            oss << "ApiLayerInterface::LoadApiLayers skipping layer " << manifest_file->LayerName()
                << " because negotiation function " << function_name << " was not found";
            LoaderLogger::LogErrorMessage(openxr_command, oss.str());
            LoaderPlatformLibraryClose(layer_library);
            last_error = XR_ERROR_API_LAYER_NOT_PRESENT;
            continue;
        }

        // Loader info for negotiation
        XrNegotiateLoaderInfo loader_info = {};
        loader_info.structType = XR_LOADER_INTERFACE_STRUCT_LOADER_INFO;
        loader_info.structVersion = XR_LOADER_INFO_STRUCT_VERSION;
        loader_info.structSize = sizeof(XrNegotiateLoaderInfo);
        loader_info.minInterfaceVersion = 1;
        loader_info.maxInterfaceVersion = XR_CURRENT_LOADER_API_LAYER_VERSION;
        loader_info.minApiVersion = XR_MAKE_VERSION(1, 0, 0);
        loader_info.maxApiVersion = XR_MAKE_VERSION(1, 0x3ff, 0xfff);  // Maximum allowed version for this major version.

        // Set up the layer return structure
        XrNegotiateApiLayerRequest api_layer_info = {};
        api_layer_info.structType = XR_LOADER_INTERFACE_STRUCT_API_LAYER_REQUEST;
        api_layer_info.structVersion = XR_API_LAYER_INFO_STRUCT_VERSION;
        api_layer_info.structSize = sizeof(XrNegotiateApiLayerRequest);

        XrResult res = negotiate(&loader_info, manifest_file->LayerName().c_str(), &api_layer_info);
        // If we supposedly succeeded, but got a nullptr for getInstanceProcAddr
        // then something still went wrong, so return with an error.
        if (XR_SUCCEEDED(res) && nullptr == api_layer_info.getInstanceProcAddr) {
            std::string warning_message = "ApiLayerInterface::LoadApiLayers skipping layer ";
            warning_message += manifest_file->LayerName();
            warning_message += ", negotiation did not return a valid getInstanceProcAddr";
            LoaderLogger::LogWarningMessage(openxr_command, warning_message);
            res = XR_ERROR_FILE_CONTENTS_INVALID;
        }
        if (XR_FAILED(res)) {
            if (!any_loaded) {
                last_error = res;
            }
            std::ostringstream oss;
            oss << "ApiLayerInterface::LoadApiLayers skipping layer " << manifest_file->LayerName()
                << " due to failed negotiation with error " << res;
            LoaderLogger::LogWarningMessage(openxr_command, oss.str());
            LoaderPlatformLibraryClose(layer_library);
            continue;
        }

        {
            std::ostringstream oss;
            oss << "ApiLayerInterface::LoadApiLayers succeeded loading layer " << manifest_file->LayerName()
                << " using interface version " << api_layer_info.layerInterfaceVersion << " and OpenXR API version "
                << XR_VERSION_MAJOR(api_layer_info.layerApiVersion) << "." << XR_VERSION_MINOR(api_layer_info.layerApiVersion);
            LoaderLogger::LogInfoMessage(openxr_command, oss.str());
        }

        // Grab the list of extensions this layer supports for easy filtering after the
        // xrCreateInstance call
        std::vector<std::string> supported_extensions;
        std::vector<XrExtensionProperties> extension_properties;
        manifest_file->GetInstanceExtensionProperties(extension_properties);
        supported_extensions.reserve(extension_properties.size());
        for (XrExtensionProperties& ext_prop : extension_properties) {
            supported_extensions.emplace_back(ext_prop.extensionName);
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

    // If we failed catastrophically for some reason, clean up everything.
    if (XR_FAILED(last_error)) {
        api_layer_interfaces.clear();
    }

    return last_error;
}

ApiLayerInterface::ApiLayerInterface(const std::string& layer_name, LoaderPlatformLibraryHandle layer_library,
                                     std::vector<std::string>& supported_extensions,
                                     PFN_xrGetInstanceProcAddr get_instance_proc_addr,
                                     PFN_xrCreateApiLayerInstance create_api_layer_instance)
    : _layer_name(layer_name),
      _layer_library(layer_library),
      _get_instance_proc_addr(get_instance_proc_addr),
      _create_api_layer_instance(create_api_layer_instance),
      _supported_extensions(supported_extensions) {}

ApiLayerInterface::~ApiLayerInterface() {
    std::string info_message = "ApiLayerInterface being destroyed for layer ";
    info_message += _layer_name;
    LoaderLogger::LogInfoMessage("", info_message);
    LoaderPlatformLibraryClose(_layer_library);
}

bool ApiLayerInterface::SupportsExtension(const std::string& extension_name) const {
    bool found_prop = false;
    for (const std::string& supported_extension : _supported_extensions) {
        if (supported_extension == extension_name) {
            found_prop = true;
            break;
        }
    }
    return found_prop;
}
