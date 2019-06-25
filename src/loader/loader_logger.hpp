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

#include <memory>
#include <mutex>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

// Use internal versions of flags similar to XR_EXT_debug_utils so that
// we're not tightly coupled to that extension.  This way, if the extension
// changes or gets replaced, we can be flexible in the loader.
#define XR_LOADER_LOG_MESSAGE_SEVERITY_VERBOSE_BIT 0x00000001
#define XR_LOADER_LOG_MESSAGE_SEVERITY_INFO_BIT 0x00000010
#define XR_LOADER_LOG_MESSAGE_SEVERITY_WARNING_BIT 0x00000100
#define XR_LOADER_LOG_MESSAGE_SEVERITY_ERROR_BIT 0x00001000
typedef XrFlags64 XrLoaderLogMessageSeverityFlagBits;
typedef XrFlags64 XrLoaderLogMessageSeverityFlags;

#define XR_LOADER_LOG_MESSAGE_TYPE_GENERAL_BIT 0x00000001
#define XR_LOADER_LOG_MESSAGE_TYPE_SPECIFICATION_BIT 0x00000002
#define XR_LOADER_LOG_MESSAGE_TYPE_PERFORMANCE_BIT 0x00000004
typedef XrFlags64 XrLoaderLogMessageTypeFlagBits;
typedef XrFlags64 XrLoaderLogMessageTypeFlags;

//! Turns a uint64_t into a string formatted as hex.
//!
//! The core of the HandleToString implementation is in here.
std::string Uint64ToHexString(uint64_t val);

template <typename T>
static inline std::string HandleToString(T handle) {
    return Uint64ToHexString(reinterpret_cast<uint64_t>(handle));
}

struct XrLoaderLogObjectInfo {
    //! Type-erased handle value
    uint64_t handle;

    //! Kind of object this handle refers to
    XrObjectType type;

    //! To be assigned by the application - not part of this object's identity
    std::string name;

    /// Un-erase the type of the handle and get it properly typed again.
    ///
    /// Note: Does not check the type before doing it!
    template <typename HandleType>
    HandleType& GetTypedHandle() {
        return reinterpret_cast<HandleType&>(handle);
    }

    //! @overload
    template <typename HandleType>
    HandleType const& GetTypedHandle() const {
        return reinterpret_cast<HandleType&>(handle);
    }

    XrLoaderLogObjectInfo() = default;

    template <typename T>
    XrLoaderLogObjectInfo(T h, XrObjectType t) : handle(reinterpret_cast<uint64_t>(h)), type(t) {}

    std::string ToString() const;
};

//! True if the two object infos have the same handle value and handle type
static inline bool Equivalent(XrLoaderLogObjectInfo const& a, XrLoaderLogObjectInfo const& b) {
    return a.handle == b.handle && a.type == b.type;
}
//! @overload
static inline bool Equivalent(XrDebugUtilsObjectNameInfoEXT const& a, XrLoaderLogObjectInfo const& b) {
    return a.objectHandle == b.handle && a.objectType == b.type;
}
//! @overload
static inline bool Equivalent(XrLoaderLogObjectInfo const& a, XrDebugUtilsObjectNameInfoEXT const& b) { return Equivalent(b, a); }

struct XrLoaderLogMessengerCallbackData {
    const char* message_id;
    const char* command_name;
    const char* message;
    uint8_t object_count;
    XrLoaderLogObjectInfo* objects;
    uint8_t session_labels_count;
    XrDebugUtilsLabelEXT* session_labels;
};

enum XrLoaderLogType {
    XR_LOADER_LOG_UNKNOWN = 0,
    XR_LOADER_LOG_STDERR,
    XR_LOADER_LOG_STDOUT,
    XR_LOADER_LOG_DEBUG_UTILS,
};

class LoaderLogRecorder {
   public:
    LoaderLogRecorder(XrLoaderLogType type, void* user_data, XrLoaderLogMessageSeverityFlags message_severities,
                      XrLoaderLogMessageTypeFlags message_types) {
        _active = false;
        _user_data = user_data;
        _type = type;
        _unique_id = 0;
        _message_severities = message_severities;
        _message_types = message_types;
    }
    virtual ~LoaderLogRecorder() {}

    XrLoaderLogType Type() { return _type; }
    uint64_t UniqueId() { return _unique_id; }
    XrLoaderLogMessageSeverityFlags MessageSeverities() { return _message_severities; }
    XrLoaderLogMessageTypeFlags MessageTypes() { return _message_types; }

    virtual void Start() { _active = true; }
    bool IsPaused() { return _active; }
    virtual void Pause() { _active = false; }
    virtual void Resume() { _active = true; }
    virtual void Stop() { _active = false; }

    virtual bool LogMessage(XrLoaderLogMessageSeverityFlagBits message_severity, XrLoaderLogMessageTypeFlags message_type,
                            const XrLoaderLogMessengerCallbackData* callback_data) = 0;

