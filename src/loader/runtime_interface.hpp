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

#include "loader_platform.hpp"

#include <openxr/openxr.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>

struct XrGeneratedDispatchTable;

class RuntimeInterface {
   public:
    virtual ~RuntimeInterface();

    // Helper functions for loading and unloading the runtime (but only when necessary)
    static XrResult LoadRuntime(const std::string& openxr_command);
    static void UnloadRuntime(const std::string& openxr_command);
    static RuntimeInterface& GetRuntime() { return *(_single_runtime_interface.get()); }
    static XrResult GetInstanceProcAddr(XrInstance instance, const char* name, PFN_xrVoidFunction* function);
    static const XrGeneratedDispatchTable* GetDispatchTable(XrInstance instance);
    static const XrGeneratedDispatchTable* GetDebugUtilsMessengerDispatchTable(XrDebugUtilsMessengerEXT messenger);

    void GetInstanceExtensionProperties(std::vector<XrExtensionProperties>& extension_properties);
    bool SupportsExtension(const std::string& extension_name);
    XrResult CreateInstance(const XrInstanceCreateInfo* info, XrInstance* instance);
    XrResult DestroyInstance(XrInstance instance);
    bool TrackDebugMessenger(XrInstance instance, XrDebugUtilsMessengerEXT messenger);
    void ForgetDebugMessenger(XrDebugUtilsMessengerEXT messenger);

    // No default construction
    RuntimeInterface() = delete;

    // Non-copyable
    RuntimeInterface(const RuntimeInterface&) = delete;
    RuntimeInterface& operator=(const RuntimeInterface&) = delete;

   private:
    RuntimeInterface(LoaderPlatformLibraryHandle runtime_library, PFN_xrGetInstanceProcAddr get_instant_proc_addr);
    void SetSupportedExtensions(std::vector<std::string>& supported_extensions);

    static std::unique_ptr<RuntimeInterface> _single_runtime_interface;
    static uint32_t _single_runtime_count;
    LoaderPlatformLibraryHandle _runtime_library;
    PFN_xrGetInstanceProcAddr _get_instance_proc_addr;
    std::unordered_map<XrInstance, std::unique_ptr<XrGeneratedDispatchTable>> _dispatch_table_map;
    std::mutex _dispatch_table_mutex;
    std::unordered_map<XrDebugUtilsMessengerEXT, XrInstance> _messenger_to_instance_map;
    std::mutex _messenger_to_instance_mutex;
    std::vector<std::string> _supported_extensions;
};
