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

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>
#include <mutex>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <vector>

#include "xr_generated_core_validation.hpp"
#include "loader_interfaces.h"
#include "platform_utils.hpp"
#include "xr_generated_dispatch_table.h"

#if defined(__GNUC__) && __GNUC__ >= 4
#define LAYER_EXPORT __attribute__((visibility("default")))
#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x590)
#define LAYER_EXPORT __attribute__((visibility("default")))
#else
#define LAYER_EXPORT
#endif

// Log recording information
enum CoreValidationRecordType {
    RECORD_NONE = 0,
    RECORD_TEXT_COUT,
    RECORD_TEXT_FILE,
    RECORD_HTML_FILE,
};

struct CoreValidationRecordInfo {
    bool initialized;
    CoreValidationRecordType type;
    std::string file_name;
};

static CoreValidationRecordInfo g_record_info = {};
static std::mutex g_record_mutex = {};

// HTML utilities
bool CoreValidationWriteHtmlHeader(void) {
    try {
        std::unique_lock<std::mutex> mlock(g_record_mutex);
        std::ofstream html_file;
        html_file.open(g_record_info.file_name, std::ios::out);
        html_file
            << "<!doctype html>\n"
               "<html>\n"
               "    <head>\n"
               "        <title>OpenXR Core Validation</title>\n"
               "        <style type='text/css'>\n"
               "        html {\n"
               "            background-color: #0b1e48;\n"
               "            background-image: url('https://vulkan.lunarg.com/img/bg-starfield.jpg');\n"
               "            background-position: center;\n"
               "            -webkit-background-size: cover;\n"
               "            -moz-background-size: cover;\n"
               "            -o-background-size: cover;\n"
               "            background-size: cover;\n"
               "            background-attachment: fixed;\n"
               "            background-repeat: no-repeat;\n"
               "            height: 100%;\n"
               "        }\n"
               "        #header {\n"
               "            z-index: -1;\n"
               "        }\n"
               "        #header>img {\n"
               "            position: absolute;\n"
               "            width: 160px;\n"
               "            margin-left: -280px;\n"
               "            top: -10px;\n"
               "            left: 50%;\n"
               "        }\n"
               "        #header>h1 {\n"
               "            font-family: Arial, 'Helvetica Neue', Helvetica, sans-serif;\n"
               "            font-size: 48px;\n"
               "            font-weight: 200;\n"
               "            text-shadow: 4px 4px 5px #000;\n"
               "            color: #eee;\n"
               "            position: absolute;\n"
               "            width: 600px;\n"
               "            margin-left: -80px;\n"
               "            top: 8px;\n"
               "            left: 50%;\n"
               "        }\n"
               "        body {\n"
               "            font-family: Consolas, monaco, monospace;\n"
               "            font-size: 14px;\n"
               "            line-height: 20px;\n"
               "            color: #eee;\n"
               "            height: 100%;\n"
               "            margin: 0;\n"
               "            overflow: hidden;\n"
               "        }\n"
               "        #wrapper {\n"
               "            background-color: rgba(0, 0, 0, 0.7);\n"
               "            border: 1px solid #446;\n"
               "            box-shadow: 0px 0px 10px #000;\n"
               "            padding: 8px 12px;\n"
               "            display: inline-block;\n"
               "            position: absolute;\n"
               "            top: 80px;\n"
               "            bottom: 25px;\n"
               "            left: 50px;\n"
               "            right: 50px;\n"
               "            overflow: auto;\n"
               "        }\n"
               "        details>*:not(summary) {\n"
               "            margin-left: 22px;\n"
               "        }\n"
               "        summary:only-child {\n"
               "            display: block;\n"
               "            padding-left: 15px;\n"
               "        }\n"
               "        details>summary:only-child::-webkit-details-marker {\n"
               "            display: none;\n"
               "            padding-left: 15px;\n"
               "        }\n"
               "        .headervar, .generalheadertype, .warningheadertype, .errorheadertype, .debugheadertype, .headerval {\n"
               "            display: inline;\n"
               "            margin: 0 9px;\n"
               "        }\n"
               "        .var, .type, .val {\n"
               "            display: inline;\n"
               "            margin: 0 6px;\n"
               "        }\n"
               "        .warningheadertype, .type {\n"
               "            color: #dce22f;\n"
               "        }\n"
               "        .errorheadertype, .type {\n"
               "            color: #ff1616;\n"
               "        }\n"
               "        .debugheadertype, .type {\n"
               "            color: #888;\n"
               "        }\n"
               "        .generalheadertype, .type {\n"
               "            color: #acf;\n"
               "        }\n"
               "        .headerval, .val {\n"
               "            color: #afa;\n"
               "            text-align: right;\n"
               "        }\n"
               "        .thd {\n"
               "            color: #888;\n"
               "        }\n"
               "        </style>\n"
               "    </head>\n"
               "    <body>\n"
               "        <div id='header'>\n"
               "            <img src='https://lunarg.com/wp-content/uploads/2016/02/LunarG-wReg-150.png' />\n"
               "            <h1>OpenXR Core Validation</h1>\n"
               "        </div>\n"
               "        <div id='wrapper'>\n";
        return true;
    } catch (...) {
        return false;
    }
}

