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

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "xr_dependencies.h"
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include "hex_and_handles.h"
#include "extra_algorithms.h"
#include "loader_platform.hpp"
#include "platform_utils.hpp"
#include "loader_logger.hpp"
#include "loader_logger_recorders.hpp"

std::unique_ptr<LoaderLogger> LoaderLogger::_instance;
std::once_flag LoaderLogger::_once_flag;

std::string XrLoaderLogObjectInfo::ToString() const {
    std::ostringstream oss;
    oss << Uint64ToHexString(handle);
    if (!name.empty()) {
        oss << " (" << name << ")";
    }
    return oss.str();
}

void ObjectInfoCollection::AddObjectName(uint64_t object_handle, XrObjectType object_type, const std::string& object_name) {
    // If name is empty, we should erase it
    if (object_name.empty()) {
        vector_remove_if_and_erase(_object_info, [=](XrLoaderLogObjectInfo const& info) { return info.handle == object_handle; });
        return;
    }
    // Otherwise, add it or update the name

    XrLoaderLogObjectInfo new_obj = {object_handle, object_type};

    // If it already exists, update the name
    auto lookup_info = LookUpStoredObjectInfo(new_obj);
    if (lookup_info != nullptr) {
        lookup_info->name = object_name;
        return;
    }

    // It doesn't exist, so add a new info block
    new_obj.name = object_name;
    _object_info.push_back(new_obj);
}

XrLoaderLogObjectInfo const* ObjectInfoCollection::LookUpStoredObjectInfo(XrLoaderLogObjectInfo const& info) const {
    auto e = _object_info.end();
    auto it = std::find_if(_object_info.begin(), e, [&](XrLoaderLogObjectInfo const& stored) { return Equivalent(stored, info); });
    if (it != e) {
        return &(*it);
    }
    return nullptr;
}

XrLoaderLogObjectInfo* ObjectInfoCollection::LookUpStoredObjectInfo(XrLoaderLogObjectInfo const& info) {
    auto e = _object_info.end();
    auto it = std::find_if(_object_info.begin(), e, [&](XrLoaderLogObjectInfo const& stored) { return Equivalent(stored, info); });
    if (it != e) {
        return &(*it);
    }
    return nullptr;
}

bool ObjectInfoCollection::LookUpObjectName(XrDebugUtilsObjectNameInfoEXT& info) const {
    auto info_lookup = LookUpStoredObjectInfo(info.objectHandle, info.objectType);
    if (info_lookup != nullptr) {
        info.objectName = info_lookup->name.c_str();
        return true;
    }
    return false;
}

bool ObjectInfoCollection::LookUpObjectName(XrLoaderLogObjectInfo& info) const {
    auto info_lookup = LookUpStoredObjectInfo(info);
    if (info_lookup != nullptr) {
        info.name = info_lookup->name;
        return true;
    }
    return false;
}

// Utility functions for converting to/from XR_EXT_debug_utils values

XrLoaderLogMessageSeverityFlags DebugUtilsSeveritiesToLoaderLogMessageSeverities(
    XrDebugUtilsMessageSeverityFlagsEXT utils_severities) {
    XrLoaderLogMessageSeverityFlags log_severities = 0UL;
    if (utils_severities & XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        log_severities |= XR_LOADER_LOG_MESSAGE_SEVERITY_VERBOSE_BIT;
    }
    if (utils_severities & XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        log_severities |= XR_LOADER_LOG_MESSAGE_SEVERITY_INFO_BIT;
    }
    if (utils_severities & XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        log_severities |= XR_LOADER_LOG_MESSAGE_SEVERITY_WARNING_BIT;
    }
    if (utils_severities & XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        log_severities |= XR_LOADER_LOG_MESSAGE_SEVERITY_ERROR_BIT;
    }
    return log_severities;
}

XrDebugUtilsMessageSeverityFlagsEXT LoaderLogMessageSeveritiesToDebugUtilsMessageSeverities(
    XrLoaderLogMessageSeverityFlags log_severities) {
    XrDebugUtilsMessageSeverityFlagsEXT utils_severities = 0UL;
    if (log_severities & XR_LOADER_LOG_MESSAGE_SEVERITY_VERBOSE_BIT) {
        utils_severities |= XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    }
    if (log_severities & XR_LOADER_LOG_MESSAGE_SEVERITY_INFO_BIT) {
        utils_severities |= XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    }
    if (log_severities & XR_LOADER_LOG_MESSAGE_SEVERITY_WARNING_BIT) {
        utils_severities |= XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    }
    if (log_severities & XR_LOADER_LOG_MESSAGE_SEVERITY_ERROR_BIT) {
        utils_severities |= XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    }
    return utils_severities;
}

