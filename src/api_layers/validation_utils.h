// Copyright (c) 2018-2019 The Khronos Group Inc.
// Copyright (c) 2018-2019 Valve Corporation
// Copyright (c) 2018-2019 LunarG, Inc.
// Copyright (c) 2019, Collabora, Ltd.
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
// Author: Ryan Pavlik <ryan.pavlik@collabora.com>
//         Mark Young <marky@lunarg.com>
//

#ifndef VALIDATION_UTILS_H_
#define VALIDATION_UTILS_H_ 1

#include "api_layer_platform_defines.h"

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <vector>
#include <unordered_map>
#include <string>
#include <mutex>
#include <memory>

// Handle information for easy conversion
#if (defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__)) || defined(_M_X64) || defined(__ia64) || \
     defined(_M_IA64) || defined(__aarch64__) || defined(__powerpc64__))
#define XR_VALIDATION_GENERIC_HANDLE_TYPE uint64_t
#define CONVERT_HANDLE_TO_GENERIC(handle) reinterpret_cast<uint64_t>(handle)
#define CONVERT_GENERIC_TO_HANDLE(type, handle) reinterpret_cast<type>(handle)
#define CHECK_FOR_NULL_HANDLE(handle) (XR_NULL_HANDLE == reinterpret_cast<void *>(handle))
#else
#define XR_VALIDATION_GENERIC_HANDLE_TYPE uint64_t
#define CONVERT_HANDLE_TO_GENERIC(handle) (handle)
#define CONVERT_GENERIC_TO_HANDLE(type, handle) (handle)
#define CHECK_FOR_NULL_HANDLE(handle) (XR_NULL_HANDLE == handle)
#endif
// Structure used for storing the instance information we need for validating
// various aspects of the OpenXR API.

// Debug Utils items
struct CoreValidationMessengerInfo {
    XrDebugUtilsMessengerEXT messenger;
    XrDebugUtilsMessengerCreateInfoEXT *create_info;
};

struct XrGeneratedDispatchTable;

struct CoreValidationMessengerInfoDeleter {
    void operator()(CoreValidationMessengerInfo *ptr) const {
        delete ptr->create_info;
        delete ptr;
    }
};

struct XrDebugUtilsObjectNameInfoEXTDeleter {
    void operator()(XrDebugUtilsObjectNameInfoEXT *ptr) const {
        delete ptr->objectName;
        delete ptr;
    }
};

typedef std::unique_ptr<XrDebugUtilsObjectNameInfoEXT, XrDebugUtilsObjectNameInfoEXTDeleter> UniqueXrDebugUtilsObjectNameInfoEXT;

typedef std::unique_ptr<CoreValidationMessengerInfo, CoreValidationMessengerInfoDeleter> UniqueCoreValidationMessengerInfo;

// Define the instance struct used for passing information around.
// This information includes things like the dispatch table as well as the
// enabled extensions.
struct GenValidUsageXrInstanceInfo {
    GenValidUsageXrInstanceInfo(XrInstance inst, PFN_xrGetInstanceProcAddr next_get_instance_proc_addr);
    ~GenValidUsageXrInstanceInfo();
    const XrInstance instance;
    XrGeneratedDispatchTable *dispatch_table;
    std::vector<std::string> enabled_extensions;
    std::vector<UniqueCoreValidationMessengerInfo> debug_messengers;
    std::vector<UniqueXrDebugUtilsObjectNameInfoEXT> object_names;
};

// Structure used for storing information for other handles
struct GenValidUsageXrHandleInfo {
    GenValidUsageXrInstanceInfo *instance_info;
    XrObjectType direct_parent_type;
    XR_VALIDATION_GENERIC_HANDLE_TYPE direct_parent_handle;
};

// Structure used for storing session label information
struct GenValidUsageXrInternalSessionLabel {
    XrDebugUtilsLabelEXT debug_utils_label;
    std::string label_name;
    bool is_individual_label;
};

// Enum used for indicating handle validation status.
enum ValidateXrHandleResult {
    VALIDATE_XR_HANDLE_NULL,
    VALIDATE_XR_HANDLE_INVALID,
    VALIDATE_XR_HANDLE_SUCCESS,
};

// Unordered Map associating pointer to a vector of session label information to a session's handle
extern std::unordered_map<XrSession, std::vector<GenValidUsageXrInternalSessionLabel *> *> g_xr_session_labels;

