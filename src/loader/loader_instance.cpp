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
XrResult LoaderInstance::CreateInstance(std::vector<std::unique_ptr<ApiLayerInterface>>& api_layer_interfaces,
                                        const XrInstanceCreateInfo* info, XrInstance* instance) {
    XrResult last_error = XR_SUCCESS;
    try {
        LoaderLogger::LogVerboseMessage("xrCreateInstance", "Entering LoaderInstance::CreateInstance");
        std::stack<PFN_xrGetInstanceProcAddr> get_instance_proc_addr_stack;
        std::stack<PFN_xrCreateInstance> create_instance_stack;
        std::stack<PFN_xrCreateApiLayerInstance> create_api_layer_instance_stack;
        LoaderInstance* loader_instance;

        get_instance_proc_addr_stack.push(LoaderXrTermGetInstanceProcAddr);
        create_instance_stack.push(LoaderXrTermCreateInstance);

        // Now create the loader instance instance
        loader_instance = new LoaderInstance(api_layer_interfaces);
        *instance = reinterpret_cast<XrInstance>(loader_instance);

        std::vector<std::unique_ptr<ApiLayerInterface>>& instance_layer_interfaces = loader_instance->LayerInterfaces();

        // Only start the xrCreateApiLayerInstance stack if we have layers.
        if (instance_layer_interfaces.size() > 0) {
            XrApiLayerCreateInfo api_layer_create_info = {};
            XrApiLayerNextInfo* next_api_layer_info = new XrApiLayerNextInfo[instance_layer_interfaces.size()];
            uint32_t next_info_index = static_cast<uint32_t>(instance_layer_interfaces.size() - 1);

            create_api_layer_instance_stack.push(LoaderXrTermCreateApiLayerInstance);

            // Go through all layers, and override the instance pointers with the layer version.  However,
            // go backwards through the layer list so we replace in reverse order so the layers can call their next function
            // appropriately.
            XrApiLayerNextInfo* last_layer_next_api_layer_info = nullptr;
            PFN_xrGetInstanceProcAddr last_layer_get_instance_proc_addr = LoaderXrTermGetInstanceProcAddr;
            PFN_xrCreateApiLayerInstance last_layer_create_api_layer_instance = LoaderXrTermCreateApiLayerInstance;
            for (auto layer_interface = instance_layer_interfaces.rbegin(); layer_interface != instance_layer_interfaces.rend();
                 ++layer_interface) {
                PFN_xrGetInstanceProcAddr layer_get_instance_proc_addr = (*layer_interface)->GetInstanceProcAddrFuncPointer();
                PFN_xrCreateInstance layer_create_instance;
                layer_get_instance_proc_addr(XR_NULL_HANDLE, "xrCreateInstance",
                                             reinterpret_cast<PFN_xrVoidFunction*>(&layer_create_instance));
                get_instance_proc_addr_stack.push(layer_get_instance_proc_addr);
                create_instance_stack.push(layer_create_instance);
                PFN_xrCreateApiLayerInstance layer_create_api_layer_instance =
                    (*layer_interface)->GetCreateApiLayerInstanceFuncPointer();
                create_api_layer_instance_stack.push(layer_create_api_layer_instance);
                next_api_layer_info[next_info_index].structType = XR_LOADER_INTERFACE_STRUCT_API_LAYER_NEXT_INFO;
                next_api_layer_info[next_info_index].structVersion = XR_API_LAYER_NEXT_INFO_STRUCT_VERSION;
                next_api_layer_info[next_info_index].structSize = sizeof(XrApiLayerNextInfo);
                strncpy(next_api_layer_info[next_info_index].layerName, (*layer_interface)->LayerName().c_str(),
                        XR_MAX_API_LAYER_NAME_SIZE - 1);
                next_api_layer_info[next_info_index].layerName[XR_MAX_API_LAYER_NAME_SIZE - 1] = '\0';

                next_api_layer_info[next_info_index].next = last_layer_next_api_layer_info;
                next_api_layer_info[next_info_index].nextGetInstanceProcAddr = last_layer_get_instance_proc_addr;
                next_api_layer_info[next_info_index].nextCreateApiLayerInstance = last_layer_create_api_layer_instance;
                last_layer_next_api_layer_info = &next_api_layer_info[next_info_index];
                last_layer_get_instance_proc_addr = layer_get_instance_proc_addr;
                last_layer_create_api_layer_instance = layer_create_api_layer_instance;
                next_info_index--;
            }

            // Setup the layer create info
            api_layer_create_info.structType = XR_LOADER_INTERFACE_STRUCT_API_LAYER_CREATE_INFO;
            api_layer_create_info.structVersion = XR_API_LAYER_CREATE_INFO_STRUCT_VERSION;
            api_layer_create_info.structSize = sizeof(XrApiLayerCreateInfo);
            api_layer_create_info.loaderInstance = reinterpret_cast<void*>(loader_instance);
            api_layer_create_info.settings_file_location[0] = '\0';
            api_layer_create_info.nextInfo = next_api_layer_info;

            last_error = create_api_layer_instance_stack.top()(info, &api_layer_create_info, instance);

            delete[] next_api_layer_info;
        } else {
            last_error = create_instance_stack.top()(info, instance);
        }

        if (XR_SUCCESS == last_error) {
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
                    for (auto layer_interface = instance_layer_interfaces.begin();
                         layer_interface != instance_layer_interfaces.end(); ++layer_interface) {
                        if ((*layer_interface)->SupportsExtension(info->enabledExtensionNames[ext])) {
                            found = true;
                            break;
                        }
                    }
                }
                if (!found) {
                    LoaderLogger::LogErrorMessage("xrCreateInstance",
                                                  "LoaderInstance::CreateInstance encountered unsupported extension.");
                    last_error = XR_ERROR_EXTENSION_NOT_PRESENT;
                    break;
                }
                loader_instance->AddEnabledExtension(info->enabledExtensionNames[ext]);
            }
        } else {
            LoaderLogger::LogErrorMessage("xrCreateInstance", "LoaderInstance::CreateInstance chained CreateInstance call failed");
        }

        if (XR_SUCCESS == last_error) {
            // Create the top-level dispatch table for the instance.  This will contain the function pointers to the
            // first instantiation of every command, whether that is in a layer, or a runtime.
            last_error = loader_instance->CreateDispatchTable(*instance);
            if (XR_SUCCESS != last_error) {
                LoaderLogger::LogErrorMessage("xrCreateInstance",
                                              "LoaderInstance::CreateInstance failed creating top-level dispatch table");
            }

            std::unique_lock<std::mutex> lock(g_instance_mutex);
            auto exists = g_instance_map.find(*instance);
            if (exists == g_instance_map.end()) {
                g_instance_map[*instance] = loader_instance;
            }
        }

        if (XR_SUCCESS != last_error) {
            delete loader_instance;
            loader_instance = nullptr;
        } else {
            std::string info_message = "LoaderInstance::CreateInstance succeeded with ";
            info_message += std::to_string(loader_instance->LayerInterfaces().size());
            info_message += " layers enabled and runtime interface - created instance = 0x";
            std::ostringstream oss;
            oss << std::hex << reinterpret_cast<uintptr_t>(loader_instance);
            info_message += oss.str();
            LoaderLogger::LogInfoMessage("xrCreateInstance", info_message);
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

LoaderInstance::LoaderInstance(std::vector<std::unique_ptr<ApiLayerInterface>>& api_layer_interfaces)
    : _unique_id(0xDECAFBAD), _api_version(XR_CURRENT_API_VERSION), _dispatch_valid(false), _messenger(XR_NULL_HANDLE) {
    try {
        for (auto l_iter = api_layer_interfaces.begin(); api_layer_interfaces.size() > 0 && l_iter != api_layer_interfaces.end();
             /* No iterate */) {
            _api_layer_interfaces.push_back(std::move(*l_iter));
            api_layer_interfaces.erase(l_iter);
        }
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrCreateInstance", "LoaderInstance::LoaderInstance - Unknown error occurred");
        throw;
    }
}

LoaderInstance::~LoaderInstance() {
    std::string info_message = "Destroying LoaderInstance = 0x";
    std::ostringstream oss;
    oss << std::hex << reinterpret_cast<uintptr_t>(this);
    info_message += oss.str();
    LoaderLogger::LogInfoMessage("xrDestroyInstance", info_message);
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