bool CoreValidationWriteHtmlFooter(void) {
    try {
        std::unique_lock<std::mutex> mlock(g_record_mutex);
        std::ofstream html_file;
        html_file.open(g_record_info.file_name, std::ios::out | std::ios::app);
        html_file << "        </div>\n"
                     "    </body>\n"
                     "</html>";

        // Writing the footer means we're done.
        if (g_record_info.initialized) {
            g_record_info.initialized = false;
            g_record_info.type = RECORD_NONE;
        }
        return true;
    } catch (...) {
        return false;
    }
}

// Function to record all the core validation information
void CoreValidLogMessage(GenValidUsageXrInstanceInfo *instance_info, const std::string &message_id,
                         GenValidUsageDebugSeverity message_severity, const std::string &command_name,
                         std::vector<GenValidUsageXrObjectInfo> objects_info, const std::string &message) {
    if (g_record_info.initialized) {
        std::unique_lock<std::mutex> mlock(g_record_mutex);

        // Debug Utils items (in case we need them)
        XrDebugUtilsMessageSeverityFlagsEXT debug_utils_severity = 0;
        std::vector<XrDebugUtilsObjectNameInfoEXT> debug_utils_objects;
        std::vector<XrDebugUtilsLabelEXT> session_labels;

        std::string severity_string;
        switch (message_severity) {
            case VALID_USAGE_DEBUG_SEVERITY_DEBUG:
                severity_string = "VALID_DEBUG";
                debug_utils_severity = XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
                break;
            case VALID_USAGE_DEBUG_SEVERITY_INFO:
                severity_string = "VALID_INFO";
                debug_utils_severity = XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
                break;
            case VALID_USAGE_DEBUG_SEVERITY_WARNING:
                severity_string = "VALID_WARNING";
                debug_utils_severity = XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
                break;
            case VALID_USAGE_DEBUG_SEVERITY_ERROR:
                severity_string = "VALID_ERROR";
                debug_utils_severity = XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                break;
            default:
                severity_string = "VALID_UNKNOWN";
                break;
        }

        // If we have instance information, see if we need to log this information out to a debug messenger
        // callback.
        if (nullptr != instance_info) {
            if (objects_info.size() > 0) {
                for (uint32_t obj = 0; obj < objects_info.size(); ++obj) {
                    XrDebugUtilsObjectNameInfoEXT obj_name_info = {};
                    obj_name_info.next = nullptr;
                    obj_name_info.objectType = objects_info[obj].type;
                    obj_name_info.objectHandle = objects_info[obj].handle;
                    // If there's a session in the list, see if it has labels
                    if (XR_OBJECT_TYPE_SESSION == objects_info[obj].type) {
                        XrSession session = reinterpret_cast<XrSession &>(objects_info[obj].handle);
                        auto session_label_iterator = g_xr_session_labels.find(session);
                        if (session_label_iterator != g_xr_session_labels.end()) {
                            auto rev_iter = session_label_iterator->second->rbegin();
                            for (; rev_iter != session_label_iterator->second->rend(); ++rev_iter) {
                                session_labels.push_back((*rev_iter)->debug_utils_label);
                            }
                        }
                    }
                    // Loop through all object names and see if any match
                    for (uint32_t name_index = 0; name_index < instance_info->object_names.size(); ++name_index) {
                        if (instance_info->object_names[name_index]->objectType == objects_info[obj].type &&
                            instance_info->object_names[name_index]->objectHandle == objects_info[obj].handle) {
                            obj_name_info.objectName = instance_info->object_names[name_index]->objectName;
                            break;
                        }
                    }
                    debug_utils_objects.push_back(obj_name_info);
                }
            }
            if (instance_info->debug_messengers.size() > 0) {
                // Setup our callback data once
                XrDebugUtilsMessengerCallbackDataEXT callback_data = {};
                callback_data.type = XR_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT;
                callback_data.messageId = message_id.c_str();
                callback_data.functionName = command_name.c_str();
                callback_data.message = message.c_str();
                callback_data.objectCount = static_cast<uint8_t>(debug_utils_objects.size());
                if (debug_utils_objects.empty()) {
                    callback_data.objects = nullptr;
                } else {
                    callback_data.objects = debug_utils_objects.data();
                }
                callback_data.sessionLabelCount = static_cast<uint8_t>(session_labels.size());
                if (session_labels.empty()) {
                    callback_data.sessionLabels = nullptr;
                } else {
                    callback_data.sessionLabels = session_labels.data();
                }

                // Loop through all active messengers and give each a chance to output information
                for (uint32_t msg_index = 0; msg_index < instance_info->debug_messengers.size(); ++msg_index) {
                    CoreValidationMessengerInfo *validation_messenger_info = instance_info->debug_messengers[msg_index];
                    XrDebugUtilsMessengerCreateInfoEXT *messenger_create_info = validation_messenger_info->create_info;
                    // If a callback exists, and the message is of a type this callback cares about, call it.
                    if (nullptr != messenger_create_info->userCallback &&
                        0 != (messenger_create_info->messageSeverities | debug_utils_severity) &&
                        0 != (messenger_create_info->messageTypes | XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)) {
                        XrBool32 ret_val = messenger_create_info->userCallback(debug_utils_severity,
                                                                               XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
                                                                               &callback_data, messenger_create_info->userData);
                    }
                }
            }
        }

        switch (g_record_info.type) {
            case RECORD_TEXT_COUT: {
                std::cout << "[" << severity_string << " | " << message_id << " | " << command_name << "]: " << message
                          << std::endl;
                if (!objects_info.empty()) {
                    std::cout << "  Objects:" << std::endl;
                    uint32_t count = 0;
                    for (auto object_info : objects_info) {
                        std::string object_type = GenValidUsageXrObjectTypeToString(object_info.type);
                        std::ostringstream oss_object_handle;
                        oss_object_handle << std::hex << reinterpret_cast<const void *>(object_info.handle);
                        std::cout << "   [" << std::to_string(count++) << "] - " << object_type << " (" << oss_object_handle.str()
                                  << ")";
                        std::cout << std::endl;
                    }
                }
                if (!session_labels.empty()) {
                    std::cout << "  Session Labels:" << std::endl;
                    uint32_t count = 0;
                    for (auto session_label : session_labels) {
                        std::cout << "   [" << std::to_string(count++) << "] - " << session_label.labelName << std::endl;
                    }
                }
                std::cout << std::flush;
                break;
            }
            case RECORD_TEXT_FILE: {
                std::ofstream text_file;
                text_file.open(g_record_info.file_name, std::ios::out | std::ios::app);
                text_file << "[" << severity_string << " | " << message_id << " | " << command_name << "]: " << message
                          << std::endl;
                if (!objects_info.empty()) {
                    text_file << "  Objects:" << std::endl;
                    uint32_t count = 0;
                    for (auto object_info : objects_info) {
                        std::string object_type = GenValidUsageXrObjectTypeToString(object_info.type);
                        std::ostringstream oss_object_handle;
                        oss_object_handle << std::hex << reinterpret_cast<const void *>(object_info.handle);
                        text_file << "   [" << std::to_string(count++) << "] - " << object_type << " (" << oss_object_handle.str()
                                  << ")";
                        text_file << std::endl;
                    }
                }
                if (!session_labels.empty()) {
                    text_file << "  Session Labels:" << std::endl;
                    uint32_t count = 0;
                    for (auto session_label : session_labels) {
                        text_file << "   [" << std::to_string(count++) << "] - " << session_label.labelName << std::endl;
                    }
                }
                text_file << std::flush;
                text_file.close();
                break;
            }
            case RECORD_HTML_FILE: {
                std::ofstream text_file;
                text_file.open(g_record_info.file_name, std::ios::out | std::ios::app);
                text_file << "<details class='data'>\n";
                std::string header_type = "generalheadertype";
                switch (message_severity) {
                    case VALID_USAGE_DEBUG_SEVERITY_DEBUG:
                        header_type = "debugheadertype";
                        severity_string = "Debug Message";
                        break;
                    case VALID_USAGE_DEBUG_SEVERITY_INFO:
                        severity_string = "Info Message";
                        break;
                    case VALID_USAGE_DEBUG_SEVERITY_WARNING:
                        header_type = "warningheadertype";
                        severity_string = "Warning Message";
                        break;
                    case VALID_USAGE_DEBUG_SEVERITY_ERROR:
                        header_type = "errorheadertype";
                        severity_string = "Error Message";
                        break;
                    default:
                        severity_string = "Unknown Message";
                        break;
                }
                text_file << "   <summary>\n"
                          << "      <div class='" << header_type << "'>" << severity_string << "</div>\n"
                          << "      <div class='headerval'>" << command_name << "</div>\n"
                          << "      <div class='headervar'>" << message_id << "</div>\n"
                          << "   </summary>\n";
                text_file << "   <div class='data'>\n";
                text_file << "      <div class='val'>" << message << "</div>\n";
                if (!objects_info.empty()) {
                    text_file << "      <details class='data'>\n";
                    text_file << "         <summary>\n";
                    text_file << "            <div class='type'>Relevant OpenXR Objects</div>\n";
                    text_file << "         </summary>\n";
                    uint32_t count = 0;
                    for (auto object_info : objects_info) {
                        std::string object_type = GenValidUsageXrObjectTypeToString(object_info.type);
                        std::ostringstream oss_object_handle;
                        oss_object_handle << std::hex << reinterpret_cast<const void *>(object_info.handle);
                        text_file << "         <div class='data'>\n";
                        text_file << "             <div class='var'>[" << count++ << "]</div>\n";
                        text_file << "             <div class='type'>" << object_type << "</div>\n";
                        if (oss_object_handle.str() == "0") {
                            // For some reason, nullptr never has 0x prefix added
                            text_file << "             <div class='val'>0x" << oss_object_handle.str() << "</div>\n";
                        } else {
                            text_file << "             <div class='val'>" << oss_object_handle.str() << "</div>\n";
                        }
                        text_file << "         </div>\n";
                    }
                    text_file << "      </details>\n";
                    text_file << std::flush;
                }
                if (!session_labels.empty()) {
                    text_file << "      <details class='data'>\n";
                    text_file << "         <summary>\n";
                    text_file << "            <div class='type'>Relevant Session Labels</div>\n";
                    text_file << "         </summary>\n";
                    uint32_t count = 0;
                    for (auto session_label : session_labels) {
                        text_file << "         <div class='data'>\n";
                        text_file << "             <div class='var'>[" << count++ << "]</div>\n";
                        text_file << "             <div class='type'>" << session_label.labelName << "</div>\n";
                        text_file << "         </div>\n";
                    }
                    text_file << "      </details>\n";
                }
                text_file << "   </div>\n";
                text_file << "</details>\n";
                break;
            }
            default:
                break;
        }
    }
}

