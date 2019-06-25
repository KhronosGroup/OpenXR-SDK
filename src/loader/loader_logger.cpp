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

#include "loader_platform.hpp"
#include "platform_utils.hpp"
#include "loader_logger.hpp"

std::unique_ptr<LoaderLogger> LoaderLogger::_instance;
std::once_flag LoaderLogger::_once_flag;

// Standard Error logger, always on for now
StdErrLoaderLogRecorder::StdErrLoaderLogRecorder(void* user_data)
    : LoaderLogRecorder(XR_LOADER_LOG_STDERR, user_data, XR_LOADER_LOG_MESSAGE_SEVERITY_ERROR_BIT, 0xFFFFFFFFUL) {
    // Automatically start
    Start();
}

std::string Uint64ToHexString(uint64_t val) {
    std::ostringstream oss;
    oss << "0x";
    oss << std::hex << std::setw(16) << std::setfill('0') << val;
    return oss.str();
}

std::string XrLoaderLogObjectInfo::ToString() const {
    std::ostringstream oss;
    oss << Uint64ToHexString(handle);
    if (!name.empty()) {
        oss << " (" << name << ")";
    }
    return oss.str();
}

bool StdErrLoaderLogRecorder::LogMessage(XrLoaderLogMessageSeverityFlagBits message_severity,
                                         XrLoaderLogMessageTypeFlags message_type,
                                         const XrLoaderLogMessengerCallbackData* callback_data) {
    if (_active && XR_LOADER_LOG_MESSAGE_SEVERITY_ERROR_BIT <= message_severity) {
        std::cerr << "Error [";
        switch (message_type) {
            case XR_LOADER_LOG_MESSAGE_TYPE_GENERAL_BIT:
                std::cerr << "GENERAL";
                break;
            case XR_LOADER_LOG_MESSAGE_TYPE_SPECIFICATION_BIT:
                std::cerr << "SPEC";
                break;
            case XR_LOADER_LOG_MESSAGE_TYPE_PERFORMANCE_BIT:
                std::cerr << "PERF";
                break;
            default:
                std::cerr << "UNKNOWN";
                break;
        }
        std::cerr << " | " << callback_data->command_name << " | " << callback_data->message_id << "] : " << callback_data->message
                  << std::endl;

        for (uint32_t obj = 0; obj < callback_data->object_count; ++obj) {
            std::cerr << "    Object[" << obj << "] = " << callback_data->objects[obj].ToString();
            std::cerr << std::endl;
        }
        for (uint32_t label = 0; label < callback_data->session_labels_count; ++label) {
            std::cerr << "    SessionLabel[" << std::to_string(label) << "] = " << callback_data->session_labels[label].labelName;
            std::cerr << std::endl;
        }
    }

    // Return of "true" means that we should exit the application after the logged message.  We
    // don't want to do that for our internal logging.  Only let a user return true.
    return false;
}

// Standard Output logger used with XR_LOADER_DEBUG
StdOutLoaderLogRecorder::StdOutLoaderLogRecorder(void* user_data, XrLoaderLogMessageSeverityFlags flags)
    : LoaderLogRecorder(XR_LOADER_LOG_STDOUT, user_data, flags, 0xFFFFFFFFUL) {
    // Automatically start
    Start();
}