   protected:
    bool _active;
    XrLoaderLogType _type;
    uint64_t _unique_id;
    void* _user_data;
    XrLoaderLogMessageSeverityFlags _message_severities;
    XrLoaderLogMessageTypeFlags _message_types;
};

// Standard Error logger, always on for now
class StdErrLoaderLogRecorder : public LoaderLogRecorder {
   public:
    StdErrLoaderLogRecorder(void* user_data);
    ~StdErrLoaderLogRecorder() {}

    virtual bool LogMessage(XrLoaderLogMessageSeverityFlagBits message_severity, XrLoaderLogMessageTypeFlags message_type,
                            const XrLoaderLogMessengerCallbackData* callback_data);
};

// Standard Output logger used with XR_LOADER_DEBUG
class StdOutLoaderLogRecorder : public LoaderLogRecorder {
   public:
    StdOutLoaderLogRecorder(void* user_data, XrLoaderLogMessageSeverityFlags flags);
    ~StdOutLoaderLogRecorder() {}

    virtual bool LogMessage(XrLoaderLogMessageSeverityFlagBits message_severity, XrLoaderLogMessageTypeFlags message_type,
                            const XrLoaderLogMessengerCallbackData* callback_data);
};

// Debug Utils logger used with XR_EXT_debug_utils
class DebugUtilsLogRecorder : public LoaderLogRecorder {
   public:
    DebugUtilsLogRecorder(const XrDebugUtilsMessengerCreateInfoEXT* create_info, XrDebugUtilsMessengerEXT debug_messenger);
    ~DebugUtilsLogRecorder() {}

    virtual bool LogMessage(XrLoaderLogMessageSeverityFlagBits message_severity, XrLoaderLogMessageTypeFlags message_type,
                            const XrLoaderLogMessengerCallbackData* callback_data);

    // Extension-specific logging functions
    bool LogDebugUtilsMessage(XrDebugUtilsMessageSeverityFlagsEXT message_severity, XrDebugUtilsMessageTypeFlagsEXT message_type,
                              const XrDebugUtilsMessengerCallbackDataEXT* callback_data);

   private:
    PFN_xrDebugUtilsMessengerCallbackEXT _user_callback;
};

// TODO: Add other Derived classes:
//  - FileLoaderLogRecorder     - During/after xrCreateInstance
//  - PipeLoaderLogRecorder?    - During/after xrCreateInstance

class ObjectInfoCollection {
   public:
    //! Called from LoaderXrTermSetDebugUtilsObjectNameEXT - an empty name means remove
    void AddObjectName(uint64_t object_handle, XrObjectType object_type, const std::string& object_name);

    //! Find the stored object info, if any, matching handle and type.
    //! Return nullptr if not found.
    XrLoaderLogObjectInfo const* LookUpStoredObjectInfo(XrLoaderLogObjectInfo const& info) const;
    //! Find the stored object info, if any, matching handle and type.
    //! Return nullptr if not found.
    XrLoaderLogObjectInfo* LookUpStoredObjectInfo(XrLoaderLogObjectInfo const& info);

    //! Find the stored object info, if any.
    //! Return nullptr if not found.
    XrLoaderLogObjectInfo const* LookUpStoredObjectInfo(uint64_t handle, XrObjectType type) const {
        return LookUpStoredObjectInfo({handle, type});
    }

    //! Find the object name, if any, and update debug utils info accordingly.
    //! Return true if found and updated.
    bool LookUpObjectName(XrDebugUtilsObjectNameInfoEXT& info) const;

    //! Find the object name, if any, and update logging info accordingly.
    //! Return true if found and updated.
    bool LookUpObjectName(XrLoaderLogObjectInfo& info) const;

    //! Is the collection empty?
    bool Empty() const { return _object_info.empty(); }

   private:
    // Object names that have been set for given objects
    std::vector<XrLoaderLogObjectInfo> _object_info;
};
class LoaderLogger {
   public:
    static LoaderLogger& GetInstance() {
        std::call_once(LoaderLogger::_once_flag, []() { _instance.reset(new LoaderLogger); });
        return *(_instance.get());
    }

    void AddLogRecorder(std::unique_ptr<LoaderLogRecorder>& recorder);
    void RemoveLogRecorder(uint64_t unique_id);

    //! Called from LoaderXrTermSetDebugUtilsObjectNameEXT - an empty name means remove
    void AddObjectName(uint64_t object_handle, XrObjectType object_type, const std::string& object_name);
    void BeginLabelRegion(XrSession session, const XrDebugUtilsLabelEXT* label_info);
    void EndLabelRegion(XrSession session);
    void InsertLabel(XrSession session, const XrDebugUtilsLabelEXT* label_info);
    void DeleteSessionLabels(XrSession session);