// This function is used to delete session labels when a session is destroyed
extern void CoreValidationDeleteSessionLabels(XrSession session);

// Object information used for logging.
struct GenValidUsageXrObjectInfo {
    XR_VALIDATION_GENERIC_HANDLE_TYPE handle;
    XrObjectType type;
    GenValidUsageXrObjectInfo() = default;
    template <typename T>
    GenValidUsageXrObjectInfo(T h, XrObjectType t) : handle(CONVERT_HANDLE_TO_GENERIC(h)), type(t) {}
};

// Debug message severity levels for logging.
enum GenValidUsageDebugSeverity {
    VALID_USAGE_DEBUG_SEVERITY_DEBUG = 0,
    VALID_USAGE_DEBUG_SEVERITY_INFO = 7,
    VALID_USAGE_DEBUG_SEVERITY_WARNING = 14,
    VALID_USAGE_DEBUG_SEVERITY_ERROR = 21,
};

// in core_validation.cpp
void EraseAllInstanceTableMapElements(GenValidUsageXrInstanceInfo *search_value);

typedef std::unique_lock<std::mutex> UniqueLock;
template <typename HandleType, typename InfoType>
class HandleInfoBase {
   public:
    typedef InfoType info_t;
    typedef HandleType handle_t;
    typedef std::unordered_map<HandleType, std::unique_ptr<InfoType>> map_t;
    typedef typename map_t::value_type value_t;

    /// Validate a handle.
    ///
    /// Returns an enum indicating null, invalid (not found), or success.
    ValidateXrHandleResult verifyHandle(HandleType const *handle_to_check);

    /// Lookup a handle.
    /// Throws if not found.
    InfoType *get(HandleType handle);

    /// Lookup a handle, returning a pointer (if found) as well as a lock for this object's dispatch mutex.
    std::pair<UniqueLock, InfoType *> getWithLock(HandleType handle);

    bool empty() const { return info_map_.empty(); }

    /// Insert an info for the supplied handle.
    /// Throws if it's already there.
    void insert(HandleType handle, std::unique_ptr<InfoType> &&info);

    /// Remove the info associated with the supplied handle.
    /// Throws if not found.
    void erase(HandleType handle);

    /// Get a constant reference to the whole map as well as a lock for this object's dispatch mutex.
    std::pair<UniqueLock, map_t const &> lockMapConst();

    /// Get a  reference to the whole map as well as a lock for this object's dispatch mutex.
    std::pair<UniqueLock, map_t &> lockMap();

   protected:
    map_t info_map_;
    std::mutex dispatch_mutex_;
};

/// Subclass used exclusively for instances.
class InstanceHandleInfo : public HandleInfoBase<XrInstance, GenValidUsageXrInstanceInfo> {
   public:
    typedef HandleInfoBase<XrInstance, GenValidUsageXrInstanceInfo> base_t;
    typedef typename base_t::info_t info_t;
    typedef typename base_t::handle_t handle_t;
};

/// Generic handle info for everything-except-instance handles.
template <typename HandleType>
class HandleInfo : public HandleInfoBase<HandleType, GenValidUsageXrHandleInfo> {
   public:
    typedef HandleInfoBase<HandleType, GenValidUsageXrHandleInfo> base_t;
    typedef typename base_t::info_t info_t;
    typedef typename base_t::handle_t handle_t;

    /// Lookup a handle and its instance info
    /// Throws if not found.
    std::pair<GenValidUsageXrHandleInfo *, GenValidUsageXrInstanceInfo *> getWithInstanceInfo(HandleType handle);

    /// Removes handles associated  with an instance.
    void removeHandlesForInstance(GenValidUsageXrInstanceInfo *search_value);
};

template <typename T, typename Pred>
inline void map_erase_if(T &container, Pred &&predicate) {
    for (auto it = container.begin(); it != container.end();) {
        if (predicate(*it)) {
            auto here = it;
            ++it;
            container.erase(here);
        } else {
            ++it;
        }
    }
}

// -- Only implementations of templates follow --//