bool StdOutLoaderLogRecorder::LogMessage(XrLoaderLogMessageSeverityFlagBits message_severity,
                                         XrLoaderLogMessageTypeFlags message_type,
                                         const XrLoaderLogMessengerCallbackData* callback_data) {
    if (_active && 0 != (_message_severities & message_severity) && 0 != (_message_types & message_type)) {
        if (XR_LOADER_LOG_MESSAGE_SEVERITY_INFO_BIT > message_severity) {
            std::cout << "Verbose [";
        } else if (XR_LOADER_LOG_MESSAGE_SEVERITY_WARNING_BIT > message_severity) {
            std::cout << "Info [";
        } else if (XR_LOADER_LOG_MESSAGE_SEVERITY_ERROR_BIT > message_severity) {
            std::cout << "Warning [";
        } else {
            std::cout << "Error [";
        }
        switch (message_type) {
            case XR_LOADER_LOG_MESSAGE_TYPE_GENERAL_BIT:
                std::cout << "GENERAL";
                break;
            case XR_LOADER_LOG_MESSAGE_TYPE_SPECIFICATION_BIT:
                std::cout << "SPEC";
                break;
            case XR_LOADER_LOG_MESSAGE_TYPE_PERFORMANCE_BIT:
                std::cout << "PERF";
                break;
            default:
                std::cout << "UNKNOWN";
                break;
        }
        std::cout << " | " << callback_data->command_name << " | " << callback_data->message_id << "] : " << callback_data->message
                  << std::endl;

        for (uint32_t obj = 0; obj < callback_data->object_count; ++obj) {
            std::cout << "    Object[" << std::to_string(obj) << "] = " << std::to_string(callback_data->objects[obj].handle);
            if (!callback_data->objects[obj].name.empty()) {
                std::cout << " (" << callback_data->objects[obj].name << ")";
            }
            std::cout << std::endl;
        }
        for (uint32_t label = 0; label < callback_data->session_labels_count; ++label) {
            std::cout << "    SessionLabel[" << std::to_string(label) << "] = " << callback_data->session_labels[label].labelName;
            std::cout << std::endl;
        }
    }

    // Return of "true" means that we should exit the application after the logged message.  We
    // don't want to do that for our internal logging.  Only let a user return true.
    return false;
}

void ObjectInfoCollection::AddObjectName(uint64_t object_handle, XrObjectType object_type, const std::string& object_name) {
    // If name is empty, we should erase it
    if (object_name.empty()) {
        auto new_end = std::remove_if(_object_info.begin(), _object_info.end(),
                                      [=](XrLoaderLogObjectInfo const& info) { return info.handle == object_handle; });
        _object_info.erase(new_end);
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

// A logger associated with the XR_EXT_debug_utils extension

DebugUtilsLogRecorder::DebugUtilsLogRecorder(const XrDebugUtilsMessengerCreateInfoEXT* create_info,
                                             XrDebugUtilsMessengerEXT debug_messenger)
    : LoaderLogRecorder(XR_LOADER_LOG_DEBUG_UTILS, static_cast<void*>(create_info->userData),
                        DebugUtilsSeveritiesToLoaderLogMessageSeverities(create_info->messageSeverities),
                        DebugUtilsMessageTypesToLoaderLogMessageTypes(create_info->messageTypes)),
      _user_callback(create_info->userCallback) {
    // Use the debug messenger value to uniquely identify this logger with that messenger
    _unique_id = reinterpret_cast<uint64_t&>(debug_messenger);
    Start();
}

// Extension-specific logging functions
bool DebugUtilsLogRecorder::LogMessage(XrLoaderLogMessageSeverityFlagBits message_severity,
                                       XrLoaderLogMessageTypeFlags message_type,
                                       const XrLoaderLogMessengerCallbackData* callback_data) {
    bool should_exit = false;
    if (_active && 0 != (_message_severities & message_severity) && 0 != (_message_types & message_type)) {
        XrDebugUtilsMessageSeverityFlagsEXT utils_severity = DebugUtilsSeveritiesToLoaderLogMessageSeverities(message_severity);
        XrDebugUtilsMessageTypeFlagsEXT utils_type = LoaderLogMessageTypesToDebugUtilsMessageTypes(message_type);

        // Convert the loader log message into the debug utils log message information
        XrDebugUtilsMessengerCallbackDataEXT utils_callback_data = {};
        utils_callback_data.type = XR_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT;
        utils_callback_data.messageId = callback_data->message_id;
        utils_callback_data.functionName = callback_data->command_name;
        utils_callback_data.message = callback_data->message;
        std::vector<XrDebugUtilsObjectNameInfoEXT> utils_objects;
        utils_objects.resize(callback_data->object_count);
        for (uint8_t object = 0; object < callback_data->object_count; ++object) {
            utils_objects[object].type = XR_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            utils_objects[object].next = nullptr;
            utils_objects[object].objectHandle = callback_data->objects[object].handle;
            utils_objects[object].objectType = callback_data->objects[object].type;
            utils_objects[object].objectName = callback_data->objects[object].name.c_str();
        }
        utils_callback_data.objectCount = callback_data->object_count;
        utils_callback_data.objects = utils_objects.data();
        utils_callback_data.sessionLabelCount = callback_data->session_labels_count;
        utils_callback_data.sessionLabels = callback_data->session_labels;

        // Call the user callback with the appropriate info
        // Return of "true" means that we should exit the application after the logged message.
        should_exit = (_user_callback(utils_severity, utils_type, &utils_callback_data, _user_data) == XR_TRUE);
    }

    return should_exit;
}

bool DebugUtilsLogRecorder::LogDebugUtilsMessage(XrDebugUtilsMessageSeverityFlagsEXT message_severity,
                                                 XrDebugUtilsMessageTypeFlagsEXT message_type,
                                                 const XrDebugUtilsMessengerCallbackDataEXT* callback_data) {
    // Call the user callback with the appropriate info
    // Return of "true" means that we should exit the application after the logged message.
    return (_user_callback(message_severity, message_type, callback_data, _user_data) == XR_TRUE);
}

LoaderLogger::LoaderLogger() {
    // Add an error logger by default so that we at least get errors out to std::cerr.
    std::unique_ptr<LoaderLogRecorder> base_recorder(new StdErrLoaderLogRecorder(nullptr));
    AddLogRecorder(base_recorder);

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
        std::unique_ptr<LoaderLogRecorder> debug_recorder(new StdOutLoaderLogRecorder(nullptr, debug_flags));
        AddLogRecorder(debug_recorder);
    }
}

void LoaderLogger::AddLogRecorder(std::unique_ptr<LoaderLogRecorder>& recorder) { _recorders.push_back(std::move(recorder)); }

void LoaderLogger::RemoveLogRecorder(uint64_t unique_id) {
    for (uint32_t index = 0; index < _recorders.size(); ++index) {
        if (_recorders[index]->UniqueId() == unique_id) {
            _recorders.erase(_recorders.begin() + index);
            break;
        }
    }
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
                       [](InternalSessionLabel* label) { return label->debug_utils_label; });
    }
}