XrLoaderLogMessageTypeFlagBits DebugUtilsMessageTypesToLoaderLogMessageTypes(XrDebugUtilsMessageTypeFlagsEXT utils_types) {
    XrLoaderLogMessageTypeFlagBits log_types = 0UL;
    if (utils_types & XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
        log_types |= XR_LOADER_LOG_MESSAGE_TYPE_GENERAL_BIT;
    }
    if (utils_types & XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
        log_types |= XR_LOADER_LOG_MESSAGE_TYPE_SPECIFICATION_BIT;
    }
    if (utils_types & XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
        log_types |= XR_LOADER_LOG_MESSAGE_TYPE_PERFORMANCE_BIT;
    }
    return log_types;
}

XrDebugUtilsMessageTypeFlagsEXT LoaderLogMessageTypesToDebugUtilsMessageTypes(XrLoaderLogMessageTypeFlagBits log_types) {
    XrDebugUtilsMessageTypeFlagsEXT utils_types = 0UL;
    if (log_types & XR_LOADER_LOG_MESSAGE_TYPE_GENERAL_BIT) {
        utils_types |= XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    }
    if (log_types & XR_LOADER_LOG_MESSAGE_TYPE_SPECIFICATION_BIT) {
        utils_types |= XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    }
    if (log_types & XR_LOADER_LOG_MESSAGE_TYPE_PERFORMANCE_BIT) {
        utils_types |= XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    }
    return utils_types;
}

LoaderLogger::LoaderLogger() {
    // Add an error logger by default so that we at least get errors out to std::cerr.
    AddLogRecorder(MakeStdErrLoaderLogRecorder(nullptr));

    // If the environment variable to enable loader debugging is set, then enable the
    // appropriate logging out to std::cout.
    char* loader_debug = PlatformUtilsGetSecureEnv("XR_LOADER_DEBUG");
    if (nullptr != loader_debug) {
        std::string debug_string = loader_debug;
        PlatformUtilsFreeEnv(loader_debug);
        XrLoaderLogMessageSeverityFlags debug_flags = {};
        if (debug_string == "error") {
            debug_flags = XR_LOADER_LOG_MESSAGE_SEVERITY_ERROR_BIT;
        } else if (debug_string == "warn") {
            debug_flags = XR_LOADER_LOG_MESSAGE_SEVERITY_ERROR_BIT | XR_LOADER_LOG_MESSAGE_SEVERITY_WARNING_BIT;
        } else if (debug_string == "info") {
            debug_flags = XR_LOADER_LOG_MESSAGE_SEVERITY_ERROR_BIT | XR_LOADER_LOG_MESSAGE_SEVERITY_WARNING_BIT |
                          XR_LOADER_LOG_MESSAGE_SEVERITY_INFO_BIT;
        } else if (debug_string == "all" || debug_string == "verbose") {
            debug_flags = XR_LOADER_LOG_MESSAGE_SEVERITY_ERROR_BIT | XR_LOADER_LOG_MESSAGE_SEVERITY_WARNING_BIT |
                          XR_LOADER_LOG_MESSAGE_SEVERITY_INFO_BIT | XR_LOADER_LOG_MESSAGE_SEVERITY_VERBOSE_BIT;
        }
        AddLogRecorder(MakeStdOutLoaderLogRecorder(nullptr, debug_flags));
    }
}

void LoaderLogger::AddLogRecorder(std::unique_ptr<LoaderLogRecorder>&& recorder) { _recorders.push_back(std::move(recorder)); }

void LoaderLogger::RemoveLogRecorder(uint64_t unique_id) {
    vector_remove_if_and_erase(
        _recorders, [=](std::unique_ptr<LoaderLogRecorder> const& recorder) { return recorder->UniqueId() == unique_id; });
}