    bool LogMessage(XrLoaderLogMessageSeverityFlagBits message_severity, XrLoaderLogMessageTypeFlags message_type,
                    const std::string& message_id, const std::string& command_name, const std::string& message,
                    const std::vector<XrLoaderLogObjectInfo>& objects = {});
    static bool LogErrorMessage(const std::string& command_name, const std::string& message,
                                const std::vector<XrLoaderLogObjectInfo>& objects = {}) {
        return GetInstance().LogMessage(XR_LOADER_LOG_MESSAGE_SEVERITY_ERROR_BIT, XR_LOADER_LOG_MESSAGE_TYPE_GENERAL_BIT,
                                        "OpenXR-Loader", command_name, message, objects);
    }
    static bool LogWarningMessage(const std::string& command_name, const std::string& message,
                                  const std::vector<XrLoaderLogObjectInfo>& objects = {}) {
        return GetInstance().LogMessage(XR_LOADER_LOG_MESSAGE_SEVERITY_WARNING_BIT, XR_LOADER_LOG_MESSAGE_TYPE_GENERAL_BIT,
                                        "OpenXR-Loader", command_name, message, objects);
    }
    static bool LogInfoMessage(const std::string& command_name, const std::string& message,
                               const std::vector<XrLoaderLogObjectInfo>& objects = {}) {
        return GetInstance().LogMessage(XR_LOADER_LOG_MESSAGE_SEVERITY_INFO_BIT, XR_LOADER_LOG_MESSAGE_TYPE_GENERAL_BIT,
                                        "OpenXR-Loader", command_name, message, objects);
    }
    static bool LogVerboseMessage(const std::string& command_name, const std::string& message,
                                  const std::vector<XrLoaderLogObjectInfo>& objects = {}) {
        return GetInstance().LogMessage(XR_LOADER_LOG_MESSAGE_SEVERITY_VERBOSE_BIT, XR_LOADER_LOG_MESSAGE_TYPE_GENERAL_BIT,
                                        "OpenXR-Loader", command_name, message, objects);
    }
    static bool LogValidationErrorMessage(const std::string& vuid, const std::string& command_name, const std::string& message,
                                          const std::vector<XrLoaderLogObjectInfo>& objects = {}) {
        return GetInstance().LogMessage(XR_LOADER_LOG_MESSAGE_SEVERITY_ERROR_BIT, XR_LOADER_LOG_MESSAGE_TYPE_SPECIFICATION_BIT,
                                        vuid, command_name, message, objects);
    }
    static bool LogValidationWarningMessage(const std::string& vuid, const std::string& command_name, const std::string& message,
                                            const std::vector<XrLoaderLogObjectInfo>& objects = {}) {
        return GetInstance().LogMessage(XR_LOADER_LOG_MESSAGE_SEVERITY_WARNING_BIT, XR_LOADER_LOG_MESSAGE_TYPE_SPECIFICATION_BIT,
                                        vuid, command_name, message, objects);
    }

    // Extension-specific logging functions
    bool LogDebugUtilsMessage(XrDebugUtilsMessageSeverityFlagsEXT message_severity, XrDebugUtilsMessageTypeFlagsEXT message_type,
                              const XrDebugUtilsMessengerCallbackDataEXT* callback_data);

   private:
    struct InternalSessionLabel {
        XrDebugUtilsLabelEXT debug_utils_label;
        std::string label_name;
        bool is_individual_label;
    };

    LoaderLogger();
    LoaderLogger(const LoaderLogger&) = delete;
    LoaderLogger& operator=(const LoaderLogger&) = delete;

    /// Retrieve labels for the given session, if any, and push them in reverse order on the vector.
    void LookUpSessionLabels(XrSession session, std::vector<XrDebugUtilsLabelEXT>& labels) const;

    void RemoveIndividualLabel(std::vector<InternalSessionLabel*>* label_vec);

    static std::unique_ptr<LoaderLogger> _instance;
    static std::once_flag _once_flag;

    // List of available recorder objects
    std::vector<std::unique_ptr<LoaderLogRecorder>> _recorders;

    ObjectInfoCollection _object_names;
    // Session labels
    std::unordered_map<XrSession, std::vector<InternalSessionLabel*>*> _session_labels;
};

// Utility functions for converting to/from XR_EXT_debug_utils values
XrLoaderLogMessageSeverityFlags DebugUtilsSeveritiesToLoaderLogMessageSeverities(
    XrDebugUtilsMessageSeverityFlagsEXT utils_severities);
XrDebugUtilsMessageSeverityFlagsEXT LoaderLogMessageSeveritiesToDebugUtilsMessageSeverities(
    XrLoaderLogMessageSeverityFlags log_severities);
XrLoaderLogMessageTypeFlagBits DebugUtilsMessageTypesToLoaderLogMessageTypes(XrDebugUtilsMessageTypeFlagsEXT utils_types);
XrDebugUtilsMessageTypeFlagsEXT LoaderLogMessageTypesToDebugUtilsMessageTypes(XrLoaderLogMessageTypeFlagBits log_types);
