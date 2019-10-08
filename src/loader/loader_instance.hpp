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

#include "extra_algorithms.h"

#include <openxr/openxr.h>

#include <array>
#include <cmath>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class LoaderInstance;
class ApiLayerInterface;
struct XrGeneratedDispatchTable;

typedef std::unique_lock<std::mutex> UniqueLock;
template <typename HandleType>
class HandleLoaderMap {
   public:
    using handle_t = HandleType;
    using map_t = std::unordered_map<HandleType, LoaderInstance*>;
    using value_t = typename map_t::value_type;

    /// Lookup a handle.
    /// Returns nullptr if not found.
    LoaderInstance* Get(HandleType handle);

    /// Insert an info for the supplied handle.
    /// Returns XR_ERROR_RUNTIME_FAILURE if it's null.
    /// Does not error if already there, because the loader is not currently very good at cleaning up handles.
    XrResult Insert(HandleType handle, LoaderInstance& loader);

    /// Remove the info associated with the supplied handle.
    /// Returns XR_ERROR_RUNTIME_FAILURE if it's null or not there.
    XrResult Erase(HandleType handle);

    /// Removes handles associated with a loader instance.
    void RemoveHandlesForLoader(LoaderInstance& loader);

   protected:
    map_t instance_map_;
    std::mutex mutex_;
};

class LoaderInstance {
   public:
    // Factory method
    static XrResult CreateInstance(std::vector<std::unique_ptr<ApiLayerInterface>>&& layer_interfaces,
                                   const XrInstanceCreateInfo* createInfo, XrInstance* instance);

    LoaderInstance(std::vector<std::unique_ptr<ApiLayerInterface>>&& api_layer_interfaces);
    virtual ~LoaderInstance();

    bool IsValid() { return _unique_id == 0xDECAFBAD; }
    XrVersion ApiVersion() { return _api_version; }
    XrResult CreateDispatchTable(XrInstance instance);
    void SetRuntimeInstance(XrInstance instance) { _runtime_instance = instance; }
    const std::unique_ptr<XrGeneratedDispatchTable>& DispatchTable() { return _dispatch_table; }
    std::vector<std::unique_ptr<ApiLayerInterface>>& LayerInterfaces() { return _api_layer_interfaces; }
    void AddEnabledExtension(const std::string& extension) { return _enabled_extensions.push_back(extension); }
    bool ExtensionIsEnabled(const std::string& extension);
    static const std::array<XrExtensionProperties, 1>& LoaderSpecificExtensions();
    XrDebugUtilsMessengerEXT DefaultDebugUtilsMessenger() { return _messenger; }
    void SetDefaultDebugUtilsMessenger(XrDebugUtilsMessengerEXT messenger) { _messenger = messenger; }

   private:
    uint32_t _unique_id;  // 0xDECAFBAD - for debugging
    XrVersion _api_version;
    std::vector<std::unique_ptr<ApiLayerInterface>> _api_layer_interfaces;
    XrInstance _runtime_instance;
    bool _dispatch_valid;
    std::unique_ptr<XrGeneratedDispatchTable> _dispatch_table;
    std::vector<std::string> _enabled_extensions;
    // Internal debug messenger created during xrCreateInstance
    XrDebugUtilsMessengerEXT _messenger;
};

template <typename HandleType>
inline LoaderInstance* HandleLoaderMap<HandleType>::Get(HandleType handle) {
    if (handle == XR_NULL_HANDLE) {
        return nullptr;
    }
    // Try to find the handle in the appropriate map
    UniqueLock lock(mutex_);
    auto entry_returned = instance_map_.find(handle);
    if (entry_returned == instance_map_.end()) {
        return nullptr;
    }
    return entry_returned->second;
}

template <typename HandleType>
inline XrResult HandleLoaderMap<HandleType>::Insert(HandleType handle, LoaderInstance& loader) {
    if (handle == XR_NULL_HANDLE) {
        // Internal error in loader or runtime.
        return XR_ERROR_RUNTIME_FAILURE;
    }
    UniqueLock lock(mutex_);
    //! @todo This check is currently disabled, because the loader is not good at cleaning up handles when their parent handles are
    //! destroyed.
#if 0
    auto entry_returned = instance_map_.find(handle);
    if (entry_returned != instance_map_.end()) {
        // Internal error in loader or runtime.
        return XR_ERROR_RUNTIME_FAILURE;
    }
#endif
    instance_map_[handle] = &loader;
    return XR_SUCCESS;
}

template <typename HandleType>
inline XrResult HandleLoaderMap<HandleType>::Erase(HandleType handle) {
    if (handle == XR_NULL_HANDLE) {
        // Internal error in loader or runtime.
        return XR_ERROR_RUNTIME_FAILURE;
    }
    UniqueLock lock(mutex_);
    auto entry_returned = instance_map_.find(handle);
    if (entry_returned == instance_map_.end()) {
        // Internal error in loader or runtime.
        return XR_ERROR_RUNTIME_FAILURE;
    }
    instance_map_.erase(handle);
    return XR_SUCCESS;
}

template <typename HandleType>
inline void HandleLoaderMap<HandleType>::RemoveHandlesForLoader(LoaderInstance& loader) {
    UniqueLock lock(mutex_);
    auto search_value = &loader;
    map_erase_if(instance_map_, [=](value_t const& data) { return data.second && data.second == search_value; });
}