// Extension-specific logging functions
bool LoaderLogger::LogDebugUtilsMessage(XrDebugUtilsMessageSeverityFlagsEXT message_severity,
                                        XrDebugUtilsMessageTypeFlagsEXT message_type,
                                        const XrDebugUtilsMessengerCallbackDataEXT* callback_data) {
    bool exit_app = false;
    for (std::unique_ptr<LoaderLogRecorder>& recorder : _recorders) {
        XrLoaderLogMessageSeverityFlags log_message_severity = DebugUtilsSeveritiesToLoaderLogMessageSeverities(message_severity);
        XrLoaderLogMessageTypeFlags log_message_type = DebugUtilsMessageTypesToLoaderLogMessageTypes(message_type);
        bool dumped = false;
        // Only send the message if it's a debug utils recorder and of the type the recorder cares about.
        if (recorder->Type() == XR_LOADER_LOG_DEBUG_UTILS &&
            (recorder->MessageSeverities() & log_message_severity) == log_message_severity &&
            (recorder->MessageTypes() & log_message_type) == log_message_type) {
            DebugUtilsLogRecorder* debug_utils_recorder = reinterpret_cast<DebugUtilsLogRecorder*>(recorder.get());
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
                    if (XR_OBJECT_TYPE_SESSION == callback_data->objects[obj].objectType) {
                        XrSession session = reinterpret_cast<XrSession&>(callback_data->objects[obj].objectHandle);
                        LookUpSessionLabels(session, labels);
                    }
                }
            }
            XrDebugUtilsMessengerCallbackDataEXT new_callback_data = *callback_data;
            std::vector<XrDebugUtilsObjectNameInfoEXT> new_objects;

            // If a name or a label has been found, we should update it in a new version of the callback
            if (obj_name_found || !labels.empty()) {
                // Copy objects
                new_objects = std::vector<XrDebugUtilsObjectNameInfoEXT>(callback_data->objects,
                                                                         callback_data->objects + callback_data->objectCount);
                for (auto& obj : new_objects) {
                    // Check for any names that have been associated with the objects and set them up here
                    _object_names.LookUpObjectName(obj);
                }
                new_callback_data.objects = new_objects.data();
                new_callback_data.sessionLabelCount = static_cast<uint32_t>(labels.size());
                new_callback_data.sessionLabels = labels.empty() ? nullptr : labels.data();
                exit_app |= debug_utils_recorder->LogDebugUtilsMessage(message_severity, message_type, &new_callback_data);
                dumped = true;
            }

            if (!dumped) {
                exit_app |= debug_utils_recorder->LogDebugUtilsMessage(message_severity, message_type, callback_data);
            }
        }
    }
    return exit_app;
}