template <typename HandleType, typename InfoType>
inline std::pair<UniqueLock, InfoType *> HandleInfoBase<HandleType, InfoType>::getWithLock(HandleType handle) {
    // Try to find the handle in the appropriate map
    UniqueLock lock(dispatch_mutex_);
    auto it = info_map_.find(handle);
    // If it is not a valid handle, it should return the end of the map.
    if (info_map_.end() == it) {
        return {std::move(lock), nullptr};
    }
    return {std::move(lock), it->second.get()};
}

template <typename HT, typename IT>
inline std::pair<UniqueLock, typename HandleInfoBase<HT, IT>::map_t const &> HandleInfoBase<HT, IT>::lockMapConst() {
    return {UniqueLock(dispatch_mutex_), info_map_};
}

template <typename HT, typename IT>
inline std::pair<UniqueLock, typename HandleInfoBase<HT, IT>::map_t &> HandleInfoBase<HT, IT>::lockMap() {
    return {UniqueLock(dispatch_mutex_), info_map_};
}

template <typename HandleType, typename InfoType>
inline ValidateXrHandleResult HandleInfoBase<HandleType, InfoType>::verifyHandle(HandleType const *handle_to_check) {
    try {
        if (nullptr == handle_to_check) {
            return VALIDATE_XR_HANDLE_INVALID;
        }
        // XR_NULL_HANDLE is valid in some cases, so we want to return that we found that value
        // and let the calling function decide what to do with it.
        if (*handle_to_check == XR_NULL_HANDLE) {
            return VALIDATE_XR_HANDLE_NULL;
        }

        // Try to find the handle in the appropriate map
        UniqueLock lock(dispatch_mutex_);
        auto entry_returned = info_map_.find(*handle_to_check);
        // If it is not a valid handle, it should return the end of the map.
        if (info_map_.end() == entry_returned) {
            return VALIDATE_XR_HANDLE_INVALID;
        }
        return VALIDATE_XR_HANDLE_SUCCESS;
    } catch (...) {
        return VALIDATE_XR_HANDLE_INVALID;
    }
}

template <typename HandleType, typename InfoType>
inline InfoType *HandleInfoBase<HandleType, InfoType>::get(HandleType handle) {
    // Try to find the handle in the appropriate map
    UniqueLock lock(dispatch_mutex_);
    auto entry_returned = info_map_.find(handle);
    if (entry_returned == info_map_.end()) {
        throw std::logic_error("Internal layer error: Handle not inserted");
    }
    return entry_returned->second.get();
}

template <typename HandleType, typename InfoType>
inline void HandleInfoBase<HandleType, InfoType>::insert(HandleType handle, std::unique_ptr<InfoType> &&info) {
    UniqueLock lock(dispatch_mutex_);
    auto entry_returned = info_map_.find(handle);
    if (entry_returned != info_map_.end()) {
        throw std::logic_error("Internal layer error: Handle already inserted");
    }
    info_map_[handle] = std::move(info);
}
template <typename HandleType, typename InfoType>
inline void HandleInfoBase<HandleType, InfoType>::erase(HandleType handle) {
    UniqueLock lock(dispatch_mutex_);
    auto entry_returned = info_map_.find(handle);
    if (entry_returned == info_map_.end()) {
        throw std::logic_error("Internal layer error: Handle not inserted");
    }
    info_map_.erase(handle);
}

template <typename HandleType>
inline std::pair<GenValidUsageXrHandleInfo *, GenValidUsageXrInstanceInfo *> HandleInfo<HandleType>::getWithInstanceInfo(
    HandleType handle) {
    // Try to find the handle in the appropriate map
    UniqueLock lock(this->dispatch_mutex_);
    auto entry_returned = this->info_map_.find(handle);
    if (entry_returned == this->info_map_.end()) {
        throw std::logic_error("Handle not registered, internal layer error.");
    }
    GenValidUsageXrHandleInfo *info = entry_returned->second.get();
    GenValidUsageXrInstanceInfo *instance_info = info->instance_info;
    return {info, instance_info};
}

template <typename HandleType>
inline void HandleInfo<HandleType>::removeHandlesForInstance(GenValidUsageXrInstanceInfo *search_value) {
    typedef typename base_t::value_t value_t;
    UniqueLock lock(this->dispatch_mutex_);
    map_erase_if(this->info_map_, [=](value_t const &data) { return data.second && data.second->instance_info == search_value; });
}

#endif  // VALIDATION_UTILS_H_
