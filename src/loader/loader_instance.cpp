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

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif  // defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)

#include "loader_instance.hpp"

#include "api_layer_interface.hpp"
#include "hex_and_handles.h"
#include "loader_interfaces.h"
#include "loader_logger.hpp"
#include "runtime_interface.hpp"
#include "xr_generated_dispatch_table.h"
#include "xr_generated_loader.hpp"

#include <openxr/openxr.h>

#include <cstring>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// Extensions that are supported by the loader, but may not be supported
// the the runtime.
const std::array<XrExtensionProperties, 1>& LoaderInstance::LoaderSpecificExtensions() {
    static const std::array<XrExtensionProperties, 1> extensions = {XrExtensionProperties{
        XR_TYPE_EXTENSION_PROPERTIES, nullptr, XR_EXT_DEBUG_UTILS_EXTENSION_NAME, XR_EXT_debug_utils_SPEC_VERSION}};
    return extensions;
}

namespace {
class InstanceCreateInfoManager {
   public:
    explicit InstanceCreateInfoManager(const XrInstanceCreateInfo* info) : original_create_info(info), modified_create_info(*info) {
        Reset();
    }

    // Reset the "modified" state to match the original state.
    void Reset() {
        enabled_extensions_cstr.clear();
        enabled_extensions_cstr.reserve(original_create_info->enabledExtensionCount);

        for (uint32_t i = 0; i < original_create_info->enabledExtensionCount; ++i) {
            enabled_extensions_cstr.push_back(original_create_info->enabledExtensionNames[i]);
        }
        Update();
    }

    // Remove extensions named in the parameter and return a pointer to the current state.
    const XrInstanceCreateInfo* FilterOutExtensions(const std::vector<const char*>& extensions_to_skip) {
        if (enabled_extensions_cstr.empty()) {
            return Get();
        }
        if (extensions_to_skip.empty()) {
            return Get();
        }
        for (auto& ext : extensions_to_skip) {
            FilterOutExtension(ext);
        }
        return Update();
    }
    // Remove the extension named in the parameter and return a pointer to the current state.
    const XrInstanceCreateInfo* FilterOutExtension(const char* extension_to_skip) {
        if (enabled_extensions_cstr.empty()) {
            return &modified_create_info;
        }
        auto b = enabled_extensions_cstr.begin();
        auto e = enabled_extensions_cstr.end();
        auto it = std::find_if(b, e, [&](const char* extension) { return strcmp(extension_to_skip, extension) == 0; });
        if (it != e) {
            // Just that one element goes away
            enabled_extensions_cstr.erase(it);
        }
        return Update();
    }

    // Get the current modified XrInstanceCreateInfo
    const XrInstanceCreateInfo* Get() const { return &modified_create_info; }

   private:
    const XrInstanceCreateInfo* Update() {
        modified_create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions_cstr.size());
        modified_create_info.enabledExtensionNames = enabled_extensions_cstr.empty() ? nullptr : enabled_extensions_cstr.data();
        return &modified_create_info;
    }
    const XrInstanceCreateInfo* original_create_info;

    XrInstanceCreateInfo modified_create_info;
    std::vector<const char*> enabled_extensions_cstr;
};
}  // namespace