// NOTE: Can't validate the following VUIDs since the command never enters a layer:
// Command: xrEnumerateApiLayerProperties
//      VUIDs:  "VUID-xrEnumerateApiLayerProperties-propertyCountOutput-parameter"
//              "VUID-xrEnumerateApiLayerProperties-properties-parameter"
// Command: xrEnumerateInstanceExtensionProperties
//      VUIDs:  "VUID-xrEnumerateInstanceExtensionProperties-layerName-parameter"
//              "VUID-xrEnumerateInstanceExtensionProperties-propertyCountOutput-parameter"
//              "VUID-xrEnumerateInstanceExtensionProperties-properties-parameter"

XrResult CoreValidationXrCreateInstance(const XrInstanceCreateInfo *info, XrInstance *instance) {
    // Shouldn't be called, coreValidationXrCreateApiLayerInstance should called instead
    return XR_SUCCESS;
}

// See if there is a debug utils create structure in the "next" chain

XrResult CoreValidationXrCreateApiLayerInstance(const XrInstanceCreateInfo *info, const struct XrApiLayerCreateInfo *apiLayerInfo,
                                                XrInstance *instance) {
    try {
        XrApiLayerCreateInfo new_api_layer_info = {};
        XrResult validation_result = XR_SUCCESS;
        bool user_defined_output = false;
        bool first_time = !g_record_info.initialized;

        if (!g_record_info.initialized) {
            g_record_info.initialized = true;
            g_record_info.type = RECORD_NONE;
        }

        char *export_type = PlatformUtilsGetEnv("XR_CORE_VALIDATION_EXPORT_TYPE");
        char *file_name = PlatformUtilsGetEnv("XR_CORE_VALIDATION_FILE_NAME");
        if (nullptr != file_name) {
            g_record_info.file_name = file_name;
            PlatformUtilsFreeEnv(file_name);
        }

        if (nullptr != export_type) {
            std::string string_export_type = export_type;
            PlatformUtilsFreeEnv(export_type);
            std::transform(string_export_type.begin(), string_export_type.end(), string_export_type.begin(),
                           [](unsigned char c) { return std::tolower(c); });

            std::cerr << "Core Validation output type: " << string_export_type << ", first time = " << std::to_string(first_time)
                      << std::endl;
            if (string_export_type == "text") {
                if (g_record_info.file_name.size() > 0) {
                    g_record_info.type = RECORD_TEXT_FILE;
                } else {
                    g_record_info.type = RECORD_TEXT_COUT;
                }
                user_defined_output = true;
            } else if (string_export_type == "html" && first_time) {
                g_record_info.type = RECORD_HTML_FILE;
                if (!CoreValidationWriteHtmlHeader()) {
                    return XR_ERROR_INITIALIZATION_FAILED;
                }
            }
        }

        // Call the generated pre valid usage check.
        validation_result = GenValidUsageInputsXrCreateInstance(info, instance);

        // Copy the contents of the layer info struct, but then move the next info up by
        // one slot so that the next layer gets information.
        memcpy(&new_api_layer_info, apiLayerInfo, sizeof(XrApiLayerCreateInfo));
        new_api_layer_info.nextInfo = apiLayerInfo->nextInfo->next;

        // Get the function pointers we need
        PFN_xrGetInstanceProcAddr next_get_instance_proc_addr = apiLayerInfo->nextInfo->nextGetInstanceProcAddr;
        PFN_xrCreateApiLayerInstance next_create_api_layer_instance = apiLayerInfo->nextInfo->nextCreateApiLayerInstance;

        // Create the instance using the layer create instance command for the next layer
        XrInstance returned_instance = *instance;
        XrResult next_result = next_create_api_layer_instance(info, &new_api_layer_info, &returned_instance);
        *instance = returned_instance;

        // Create the instance information
        GenValidUsageXrInstanceInfo *instance_info = new GenValidUsageXrInstanceInfo();

        // Create the dispatch table to the next levels
        instance_info->dispatch_table = new XrGeneratedDispatchTable();
        instance_info->instance = returned_instance;
        GeneratedXrPopulateDispatchTable(instance_info->dispatch_table, returned_instance, next_get_instance_proc_addr);

        // Save the enabled extensions.
        for (uint32_t extension = 0; extension < info->enabledExtensionCount; ++extension) {
            instance_info->enabled_extensions.push_back(info->enabledExtensionNames[extension]);
        }

        std::unique_lock<std::mutex> mlock(g_instance_dispatch_mutex);
        g_instance_info_map[returned_instance] = instance_info;
        mlock.unlock();

        // See if a debug utils messenger is supposed to be created as part of the instance
        // NOTE: We have to wait until after the instance info is added to the map for this
        //       to work properly.
        const XrBaseInStructure *next_header = reinterpret_cast<const XrBaseInStructure *>(info->next);
        const XrDebugUtilsMessengerCreateInfoEXT *dbg_utils_create_info = nullptr;
        while (next_header != nullptr) {
            if (next_header->type == XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT) {
                dbg_utils_create_info = reinterpret_cast<const XrDebugUtilsMessengerCreateInfoEXT *>(next_header);
                // Create the debug messenger.  We don't have to keep track of it because it will be tracked as part
                // of the instance info from here on out.
                XrDebugUtilsMessengerEXT messenger;
                validation_result = CoreValidationXrCreateDebugUtilsMessengerEXT(*instance, dbg_utils_create_info, &messenger);
                // If we created a debug messenger, turn off the text output unless a user indicates they wanted it
                if (XR_SUCCESS == validation_result && !user_defined_output) {
                    g_record_info.type = RECORD_NONE;
                }
                break;
            }
            next_header = reinterpret_cast<const XrBaseInStructure *>(next_header->next);
        }

        if (XR_SUCCESS == validation_result) {
            return next_result;
        } else {
            return validation_result;
        }
    } catch (std::bad_alloc &) {
        return XR_ERROR_OUT_OF_MEMORY;
    } catch (...) {
        return XR_ERROR_INITIALIZATION_FAILED;
    }
}