bool LoaderLogger::LogMessage(XrLoaderLogMessageSeverityFlagBits message_severity, XrLoaderLogMessageTypeFlags message_type,
                              const std::string& message_id, const std::string& command_name, const std::string& message,
                              const std::vector<XrLoaderLogObjectInfo>& objects) {
    XrLoaderLogMessengerCallbackData callback_data = {};
    callback_data.message_id = message_id.c_str();
    callback_data.command_name = command_name.c_str();
    callback_data.message = message.c_str();

    std::vector<XrDebugUtilsLabelEXT> labels;

    // Copy objects into a vector we can modify and will keep around past the callback.
    std::vector<XrLoaderLogObjectInfo> object_vector = objects;
    for (auto& obj : object_vector) {
        // Check for any names that have been associated with the objects and set them up here
        _object_names.LookUpObjectName(obj);
        // If this is a session, see if there are any labels associated with it for us to add
        // to the callback content.
        if (XR_OBJECT_TYPE_SESSION == obj.type) {
            LookUpSessionLabels(obj.GetTypedHandle<XrSession>(), labels);
        }
    }
    callback_data.objects = object_vector.empty() ? nullptr : object_vector.data();
    callback_data.object_count = static_cast<uint8_t>(object_vector.size());

    callback_data.session_labels = labels.empty() ? nullptr : labels.data();
    callback_data.session_labels_count = static_cast<uint8_t>(labels.size());

    bool exit_app = false;
    for (std::unique_ptr<LoaderLogRecorder>& recorder : _recorders) {
        if ((recorder->MessageSeverities() & message_severity) == message_severity &&
            (recorder->MessageTypes() & message_type) == message_type) {
            exit_app |= recorder->LogMessage(message_severity, message_type, &callback_data);
        }
    }
    return exit_app;
}

void LoaderLogger::LookUpSessionLabels(XrSession session, std::vector<XrDebugUtilsLabelEXT>& labels) const {
    auto session_label_iterator = _session_labels.find(session);
    if (session_label_iterator != _session_labels.end()) {
        auto& internalSessionLabels = *session_label_iterator->second;
        // Copy the debug utils labels in reverse order in the the labels vector.
        std::transform(internalSessionLabels.rbegin(), internalSessionLabels.rend(), std::back_inserter(labels),
                       [](InternalSessionLabelPtr const& label) { return label->debug_utils_label; });
    }
}

// Extension-specific logging functions
bool LoaderLogger::LogDebugUtilsMessage(XrDebugUtilsMessageSeverityFlagsEXT message_severity,
                                        XrDebugUtilsMessageTypeFlagsEXT message_type,
                                        const XrDebugUtilsMessengerCallbackDataEXT* callback_data) {
    bool exit_app = false;
    XrLoaderLogMessageSeverityFlags log_message_severity = DebugUtilsSeveritiesToLoaderLogMessageSeverities(message_severity);
    XrLoaderLogMessageTypeFlags log_message_type = DebugUtilsMessageTypesToLoaderLogMessageTypes(message_type);

    bool obj_name_found = false;
    std::vector<XrDebugUtilsLabelEXT> labels;
    if (!_object_names.Empty() && callback_data->objectCount > 0) {
        for (uint32_t obj = 0; obj < callback_data->objectCount; ++obj) {
            auto& current_obj = callback_data->objects[obj];
            auto stored_info = _object_names.LookUpStoredObjectInfo(current_obj.objectHandle, current_obj.objectType);
            if (stored_info != nullptr) {
                obj_name_found = true;
            }

            // If this is a session, see if there are any labels associated with it for us to add
            // to the callback content.
            if (XR_OBJECT_TYPE_SESSION == current_obj.objectType) {
                XrSession session = TreatIntegerAsHandle<XrSession>(current_obj.objectHandle);
                LookUpSessionLabels(session, labels);
            }
        }
    }
    // Use unmodified ones by default.
    XrDebugUtilsMessengerCallbackDataEXT const* callback_data_to_use = callback_data;

    XrDebugUtilsMessengerCallbackDataEXT new_callback_data = *callback_data;
    std::vector<XrDebugUtilsObjectNameInfoEXT> new_objects;

    // If a name or a label has been found, we should update it in a new version of the callback
    if (obj_name_found || !labels.empty()) {
        // Copy objects
        new_objects =
            std::vector<XrDebugUtilsObjectNameInfoEXT>(callback_data->objects, callback_data->objects + callback_data->objectCount);
        for (auto& obj : new_objects) {
            // Check for any names that have been associated with the objects and set them up here
            _object_names.LookUpObjectName(obj);
        }
        new_callback_data.objects = new_objects.data();
        new_callback_data.sessionLabelCount = static_cast<uint32_t>(labels.size());
        new_callback_data.sessionLabels = labels.empty() ? nullptr : labels.data();
        callback_data_to_use = &new_callback_data;
    }

    // Loop through the recorders
    for (std::unique_ptr<LoaderLogRecorder>& recorder : _recorders) {
        // Only send the message if it's a debug utils recorder and of the type the recorder cares about.
        if (recorder->Type() != XR_LOADER_LOG_DEBUG_UTILS ||
            (recorder->MessageSeverities() & log_message_severity) != log_message_severity ||
            (recorder->MessageTypes() & log_message_type) != log_message_type) {
            continue;
        }

        exit_app |= recorder->LogDebugUtilsMessage(message_severity, message_type, callback_data_to_use);
    }
    return exit_app;
}