// Factory method
XrResult LoaderInstance::CreateInstance(std::vector<std::unique_ptr<ApiLayerInterface>>&& api_layer_interfaces,
                                        const XrInstanceCreateInfo* info, XrInstance* instance) {
    XrResult last_error = XR_SUCCESS;
    LoaderLogger::LogVerboseMessage("xrCreateInstance", "Entering LoaderInstance::CreateInstance");

    // Topmost means "closest to the application"
    PFN_xrCreateInstance topmost_ci_fp = LoaderXrTermCreateInstance;
    PFN_xrCreateApiLayerInstance topmost_cali_fp = LoaderXrTermCreateApiLayerInstance;

    // Create the loader instance
    std::unique_ptr<LoaderInstance> loader_instance(new LoaderInstance(std::move(api_layer_interfaces)));
    *instance = reinterpret_cast<XrInstance>(loader_instance.get());

    // Remove the loader-supported-extensions (debug utils), if it's in the list of enabled extensions but not supported by
    // the runtime.
    InstanceCreateInfoManager create_info_manager{info};
    const XrInstanceCreateInfo* modified_create_info = info;
    if (info->enabledExtensionCount > 0) {
        std::vector<const char*> extensions_to_skip;
        for (const auto& ext : LoaderInstance::LoaderSpecificExtensions()) {
            if (!RuntimeInterface::GetRuntime().SupportsExtension(ext.extensionName)) {
                extensions_to_skip.emplace_back(ext.extensionName);
            }
        }
        modified_create_info = create_info_manager.FilterOutExtensions(extensions_to_skip);
    }

    // Only start the xrCreateApiLayerInstance stack if we have layers.
    std::vector<std::unique_ptr<ApiLayerInterface>>& layer_interfaces = loader_instance->LayerInterfaces();
    if (!layer_interfaces.empty()) {
        // Initialize an array of ApiLayerNextInfo structs
        std::unique_ptr<XrApiLayerNextInfo[]> next_info_list(new XrApiLayerNextInfo[layer_interfaces.size()]);
        auto ni_index = static_cast<uint32_t>(layer_interfaces.size() - 1);
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
            strncpy(next_info_list[ni_index].layerName, (*layer_interface)->LayerName().c_str(), XR_MAX_API_LAYER_NAME_SIZE - 1);
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
        api_layer_ci.nextInfo = next_info_list.get();
        //! @todo do we filter our create info extension list here?
        //! Think that actually each layer might need to filter...
        last_error = topmost_cali_fp(modified_create_info, &api_layer_ci, instance);

    } else {
        last_error = topmost_ci_fp(modified_create_info, instance);
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
                for (auto& loader_extension : LoaderInstance::LoaderSpecificExtensions()) {
                    if (strcmp(loader_extension.extensionName, info->enabledExtensionNames[ext]) == 0) {
                        found = true;
                        break;
                    }
                }
            }
            // Finally, check the enabled layers
            if (!found) {
                for (auto& layer_interface : layer_interfaces) {
                    if (layer_interface->SupportsExtension(info->enabledExtensionNames[ext])) {
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
        // Don't need to save the return value because we already set *instance
        (void)loader_instance.release();
    }

    // Always clear the input lists.  Either we use them or we don't.
    api_layer_interfaces.clear();

    return last_error;
}

LoaderInstance::LoaderInstance(std::vector<std::unique_ptr<ApiLayerInterface>>&& api_layer_interfaces)
    : _unique_id(0xDECAFBAD),
      _api_version(XR_CURRENT_API_VERSION),
      _api_layer_interfaces(std::move(api_layer_interfaces)),
      _dispatch_valid(false),
      _messenger(XR_NULL_HANDLE) {}

LoaderInstance::~LoaderInstance() {
    std::ostringstream oss;
    oss << "Destroying LoaderInstance = ";
    oss << PointerToHexString(this);
    LoaderLogger::LogInfoMessage("xrDestroyInstance", oss.str());
}

XrResult LoaderInstance::CreateDispatchTable(XrInstance instance) {
    XrResult res = XR_SUCCESS;
    // Create the top-level dispatch table.  First, we want to start with a dispatch table generated
    // using the commands from the runtime, with the exception of commands that we need a terminator
    // for.  The loaderGenInitInstanceDispatchTable utility function handles that automatically for us.
    std::unique_ptr<XrGeneratedDispatchTable> new_instance_dispatch_table(new XrGeneratedDispatchTable());
    LoaderGenInitInstanceDispatchTable(_runtime_instance, new_instance_dispatch_table);

    // Go through all layers, and override the instance pointers with the layer version.  However,
    // go backwards through the layer list so we replace in reverse order so the layers can call their next function
    // appropriately.
    if (!_api_layer_interfaces.empty()) {
        (*_api_layer_interfaces.begin())->GenUpdateInstanceDispatchTable(instance, new_instance_dispatch_table);
    }

    // Set the top-level instance dispatch table to the top-most commands now that we've figured them out.
    _dispatch_table = std::move(new_instance_dispatch_table);
    _dispatch_valid = true;
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