XrResult CoreValidationXrDestroyInstance(XrInstance instance) {
    GenValidUsageInputsXrDestroyInstance(instance);
    if (XR_NULL_HANDLE != instance) {
        std::unique_lock<std::mutex> mlock(g_instance_dispatch_mutex);
        GenValidUsageXrInstanceInfo *gen_instance_info = g_instance_info_map[instance];
        if (nullptr != gen_instance_info) {
            // If there are any object names associated with this instance, delete them.
            while (0 < gen_instance_info->object_names.size()) {
                // We have to delete the pointers manually, so step through each object deleting
                // the things we allocated before we delete the object and it's entry in the vector.
                delete gen_instance_info->object_names[0]->objectName;
                delete gen_instance_info->object_names[0];
                gen_instance_info->object_names.erase(gen_instance_info->object_names.begin());
            }
            // If there are any debug messengers associated with this instance, delete them.
            while (0 < gen_instance_info->debug_messengers.size()) {
                // We have to delete the pointers manually, so step through each object deleting
                // the things we allocated before we delete the object and it's entry in the vector.
                delete gen_instance_info->debug_messengers[0]->create_info;
                delete gen_instance_info->debug_messengers[0];
                gen_instance_info->debug_messengers.erase(gen_instance_info->debug_messengers.begin());
            }
        }
    }
    XrResult result = GenValidUsageNextXrDestroyInstance(instance);
    if (g_instance_info_map.size() == 0 && g_record_info.type == RECORD_HTML_FILE) {
        CoreValidationWriteHtmlFooter();
    }
    return result;
    return XR_SUCCESS;
}

