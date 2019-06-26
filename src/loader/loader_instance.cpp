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
#include <memory>
#include <sstream>
#include <stack>

#include "xr_dependencies.h"
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include "loader_instance.hpp"
#include "xr_generated_dispatch_table.h"
#include "xr_generated_loader.hpp"
#include "loader_logger.hpp"

// Extensions that are supported by the loader, but may not be supported
// the the runtime.
static const XrExtensionProperties g_debug_utils_props = {XR_TYPE_EXTENSION_PROPERTIES, nullptr, XR_EXT_DEBUG_UTILS_EXTENSION_NAME,
                                                          XR_EXT_debug_utils_SPEC_VERSION};
const std::vector<XrExtensionProperties> LoaderInstance::_loader_supported_extensions = {g_debug_utils_props};

// Factory method
XrResult LoaderInstance::CreateInstance(std::vector<std::unique_ptr<ApiLayerInterface>>&& api_layer_interfaces,
                                        const XrInstanceCreateInfo* info, XrInstance* instance) {
    XrResult last_error = XR_SUCCESS;
    try {
        LoaderLogger::LogVerboseMessage("xrCreateInstance", "Entering LoaderInstance::CreateInstance");

        // Topmost means "closest to the application"
        PFN_xrCreateInstance topmost_ci_fp = LoaderXrTermCreateInstance;
        PFN_xrCreateApiLayerInstance topmost_cali_fp = LoaderXrTermCreateApiLayerInstance;

        // Create the loader instance
        std::unique_ptr<LoaderInstance> loader_instance(new LoaderInstance(std::move(api_layer_interfaces)));
        *instance = reinterpret_cast<XrInstance>(loader_instance.get());

        // Only start the xrCreateApiLayerInstance stack if we have layers.
        std::vector<std::unique_ptr<ApiLayerInterface>>& layer_interfaces = loader_instance->LayerInterfaces();
        if (layer_interfaces.size() > 0) {
            // Initialize an array of ApiLayerNextInfo structs
            XrApiLayerNextInfo* next_info_list = new XrApiLayerNextInfo[layer_interfaces.size()];
            uint32_t ni_index = static_cast<uint32_t>(layer_interfaces.size() - 1);
            for (uint32_t i = 0; i <= ni_index; i++) {
                next_info_list[i].structType = XR_LOADER_INTERFACE_STRUCT_API_LAYER_NEXT_INFO;
                next_info_list[i].structVersion = XR_API_LAYER_NEXT_INFO_STRUCT_VERSION;
                next_info_list[i].structSize = sizeof(XrApiLayerNextInfo);
            }

            // Go through all layers, and override the instance pointers with the layer version.  However,
            // go backwards through the layer list so we replace in reverse order so the layers can call their next function
            // appropriately.
            XrApiLayerNextInfo* prev_nextinfo = nullptr;
            PFN_xrGetInstanceProcAddr prev_gipa_fp = LoaderXrTermGetInstanceProcAddr;
            PFN_xrCreateApiLayerInstance prev_cali_fp = LoaderXrTermCreateApiLayerInstance;
            for (auto layer_interface = layer_interfaces.rbegin(); layer_interface != layer_interfaces.rend(); ++layer_interface) {
                // Collect current layer's function pointers
                PFN_xrGetInstanceProcAddr cur_gipa_fp = (*layer_interface)->GetInstanceProcAddrFuncPointer();
                PFN_xrCreateApiLayerInstance cur_cali_fp = (*layer_interface)->GetCreateApiLayerInstanceFuncPointer();
                // Update topmosts
                cur_gipa_fp(XR_NULL_HANDLE, "xrCreateInstance", reinterpret_cast<PFN_xrVoidFunction*>(&topmost_ci_fp));
                topmost_cali_fp = cur_cali_fp;

                // Fill in layer info and link previous (lower) layer fxn pointers
                strncpy(next_info_list[ni_index].layerName, (*layer_interface)->LayerName().c_str(),
                        XR_MAX_API_LAYER_NAME_SIZE - 1);
                next_info_list[ni_index].layerName[XR_MAX_API_LAYER_NAME_SIZE - 1] = '\0';
                next_info_list[ni_index].next = prev_nextinfo;
                next_info_list[ni_index].nextGetInstanceProcAddr = prev_gipa_fp;
                next_info_list[ni_index].nextCreateApiLayerInstance = prev_cali_fp;

                // Update saved pointers for next iteration
                prev_nextinfo = &next_info_list[ni_index];
                prev_gipa_fp = cur_gipa_fp;
                prev_cali_fp = cur_cali_fp;
                ni_index--;
            }

            // Populate the ApiLayerCreateInfo struct and pass to topmost CreateApiLayerInstance()
            XrApiLayerCreateInfo api_layer_ci = {};
            api_layer_ci.structType = XR_LOADER_INTERFACE_STRUCT_API_LAYER_CREATE_INFO;
            api_layer_ci.structVersion = XR_API_LAYER_CREATE_INFO_STRUCT_VERSION;
            api_layer_ci.structSize = sizeof(XrApiLayerCreateInfo);
            api_layer_ci.loaderInstance = reinterpret_cast<void*>(loader_instance.get());
            api_layer_ci.settings_file_location[0] = '\0';
            api_layer_ci.nextInfo = next_info_list;
            last_error = topmost_cali_fp(info, &api_layer_ci, instance);

            delete[] next_info_list;
        } else {
            last_error = topmost_ci_fp(info, instance);
        }

        if (XR_SUCCEEDED(last_error)) {
            // Check the list of enabled extensions to make sure something supports them, and, if we do,
            // add it to the list of enabled extensions
            for (uint32_t ext = 0; ext < info->enabledExtensionCount; ++ext) {
                bool found = false;
                // First check the runtime
                if (RuntimeInterface::GetRuntime().SupportsExtension(info->enabledExtensionNames[ext])) {
                    found = true;
                }
                // Next check the loader
                if (!found) {
                    for (auto loader_extension : LoaderInstance::_loader_supported_extensions) {
                        if (!strcmp(loader_extension.extensionName, info->enabledExtensionNames[ext])) {
                            found = true;
                            break;
                        }
                    }
                }
                // Finally, check the enabled layers
                if (!found) {
                    for (auto layer_interface = layer_interfaces.begin(); layer_interface != layer_interfaces.end();
                         ++layer_interface) {
                        if ((*layer_interface)->SupportsExtension(info->enabledExtensionNames[ext])) {
                            found = true;
                            break;
                        }
                    }
                }
                if (!found) {
                    std::string msg = "LoaderInstance::CreateInstance, no support found for requested extension: ";
                    msg += info->enabledExtensionNames[ext];
                    LoaderLogger::LogErrorMessage("xrCreateInstance", msg);
                    last_error = XR_ERROR_EXTENSION_NOT_PRESENT;
                    break;
                }
                loader_instance->AddEnabledExtension(info->enabledExtensionNames[ext]);
            }
        } else {
            LoaderLogger::LogErrorMessage("xrCreateInstance", "LoaderInstance::CreateInstance chained CreateInstance call failed");
        }

        if (XR_SUCCEEDED(last_error)) {
            // Create the top-level dispatch table for the instance.  This will contain the function pointers to the
            // first instantiation of every command, whether that is in a layer, or a runtime.
            last_error = loader_instance->CreateDispatchTable(*instance);
            if (XR_FAILED(last_error)) {
                LoaderLogger::LogErrorMessage("xrCreateInstance",
                                              "LoaderInstance::CreateInstance failed creating top-level dispatch table");
            } else {
                last_error = g_instance_map.Insert(*instance, *loader_instance);
                if (XR_FAILED(last_error)) {
                    LoaderLogger::LogErrorMessage(
                        "xrCreateInstance",
                        "LoaderInstance::CreateInstance failed inserting new instance into map: may be null or not unique");
                }
            }
        }

        if (XR_SUCCEEDED(last_error)) {
            std::ostringstream oss;
            oss << "LoaderInstance::CreateInstance succeeded with ";
            oss << loader_instance->LayerInterfaces().size();
            oss << " layers enabled and runtime interface - created instance = ";
            oss << HandleToHexString(*instance);
            LoaderLogger::LogInfoMessage("xrCreateInstance", oss.str());
            // Make the unique_ptr no longer delete this.
            loader_instance.release();
        }
    } catch (std::bad_alloc&) {
        LoaderLogger::LogErrorMessage("xrCreateInstance", "LoaderInstance::CreateInstance - failed to allocate memory");
        last_error = XR_ERROR_OUT_OF_MEMORY;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrCreateInstance", "LoaderInstance::CreateInstance - unknown error occurred");
        last_error = XR_ERROR_INITIALIZATION_FAILED;
    }

    // Always clear the input lists.  Either we use them or we don't.
    api_layer_interfaces.clear();

    return last_error;
}