void LoaderLogger::AddObjectName(uint64_t object_handle, XrObjectType object_type, const std::string& object_name) {
    _object_names.AddObjectName(object_handle, object_type, object_name);
}

// We always want to remove the old individual label before we do anything else.
// So, do that in it's own method
void LoaderLogger::RemoveIndividualLabel(std::vector<InternalSessionLabel*>* label_vec) {
    if (!label_vec->empty()) {
        InternalSessionLabel* cur_label = label_vec->back();
        if (cur_label->is_individual_label) {
            label_vec->pop_back();
            delete cur_label;
        }
    }
}

void LoaderLogger::BeginLabelRegion(XrSession session, const XrDebugUtilsLabelEXT* label_info) {
    std::vector<InternalSessionLabel*>* vec_ptr = nullptr;
    auto session_label_iterator = _session_labels.find(session);
    if (session_label_iterator == _session_labels.end()) {
        vec_ptr = new std::vector<InternalSessionLabel*>;
        _session_labels[session] = vec_ptr;
    } else {
        vec_ptr = session_label_iterator->second;
    }

    // Individual labels do not stay around in the transition into a new label region
    RemoveIndividualLabel(vec_ptr);

    // Start the new label region
    InternalSessionLabel* new_session_label = new InternalSessionLabel;
    new_session_label->label_name = label_info->labelName;
    new_session_label->debug_utils_label = *label_info;
    new_session_label->debug_utils_label.labelName = new_session_label->label_name.c_str();
    new_session_label->is_individual_label = false;
    vec_ptr->push_back(new_session_label);
}

void LoaderLogger::EndLabelRegion(XrSession session) {
    auto session_label_iterator = _session_labels.find(session);
    if (session_label_iterator == _session_labels.end()) {
        return;
    }

    std::vector<InternalSessionLabel*>* vec_ptr = session_label_iterator->second;

    // Individual labels do not stay around in the transition out of label region
    RemoveIndividualLabel(vec_ptr);

    // Remove the last label region
    if (!vec_ptr->empty()) {
        InternalSessionLabel* cur_label = vec_ptr->back();
        vec_ptr->pop_back();
        delete cur_label;
    }
}

void LoaderLogger::InsertLabel(XrSession session, const XrDebugUtilsLabelEXT* label_info) {
    std::vector<InternalSessionLabel*>* vec_ptr = nullptr;
    auto session_label_iterator = _session_labels.find(session);
    if (session_label_iterator == _session_labels.end()) {
        vec_ptr = new std::vector<InternalSessionLabel*>;
        _session_labels[session] = vec_ptr;
    } else {
        vec_ptr = session_label_iterator->second;
    }

    // Remove any individual layer that might already be there
    RemoveIndividualLabel(vec_ptr);

    // Insert a new individual label
    InternalSessionLabel* new_session_label = new InternalSessionLabel;
    new_session_label->label_name = label_info->labelName;
    new_session_label->debug_utils_label = *label_info;
    new_session_label->debug_utils_label.labelName = new_session_label->label_name.c_str();
    new_session_label->is_individual_label = true;
    vec_ptr->push_back(new_session_label);
}

// Called during xrDestroySession.  We need to delete all session related labels.
void LoaderLogger::DeleteSessionLabels(XrSession session) {
    std::vector<InternalSessionLabel*>* vec_ptr = nullptr;
    auto session_label_iterator = _session_labels.find(session);
    if (session_label_iterator == _session_labels.end()) {
        return;
    }
    vec_ptr = session_label_iterator->second;
    while (!vec_ptr->empty()) {
        delete vec_ptr->back();
        vec_ptr->pop_back();
    }
    delete vec_ptr;
    _session_labels.erase(session);
}
