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

#pragma once

#include <string>
#include <vector>

#include "loader_platform.hpp"
#include "platform_utils.hpp"
#include "runtime_interface.hpp"
#include "api_layer_interface.hpp"
#include "xr_generated_dispatch_table.h"

class LoaderInstance {
   public:
    // Factory method
    static XrResult CreateInstance(std::vector<std::unique_ptr<ApiLayerInterface>>& layer_interfaces,
                                   const XrInstanceCreateInfo* info, XrInstance* instance);

    LoaderInstance(std::vector<std::unique_ptr<ApiLayerInterface>>& api_layer_interfaces);
    virtual ~LoaderInstance();

    bool IsValid() { return _unique_id == 0xDECAFBAD; }
    uint32_t ApiVersion() { return _api_version; }
    XrResult CreateDispatchTable(XrInstance instance);
    void SetRuntimeInstance(XrInstance instance) { _runtime_instance = instance; }
    const std::unique_ptr<XrGeneratedDispatchTable>& DispatchTable() { return _dispatch_table; }
    std::vector<std::unique_ptr<ApiLayerInterface>>& LayerInterfaces() { return _api_layer_interfaces; }
    void AddEnabledExtension(const std::string& extension) { return _enabled_extensions.push_back(extension); }
    bool ExtensionIsEnabled(const std::string& extension);
    static const std::vector<XrExtensionProperties>& LoaderSpecificExtensions() { return _loader_supported_extensions; }
    XrDebugUtilsMessengerEXT DefaultDebugUtilsMessenger() { return _messenger; }
    void SetDefaultDebugUtilsMessenger(XrDebugUtilsMessengerEXT messenger) { _messenger = messenger; }

   private:
    uint32_t _unique_id;  // 0xDECAFBAD - for debugging
    uint32_t _api_version;
    std::vector<std::unique_ptr<ApiLayerInterface>> _api_layer_interfaces;
    XrInstance _runtime_instance;
    bool _dispatch_valid;
    std::unique_ptr<XrGeneratedDispatchTable> _dispatch_table;
    static const std::vector<XrExtensionProperties> _loader_supported_extensions;
    std::vector<std::string> _enabled_extensions;
    // Internal debug messenger created during xrCreateInstance
    XrDebugUtilsMessengerEXT _messenger;
};