void LoaderLogger::AddObjectName(uint64_t object_handle, XrObjectType object_type, const std::string& object_name) {
    _object_names.AddObjectName(object_handle, object_type, object_name);
}

// We always want to remove the old individual label before we do anything else.
// So, do that in it's own method
void LoaderLogger::RemoveIndividualLabel(InternalSessionLabelList& label_vec) {
    if (!label_vec.empty() && label_vec.back()->is_individual_label) {
        label_vec.pop_back();
    }
}

InternalSessionLabelList* LoaderLogger::GetSessionLabelList(XrSession session) {
    auto session_label_iterator = _session_labels.find(session);
    if (session_label_iterator == _session_labels.end()) {
        return nullptr;
    }
    return session_label_iterator->second.get();
}

InternalSessionLabelList& LoaderLogger::GetOrCreateSessionLabelList(XrSession session) {
    InternalSessionLabelList* vec_ptr = GetSessionLabelList(session);
    if (vec_ptr == nullptr) {
        std::unique_ptr<InternalSessionLabelList> vec(new InternalSessionLabelList);
        vec_ptr = vec.get();
        _session_labels[session] = std::move(vec);
    }
    return *vec_ptr;
}

void LoaderLogger::BeginLabelRegion(XrSession session, const XrDebugUtilsLabelEXT* label_info) {
    auto& vec = GetOrCreateSessionLabelList(session);

    // Individual labels do not stay around in the transition into a new label region
    RemoveIndividualLabel(vec);

    // Start the new label region
    InternalSessionLabelPtr new_session_label(new InternalSessionLabel);
    new_session_label->label_name = label_info->labelName;
    new_session_label->debug_utils_label = *label_info;
    new_session_label->debug_utils_label.labelName = new_session_label->label_name.c_str();
    new_session_label->is_individual_label = false;
    vec.emplace_back(std::move(new_session_label));
}

void LoaderLogger::EndLabelRegion(XrSession session) {
    InternalSessionLabelList* vec_ptr = GetSessionLabelList(session);
    if (vec_ptr == nullptr) {
        return;
    }

    // Individual labels do not stay around in the transition out of label region
    RemoveIndividualLabel(*vec_ptr);

    // Remove the last label region
    if (!vec_ptr->empty()) {
        vec_ptr->pop_back();
    }
}

void LoaderLogger::InsertLabel(XrSession session, const XrDebugUtilsLabelEXT* label_info) {
    //! @todo only difference from BeginLabelRegion is value of is_individual_label
    auto& vec = GetOrCreateSessionLabelList(session);

    // Remove any individual layer that might already be there
    RemoveIndividualLabel(vec);

    // Insert a new individual label
    InternalSessionLabelPtr new_session_label(new InternalSessionLabel);
    new_session_label->label_name = label_info->labelName;
    new_session_label->debug_utils_label = *label_info;
    new_session_label->debug_utils_label.labelName = new_session_label->label_name.c_str();
    new_session_label->is_individual_label = true;
    vec.emplace_back(std::move(new_session_label));
}

// Called during xrDestroySession.  We need to delete all session related labels.
void LoaderLogger::DeleteSessionLabels(XrSession session) {
    InternalSessionLabelList* vec_ptr = GetSessionLabelList(session);
    if (vec_ptr == nullptr) {
        return;
    }
    _session_labels.erase(session);
}