LoaderInstance::LoaderInstance(std::vector<std::unique_ptr<ApiLayerInterface>>&& api_layer_interfaces) try
    : _unique_id(0xDECAFBAD),
      _api_version(XR_CURRENT_API_VERSION),
      _api_layer_interfaces(std::move(api_layer_interfaces)),
      _dispatch_valid(false),
      _messenger(XR_NULL_HANDLE) {
} catch (...) {
    LoaderLogger::LogErrorMessage("xrCreateInstance", "LoaderInstance::LoaderInstance - Unknown error occurred");
    throw;
}

LoaderInstance::~LoaderInstance() {
    std::ostringstream oss;
    oss << "Destroying LoaderInstance = ";
    oss << PointerToHexString(this);
    LoaderLogger::LogInfoMessage("xrDestroyInstance", oss.str());
}

XrResult LoaderInstance::CreateDispatchTable(XrInstance instance) {
    XrResult res = XR_SUCCESS;
    try {
        // Create the top-level dispatch table.  First, we want to start with a dispatch table generated
        // using the commands from the runtime, with the exception of commands that we need a terminator
        // for.  The loaderGenInitInstanceDispatchTable utility function handles that automatically for us.
        std::unique_ptr<XrGeneratedDispatchTable> new_instance_dispatch_table(new XrGeneratedDispatchTable());
        LoaderGenInitInstanceDispatchTable(_runtime_instance, new_instance_dispatch_table);

        // Go through all layers, and override the instance pointers with the layer version.  However,
        // go backwards through the layer list so we replace in reverse order so the layers can call their next function
        // appropriately.
        if (_api_layer_interfaces.size() > 0) {
            (*_api_layer_interfaces.begin())->GenUpdateInstanceDispatchTable(instance, new_instance_dispatch_table);
        }

        // Set the top-level instance dispatch table to the top-most commands now that we've figured them out.
        _dispatch_table = std::move(new_instance_dispatch_table);
        _dispatch_valid = true;
    } catch (std::bad_alloc&) {
        LoaderLogger::LogErrorMessage("xrCreateInstance", "LoaderInstance::CreateDispatchTable - failed to allocate memory");
        res = XR_ERROR_OUT_OF_MEMORY;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrCreateInstance", "LoaderInstance::CreateDispatchTable - unknown error occurred");
        res = XR_ERROR_INITIALIZATION_FAILED;
    }
    return res;
}

bool LoaderInstance::ExtensionIsEnabled(const std::string& extension) {
    for (std::string& cur_enabled : _enabled_extensions) {
        if (cur_enabled == extension) {
            return true;
        }
    }
    return false;
}