XrResult CoreValidationXrCreateSession(XrInstance instance, const XrSessionCreateInfo *createInfo, XrSession *session) {
    try {
        XrResult test_result = GenValidUsageInputsXrCreateSession(instance, createInfo, session);
        if (XR_SUCCESS != test_result) {
            return test_result;
        }

        std::unique_lock<std::mutex> mlock(g_instance_dispatch_mutex);
        GenValidUsageXrInstanceInfo *gen_instance_info = g_instance_info_map[instance];
        mlock.unlock();

        // Check the next chain for a graphics binding structure, we need at least one.
        uint32_t num_graphics_bindings_found = 0;
        const XrBaseInStructure *cur_ptr = reinterpret_cast<const XrBaseInStructure *>(createInfo->next);
        while (nullptr != cur_ptr) {
            switch (cur_ptr->type) {
                default:
                    continue;
#ifdef XR_USE_PLATFORM_WIN32
                case XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR:
                    num_graphics_bindings_found++;
                    break;
#endif
#ifdef XR_USE_PLATFORM_XLIB
                case XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR:
                    num_graphics_bindings_found++;
                    break;
#endif
#ifdef XR_USE_PLATFORM_XCB
                case XR_TYPE_GRAPHICS_BINDING_OPENGL_XCB_KHR:
                    num_graphics_bindings_found++;
                    break;
#endif
#ifdef XR_USE_PLATFORM_WAYLAND
                case XR_TYPE_GRAPHICS_BINDING_OPENGL_WAYLAND_KHR:
                    num_graphics_bindings_found++;
                    break;
#endif
            }
            cur_ptr = reinterpret_cast<const XrBaseInStructure *>(cur_ptr->next);
        }
        auto const &enabled_extensions = gen_instance_info->enabled_extensions;
        bool has_headless = (enabled_extensions.end() !=
                             std::find(enabled_extensions.begin(), enabled_extensions.end(), XR_KHR_HEADLESS_EXTENSION_NAME));
        bool got_right_graphics_binding_count = (num_graphics_bindings_found == 1);
        if (!got_right_graphics_binding_count && has_headless) {
            // This permits 0 as well.
            got_right_graphics_binding_count = (num_graphics_bindings_found == 0);
        }
        if (!got_right_graphics_binding_count) {
            std::vector<GenValidUsageXrObjectInfo> objects_info;
            GenValidUsageXrObjectInfo handle_info = {};
            handle_info.handle = CONVERT_HANDLE_TO_GENERIC(instance);
            handle_info.type = XR_OBJECT_TYPE_INSTANCE;
            objects_info.push_back(handle_info);
            std::ostringstream error_stream;
            error_stream << "Invalid number of graphics binding structures provided.  ";
            error_stream << "Expected ";
            if (has_headless) {
                error_stream << "0 or 1";
            } else {
                error_stream << "1";
            }
            error_stream << ", but received ";
            error_stream << num_graphics_bindings_found;
            error_stream << ".";
            // TODO: This needs to be updated with the actual VUID once we generate it.
            CoreValidLogMessage(gen_instance_info, "VUID-xrCreateSession-next-parameter", VALID_USAGE_DEBUG_SEVERITY_ERROR,
                                "xrCreateSession", objects_info, error_stream.str());
            return XR_ERROR_GRAPHICS_DEVICE_INVALID;
        }
        return GenValidUsageNextXrCreateSession(instance, createInfo, session);
    } catch (...) {
        return XR_SUCCESS;
    }
}

