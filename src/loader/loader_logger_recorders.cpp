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

#include "loader_logger_recorders.hpp"

#include "hex_and_handles.h"
#include "loader_logger.hpp"

#include <openxr/openxr.h>

#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#endif

// Anonymous namespace to keep these types private
namespace {
void OutputMessageToStream(std::ostream& os, XrLoaderLogMessageSeverityFlagBits message_severity,
                           XrLoaderLogMessageTypeFlags message_type, const XrLoaderLogMessengerCallbackData* callback_data) {
    if (XR_LOADER_LOG_MESSAGE_SEVERITY_INFO_BIT > message_severity) {
        os << "Verbose [";
    } else if (XR_LOADER_LOG_MESSAGE_SEVERITY_WARNING_BIT > message_severity) {
        os << "Info [";
    } else if (XR_LOADER_LOG_MESSAGE_SEVERITY_ERROR_BIT > message_severity) {
        os << "Warning [";
    } else {
        os << "Error [";
    }
    switch (message_type) {
        case XR_LOADER_LOG_MESSAGE_TYPE_GENERAL_BIT:
            os << "GENERAL";
            break;
        case XR_LOADER_LOG_MESSAGE_TYPE_SPECIFICATION_BIT:
            os << "SPEC";
            break;
        case XR_LOADER_LOG_MESSAGE_TYPE_PERFORMANCE_BIT:
            os << "PERF";
            break;
        default:
            os << "UNKNOWN";
            break;
    }
    os << " | " << callback_data->command_name << " | " << callback_data->message_id << "] : " << callback_data->message
       << std::endl;

    for (uint32_t obj = 0; obj < callback_data->object_count; ++obj) {
        os << "    Object[" << obj << "] = " << callback_data->objects[obj].ToString();
        os << std::endl;
    }
    for (uint32_t label = 0; label < callback_data->session_labels_count; ++label) {
        os << "    SessionLabel[" << std::to_string(label) << "] = " << callback_data->session_labels[label].labelName;
        os << std::endl;
    }
}

// With std::cerr: Standard Error logger, always on for now
// With std::cout: Standard Output logger used with XR_LOADER_DEBUG
class OstreamLoaderLogRecorder : public LoaderLogRecorder {
   public:
    OstreamLoaderLogRecorder(std::ostream& os, void* user_data, XrLoaderLogMessageSeverityFlags flags);

    bool LogMessage(XrLoaderLogMessageSeverityFlagBits message_severity, XrLoaderLogMessageTypeFlags message_type,
                    const XrLoaderLogMessengerCallbackData* callback_data) override;

   private:
    std::ostream& os_;
};

// Debug Utils logger used with XR_EXT_debug_utils
class DebugUtilsLogRecorder : public LoaderLogRecorder {
   public:
    DebugUtilsLogRecorder(const XrDebugUtilsMessengerCreateInfoEXT* create_info, XrDebugUtilsMessengerEXT debug_messenger);

    bool LogMessage(XrLoaderLogMessageSeverityFlagBits message_severity, XrLoaderLogMessageTypeFlags message_type,
                    const XrLoaderLogMessengerCallbackData* callback_data) override;

    // Extension-specific logging functions
    bool LogDebugUtilsMessage(XrDebugUtilsMessageSeverityFlagsEXT message_severity, XrDebugUtilsMessageTypeFlagsEXT message_type,
                              const XrDebugUtilsMessengerCallbackDataEXT* callback_data) override;

   private:
    PFN_xrDebugUtilsMessengerCallbackEXT _user_callback;
};

#ifdef _WIN32
// Output to debugger
class DebuggerLoaderLogRecorder : public LoaderLogRecorder {
   public:
    DebuggerLoaderLogRecorder(void* user_data, XrLoaderLogMessageSeverityFlags flags);

    bool LogMessage(XrLoaderLogMessageSeverityFlagBits message_severity, XrLoaderLogMessageTypeFlags message_type,
                    const XrLoaderLogMessengerCallbackData* callback_data) override;
};
#endif

// Unified stdout/stderr logger
OstreamLoaderLogRecorder::OstreamLoaderLogRecorder(std::ostream& os, void* user_data, XrLoaderLogMessageSeverityFlags flags)
    : LoaderLogRecorder(XR_LOADER_LOG_STDOUT, user_data, flags, 0xFFFFFFFFUL), os_(os) {
    // Automatically start
    Start();
}

bool OstreamLoaderLogRecorder::LogMessage(XrLoaderLogMessageSeverityFlagBits message_severity,
                                          XrLoaderLogMessageTypeFlags message_type,
                                          const XrLoaderLogMessengerCallbackData* callback_data) {
    if (_active && 0 != (_message_severities & message_severity) && 0 != (_message_types & message_type)) {
        OutputMessageToStream(os_, message_severity, message_type, callback_data);
    }

    // Return of "true" means that we should exit the application after the logged message.  We
    // don't want to do that for our internal logging.  Only let a user return true.
    return false;
}

// A logger associated with the XR_EXT_debug_utils extension

DebugUtilsLogRecorder::DebugUtilsLogRecorder(const XrDebugUtilsMessengerCreateInfoEXT* create_info,
                                             XrDebugUtilsMessengerEXT debug_messenger)
    : LoaderLogRecorder(XR_LOADER_LOG_DEBUG_UTILS, static_cast<void*>(create_info->userData),
                        DebugUtilsSeveritiesToLoaderLogMessageSeverities(create_info->messageSeverities),
                        DebugUtilsMessageTypesToLoaderLogMessageTypes(create_info->messageTypes)),
      _user_callback(create_info->userCallback) {
    // Use the debug messenger value to uniquely identify this logger with that messenger
    _unique_id = MakeHandleGeneric(debug_messenger);
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

#ifdef _WIN32
// Unified stdout/stderr logger
DebuggerLoaderLogRecorder::DebuggerLoaderLogRecorder(void* user_data, XrLoaderLogMessageSeverityFlags flags)
    : LoaderLogRecorder(XR_LOADER_LOG_DEBUGGER, user_data, flags, 0xFFFFFFFFUL) {
    // Automatically start
    Start();
}

bool DebuggerLoaderLogRecorder::LogMessage(XrLoaderLogMessageSeverityFlagBits message_severity,
                                           XrLoaderLogMessageTypeFlags message_type,
                                           const XrLoaderLogMessengerCallbackData* callback_data) {
    if (_active && 0 != (_message_severities & message_severity) && 0 != (_message_types & message_type)) {
        std::stringstream ss;
        OutputMessageToStream(ss, message_severity, message_type, callback_data);

        OutputDebugStringA(ss.str().c_str());
    }

    // Return of "true" means that we should exit the application after the logged message.  We
    // don't want to do that for our internal logging.  Only let a user return true.
    return false;
}
#endif
}  // namespace

std::unique_ptr<LoaderLogRecorder> MakeStdOutLoaderLogRecorder(void* user_data, XrLoaderLogMessageSeverityFlags flags) {
    std::unique_ptr<LoaderLogRecorder> recorder(new OstreamLoaderLogRecorder(std::cout, user_data, flags));
    return recorder;
}

std::unique_ptr<LoaderLogRecorder> MakeStdErrLoaderLogRecorder(void* user_data) {
    std::unique_ptr<LoaderLogRecorder> recorder(
        new OstreamLoaderLogRecorder(std::cerr, user_data, XR_LOADER_LOG_MESSAGE_SEVERITY_ERROR_BIT));
    return recorder;
}

std::unique_ptr<LoaderLogRecorder> MakeDebugUtilsLoaderLogRecorder(const XrDebugUtilsMessengerCreateInfoEXT* create_info,
                                                                   XrDebugUtilsMessengerEXT debug_messenger) {
    std::unique_ptr<LoaderLogRecorder> recorder(new DebugUtilsLogRecorder(create_info, debug_messenger));
    return recorder;
}

#ifdef _WIN32
std::unique_ptr<LoaderLogRecorder> MakeDebuggerLoaderLogRecorder(void* user_data) {
    std::unique_ptr<LoaderLogRecorder> recorder(new DebuggerLoaderLogRecorder(user_data, XR_LOADER_LOG_MESSAGE_SEVERITY_ERROR_BIT));
    return recorder;
}
#endif