// ---- XR_EXT_debug_utils extension commands
XrResult CoreValidationXrSetDebugUtilsObjectNameEXT(XrInstance instance, const XrDebugUtilsObjectNameInfoEXT *nameInfo) {
    try {
        if (!GenValidUsageInputsXrSetDebugUtilsObjectNameEXT(instance, nameInfo)) {
            return XR_ERROR_VALIDATION_FAILURE;
        }
        XrResult result = GenValidUsageNextXrSetDebugUtilsObjectNameEXT(instance, nameInfo);
        if (XR_SUCCESS == result) {
            std::unique_lock<std::mutex> mlock(g_instance_dispatch_mutex);
            GenValidUsageXrInstanceInfo *gen_instance_info = g_instance_info_map[instance];
            mlock.unlock();
            if (nullptr != gen_instance_info) {
                // Create a copy of the base object name info (no next items)
                char *name_string = new char[strlen(nameInfo->objectName) + 1];
                strcpy(name_string, nameInfo->objectName);
                bool found = false;
                for (uint32_t name_index = 0; name_index < gen_instance_info->object_names.size(); ++name_index) {
                    if (gen_instance_info->object_names[name_index]->objectHandle == nameInfo->objectHandle &&
                        gen_instance_info->object_names[name_index]->objectType == nameInfo->objectType) {
                        delete[] gen_instance_info->object_names[name_index]->objectName;
                        gen_instance_info->object_names[name_index]->objectName = name_string;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    XrDebugUtilsObjectNameInfoEXT *new_object_name = new XrDebugUtilsObjectNameInfoEXT;
                    *new_object_name = *nameInfo;
                    new_object_name->next = nullptr;
                    new_object_name->objectName = name_string;
                    gen_instance_info->object_names.push_back(new_object_name);
                }
            }
        }
        return result;
    } catch (...) {
        return XR_ERROR_VALIDATION_FAILURE;
    }
}

XrResult CoreValidationXrCreateDebugUtilsMessengerEXT(XrInstance instance, const XrDebugUtilsMessengerCreateInfoEXT *createInfo,
                                                      XrDebugUtilsMessengerEXT *messenger) {
    XrResult result = XR_SUCCESS;
    try {
        if (!GenValidUsageInputsXrCreateDebugUtilsMessengerEXT(instance, createInfo, messenger)) {
            return XR_ERROR_VALIDATION_FAILURE;
        }
        result = GenValidUsageNextXrCreateDebugUtilsMessengerEXT(instance, createInfo, messenger);
        if (XR_SUCCESS == result) {
            std::unique_lock<std::mutex> mlock(g_instance_dispatch_mutex);
            GenValidUsageXrInstanceInfo *gen_instance_info = g_instance_info_map[instance];
            if (nullptr != gen_instance_info) {
                XrDebugUtilsMessengerCreateInfoEXT *new_create_info = new XrDebugUtilsMessengerCreateInfoEXT;
                *new_create_info = *createInfo;
                new_create_info->next = nullptr;
                CoreValidationMessengerInfo *new_messenger_info = new CoreValidationMessengerInfo;
                new_messenger_info->messenger = *messenger;
                new_messenger_info->create_info = new_create_info;
                gen_instance_info->debug_messengers.push_back(new_messenger_info);
            }
        }
    } catch (...) {
        return XR_ERROR_VALIDATION_FAILURE;
    }
    return result;
}

XrResult CoreValidationXrDestroyDebugUtilsMessengerEXT(XrDebugUtilsMessengerEXT messenger) {
    try {
        if (!GenValidUsageInputsXrDestroyDebugUtilsMessengerEXT(messenger)) {
            return XR_ERROR_VALIDATION_FAILURE;
        }
        XrResult result = GenValidUsageNextXrDestroyDebugUtilsMessengerEXT(messenger);
        if (XR_NULL_HANDLE != messenger) {
            std::unique_lock<std::mutex> mlock(g_debugutilsmessengerext_dispatch_mutex);
            GenValidUsageXrHandleInfo *gen_handle_info = g_debugutilsmessengerext_info_map[messenger];
            if (nullptr != gen_handle_info) {
                GenValidUsageXrInstanceInfo *gen_instance_info = gen_handle_info->instance_info;
                for (uint32_t msg_index = 0; msg_index < gen_instance_info->debug_messengers.size(); ++msg_index) {
                    if (gen_instance_info->debug_messengers[msg_index]->messenger == messenger) {
                        delete gen_instance_info->debug_messengers[msg_index]->create_info;
                        delete gen_instance_info->debug_messengers[msg_index];
                        gen_instance_info->debug_messengers.erase(gen_instance_info->debug_messengers.begin() + msg_index);
                        break;
                    }
                }
            } else {
                return XR_ERROR_DEBUG_UTILS_MESSENGER_INVALID_EXT;
            }
        }
        return result;
    } catch (...) {
        return XR_ERROR_VALIDATION_FAILURE;
    }
}

// We always want to remove the old individual label before we do anything else.
// So, do that in it's own method
void CoreValidationRemoveIndividualLabel(std::vector<GenValidUsageXrInternalSessionLabel *> *label_vec) {
    if (!label_vec->empty()) {
        GenValidUsageXrInternalSessionLabel *cur_label = label_vec->back();
        if (cur_label->is_individual_label) {
            label_vec->pop_back();
            delete cur_label;
        }
    }
}

void CoreValidationBeginLabelRegion(XrSession session, const XrDebugUtilsLabelEXT *label_info) {
    std::vector<GenValidUsageXrInternalSessionLabel *> *vec_ptr = nullptr;
    auto session_label_iterator = g_xr_session_labels.find(session);
    if (session_label_iterator == g_xr_session_labels.end()) {
        vec_ptr = new std::vector<GenValidUsageXrInternalSessionLabel *>;
        g_xr_session_labels[session] = vec_ptr;
    } else {
        vec_ptr = session_label_iterator->second;
    }

    // Individual labels do not stay around in the transition into a new label region
    CoreValidationRemoveIndividualLabel(vec_ptr);

    // Start the new label region
    GenValidUsageXrInternalSessionLabel *new_session_label = new GenValidUsageXrInternalSessionLabel;
    new_session_label->label_name = label_info->labelName;
    new_session_label->debug_utils_label = *label_info;
    new_session_label->debug_utils_label.labelName = new_session_label->label_name.c_str();
    new_session_label->is_individual_label = false;
    vec_ptr->push_back(new_session_label);
}

void CoreValidationEndLabelRegion(XrSession session) {
    auto session_label_iterator = g_xr_session_labels.find(session);
    if (session_label_iterator == g_xr_session_labels.end()) {
        return;
    }

    std::vector<GenValidUsageXrInternalSessionLabel *> *vec_ptr = session_label_iterator->second;

    // Individual labels do not stay around in the transition out of label region
    CoreValidationRemoveIndividualLabel(vec_ptr);

    // Remove the last label region
    if (!vec_ptr->empty()) {
        GenValidUsageXrInternalSessionLabel *cur_label = vec_ptr->back();
        vec_ptr->pop_back();
        delete cur_label;
    }
}

void CoreValidationInsertLabel(XrSession session, const XrDebugUtilsLabelEXT *label_info) {
    std::vector<GenValidUsageXrInternalSessionLabel *> *vec_ptr = nullptr;
    auto session_label_iterator = g_xr_session_labels.find(session);
    if (session_label_iterator == g_xr_session_labels.end()) {
        vec_ptr = new std::vector<GenValidUsageXrInternalSessionLabel *>;
        g_xr_session_labels[session] = vec_ptr;
    } else {
        vec_ptr = session_label_iterator->second;
    }

    // Remove any individual layer that might already be there
    CoreValidationRemoveIndividualLabel(vec_ptr);

    // Insert a new individual label
    GenValidUsageXrInternalSessionLabel *new_session_label = new GenValidUsageXrInternalSessionLabel;
    new_session_label->label_name = label_info->labelName;
    new_session_label->debug_utils_label = *label_info;
    new_session_label->debug_utils_label.labelName = new_session_label->label_name.c_str();
    new_session_label->is_individual_label = true;
    vec_ptr->push_back(new_session_label);
}

// Called during xrDestroySession.  We need to delete all session related labels.
void CoreValidationDeleteSessionLabels(XrSession session) {
    std::vector<GenValidUsageXrInternalSessionLabel *> *vec_ptr = nullptr;
    auto session_label_iterator = g_xr_session_labels.find(session);
    if (session_label_iterator == g_xr_session_labels.end()) {
        return;
    }
    vec_ptr = session_label_iterator->second;
    while (!vec_ptr->empty()) {
        delete vec_ptr->back();
        vec_ptr->pop_back();
    }
    delete vec_ptr;
    g_xr_session_labels.erase(session);
}

XrResult CoreValidationXrSessionBeginDebugUtilsLabelRegionEXT(XrSession session, const XrDebugUtilsLabelEXT *labelInfo) {
    XrResult test_result = GenValidUsageInputsXrSessionBeginDebugUtilsLabelRegionEXT(session, labelInfo);
    if (XR_SUCCESS != test_result) {
        return test_result;
    }
    CoreValidationBeginLabelRegion(session, labelInfo);
    return GenValidUsageNextXrSessionBeginDebugUtilsLabelRegionEXT(session, labelInfo);
}

XrResult CoreValidationXrSessionEndDebugUtilsLabelRegionEXT(XrSession session) {
    XrResult test_result = GenValidUsageInputsXrSessionEndDebugUtilsLabelRegionEXT(session);
    if (XR_SUCCESS != test_result) {
        return test_result;
    }
    CoreValidationEndLabelRegion(session);
    return GenValidUsageNextXrSessionEndDebugUtilsLabelRegionEXT(session);
}

XrResult CoreValidationXrSessionInsertDebugUtilsLabelEXT(XrSession session, const XrDebugUtilsLabelEXT *labelInfo) {
    XrResult test_result = GenValidUsageInputsXrSessionInsertDebugUtilsLabelEXT(session, labelInfo);
    if (XR_SUCCESS != test_result) {
        return test_result;
    }
    CoreValidationInsertLabel(session, labelInfo);
    return GenValidUsageNextXrSessionInsertDebugUtilsLabelEXT(session, labelInfo);
}

// ############################################################
// NOTE: Add new validation checking above this comment block
// ############################################################

extern "C" {

// Function used to negotiate an interface betewen the loader and an API layer.  Each library exposing one or
// more API layers needs to expose at least this function.
LAYER_EXPORT XrResult xrNegotiateLoaderApiLayerInterface(const XrNegotiateLoaderInfo *loaderInfo, const char *apiLayerName,
                                                         XrNegotiateApiLayerRequest *apiLayerRequest) {
    if (nullptr == loaderInfo || nullptr == apiLayerRequest || loaderInfo->structType != XR_LOADER_INTERFACE_STRUCT_LOADER_INFO ||
        loaderInfo->structVersion != XR_LOADER_INFO_STRUCT_VERSION || loaderInfo->structSize != sizeof(XrNegotiateLoaderInfo) ||
        apiLayerRequest->structType != XR_LOADER_INTERFACE_STRUCT_API_LAYER_REQUEST ||
        apiLayerRequest->structVersion != XR_API_LAYER_INFO_STRUCT_VERSION ||
        apiLayerRequest->structSize != sizeof(XrNegotiateApiLayerRequest) ||
        loaderInfo->minInterfaceVersion > XR_CURRENT_LOADER_API_LAYER_VERSION ||
        loaderInfo->maxInterfaceVersion < XR_CURRENT_LOADER_API_LAYER_VERSION ||
        loaderInfo->maxInterfaceVersion > XR_CURRENT_LOADER_API_LAYER_VERSION ||
        loaderInfo->maxXrVersion < XR_CORE_VALIDATION_API_VERSION || loaderInfo->minXrVersion > XR_CORE_VALIDATION_API_VERSION) {
        return XR_ERROR_INITIALIZATION_FAILED;
    }

    apiLayerRequest->layerInterfaceVersion = XR_CURRENT_LOADER_API_LAYER_VERSION;
    apiLayerRequest->layerXrVersion = XR_CORE_VALIDATION_API_VERSION;
    apiLayerRequest->getInstanceProcAddr = reinterpret_cast<PFN_xrGetInstanceProcAddr>(GenValidUsageXrGetInstanceProcAddr);
    apiLayerRequest->createApiLayerInstance =
        reinterpret_cast<PFN_xrCreateApiLayerInstance>(CoreValidationXrCreateApiLayerInstance);

    return XR_SUCCESS;
}

}  // extern "C"
