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

#define XR_UTILS_INCLUDE_IMPLEMENTATION

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>
#include <mutex>
#include <unordered_map>
#include <algorithm>
#include <cctype>

#include "xr_generated_api_dump.hpp"
#include "xr_generated_dispatch_table.h"
#include "loader_interfaces.h"
#include "platform_utils.hpp"
#include "xr_utils.h"

#if defined(__GNUC__) && __GNUC__ >= 4
#define LAYER_EXPORT __attribute__((visibility("default")))
#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x590)
#define LAYER_EXPORT __attribute__((visibility("default")))
#else
#define LAYER_EXPORT
#endif

enum ApiDumpRecordType {
    RECORD_NONE = 0,
    RECORD_TEXT_COUT,
    RECORD_TEXT_FILE,
    RECORD_HTML_FILE,
    RECORD_CODE_FILE,
};

struct ApiDumpRecordInfo {
    bool initialized;
    ApiDumpRecordType type;
    std::string file_name;
};

static ApiDumpRecordInfo g_record_info = {};
static std::mutex g_record_mutex = {};

// HTML utilities
bool ApiDumpLayerWriteHtmlHeader(void) {
    try {
        std::unique_lock<std::mutex> mlock(g_record_mutex);
        std::ofstream html_file;
        html_file.open(g_record_info.file_name, std::ios::out);
        html_file << "<!doctype html>\n"
                     "<html>\n"
                     "    <head>\n"
                     "        <title>OpenXR API Dump</title>\n"
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
                     "            font-size: 44px;\n"
                     "            font-weight: 200;\n"
                     "            text-shadow: 4px 4px 5px #000;\n"
                     "            color: #eee;\n"
                     "            position: absolute;\n"
                     "            width: 400px;\n"
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
                     "        .headervar, .headertype, .headerval {\n"
                     "            display: inline;\n"
                     "            margin: 0 9px;\n"
                     "        }\n"
                     "        .var, .type, .val {\n"
                     "            display: inline;\n"
                     "            margin: 0 6px;\n"
                     "        }\n"
                     "        .headertype, .type {\n"
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
                     "            <h1>OpenXR API Dump</h1>\n"
                     "        </div>\n"
                     "        <div id='wrapper'>\n";
        return true;
    } catch (...) {
        return false;
    }
}

bool ApiDumpLayerWriteHtmlFooter(void) {
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

// Api Dump Utility function to return an instance based on the generated dispatch table
// pointer.
XrInstance FindInstanceFromDispatchTable(XrGeneratedDispatchTable *dispatch_table) {
    std::unique_lock<std::mutex> mlock(g_instance_dispatch_mutex);
    XrInstance instance = XR_NULL_HANDLE;
    for (auto it = g_instance_dispatch_map.begin(); it != g_instance_dispatch_map.end();) {
        if (it->second == dispatch_table) {
            instance = it->first;
            break;
        }
    }
    return instance;
}

// Function to record all the API dump information
bool ApiDumpLayerRecordContent(std::vector<std::tuple<std::string, std::string, std::string>> contents) {
    bool success = false;
    if (g_record_info.initialized) {
        std::unique_lock<std::mutex> mlock(g_record_mutex);
        uint32_t count = 0;
        switch (g_record_info.type) {
            case RECORD_TEXT_COUT: {
                for (auto content : contents) {
                    std::string content_type;
                    std::string content_name;
                    std::string content_value;
                    std::tie(content_type, content_name, content_value) = content;
                    if (count++ != 0) {
                        std::cout << "    ";
                    }
                    if (content_value.size() > 0) {
                        std::cout << content_type << " " << content_name << " = " << content_value << "\n";
                    } else {
                        std::cout << content_type << " " << content_name << "\n";
                    }
                }
                success = true;
                break;
            }
            case RECORD_TEXT_FILE: {
                std::ofstream text_file;
                text_file.open(g_record_info.file_name, std::ios::out | std::ios::app);
                for (auto content : contents) {
                    std::string content_type;
                    std::string content_name;
                    std::string content_value;
                    std::tie(content_type, content_name, content_value) = content;
                    if (count++ != 0) {
                        text_file << "    ";
                    }
                    if (content_value.size() > 0) {
                        text_file << content_type << " " << content_name << " = " << content_value << "\n";
                    } else {
                        text_file << content_type << " " << content_name << "\n";
                    }
                }
                text_file.close();
                success = true;
                break;
            }
            case RECORD_HTML_FILE: {
                std::ofstream text_file;
                text_file.open(g_record_info.file_name, std::ios::out | std::ios::app);
                text_file << "<details class='data'>\n";
                std::vector<std::string> prefixes;
                uint32_t last_deref_count = 0;
                for (uint32_t content_index = 0; content_index < contents.size(); ++content_index) {
                    std::string content_type;
                    std::string content_name;
                    std::string content_value;
                    std::tie(content_type, content_name, content_value) = contents[content_index];
                    if (content_index == 0) {
                        text_file << "   <summary>\n"
                                  << "      <div class='headertype'>" << content_type << "</div>\n"
                                  << "      <div class='headervar'>" << content_name << "</div>\n"
                                  << "   </summary>\n";
                    } else {
                        uint32_t cur_deref_count = 0;
                        uint32_t next_deref_count = 0;

                        // Count number of structure and pointer dereferences for the current line
                        cur_deref_count = static_cast<uint32_t>(std::count(content_name.begin(), content_name.end(), '.'));
                        std::string::size_type start = 0;
                        while ((start = content_name.find("->", start)) != std::string::npos) {
                            ++cur_deref_count;
                            start += 2;
                        }
                        // Now look for array dereferences
                        start = 0;
                        while ((start = content_name.find("[", start)) != std::string::npos) {
                            ++cur_deref_count;
                            start++;
                        }

                        // If there's something after this, see if it's a sub-component of this.
                        if (content_index < contents.size() - 1) {
                            std::string next_content_type;
                            std::string next_content_name;
                            std::string next_content_value;
                            std::tie(next_content_type, next_content_name, next_content_value) = contents[content_index + 1];

                            // Count number of structure and pointer dereferences for the next line
                            next_deref_count =
                                static_cast<uint32_t>(std::count(next_content_name.begin(), next_content_name.end(), '.'));
                            start = 0;
                            while ((start = next_content_name.find("->", start)) != std::string::npos) {
                                ++next_deref_count;
                                start += 2;
                            }
                            // Now look for array dereferences
                            start = 0;
                            while ((start = next_content_name.find("[", start)) != std::string::npos) {
                                ++next_deref_count;
                                start++;
                            }
                        }

                        // If we've reduced the number of dereferences in the name from last time, we need
                        // to close up those detail sections.
                        if (cur_deref_count < last_deref_count) {
                            uint32_t diff_count = last_deref_count - cur_deref_count;
                            while (diff_count--) {
                                text_file << "   </details>\n";
                                prefixes.pop_back();
                            }
                        }

                        // Look through any prefixes we've saved (going backwards through the list)
                        // and find the one that matches our beginning.
                        std::string short_name = content_name;
                        if (cur_deref_count > 0) {
                            for (auto it = prefixes.rbegin(); it != prefixes.rend(); ++it) {
                                if (content_name.find(*it) == 0) {
                                    std::string::size_type additional_offset = it->size() + 1;
                                    if (content_name[additional_offset - 1] == '-') {
                                        additional_offset++;
                                    } else if (content_name[additional_offset - 1] == '[') {
                                        additional_offset--;
                                    }
                                    short_name = content_name.substr(additional_offset);
                                    break;
                                }
                            }
                        }

                        bool writing_summary = false;

                        // If the next item contains this item as a prefix, start the summary.  Otherwise,
                        // start a <div> marker so that each component lands on its own line.
                        if (cur_deref_count < next_deref_count) {
                            text_file << "   <details class='data'>\n"
                                      << "      <summary>\n";
                            writing_summary = true;
                            prefixes.push_back(content_name);
                        } else {
                            text_file << "      <div class='data'>\n";
                        }

                        // Write out the content
                        text_file << "         <div class='type'>" << content_type << "</div>\n"
                                  << "         <div class='var'>" << short_name << "</div>\n";
                        bool value_needs_printing = true;
                        if (content_type.find("char") != std::string::npos) {
                            uint64_t star_count = std::count(content_type.begin(), content_type.end(), '*');
                            uint64_t bracket_count = std::count(content_type.begin(), content_type.end(), '[');
                            if (star_count + bracket_count < 2) {
                                text_file << "         <div class='val'>\"" << content_value << "\"</div>";
                                value_needs_printing = false;
                            }
                        }
                        if (content_value.size() > 0 && value_needs_printing) {
                            text_file << "         <div class='val'>" << content_value << "</div>";
                        }
                        text_file << "\n";

                        // Wrap up any summary we may have started.  Otherwise, just wrap up the
                        // <div> marker wrapping this entry.
                        if (writing_summary) {
                            text_file << "      </summary>\n";
                        } else {
                            text_file << "      </div>\n";
                        }

                        last_deref_count = cur_deref_count;
                    }
                }

                // Wrap up any remaining items
                if (last_deref_count) {
                    while (last_deref_count--) {
                        text_file << "   </details>\n";
                        prefixes.pop_back();
                    }
                }
                text_file << "</details>\n";
                break;
            }
            default:
                break;
        }
    }
    return success;
}

XrResult ApiDumpLayerXrCreateInstance(const XrInstanceCreateInfo *info, XrInstance *instance) {
    if (!g_record_info.initialized) {
        g_record_info.initialized = true;
        g_record_info.type = RECORD_TEXT_COUT;
    }
    return XR_SUCCESS;
}

XrResult ApiDumpLayerXrCreateApiLayerInstance(const XrInstanceCreateInfo *info, const struct XrApiLayerCreateInfo *apiLayerInfo,
                                              XrInstance *instance) {
    try {
        PFN_xrGetInstanceProcAddr next_get_instance_proc_addr = nullptr;
        PFN_xrCreateApiLayerInstance next_create_api_layer_instance = nullptr;
        XrApiLayerCreateInfo new_api_layer_info = {};
        bool first_time = !g_record_info.initialized;

        if (!g_record_info.initialized) {
            g_record_info.initialized = true;
            g_record_info.type = RECORD_TEXT_COUT;
        }

        char *export_type = PlatformUtilsGetEnv("XR_API_DUMP_EXPORT_TYPE");
        char *file_name = PlatformUtilsGetEnv("XR_API_DUMP_FILE_NAME");
        if (nullptr != file_name) {
            g_record_info.file_name = file_name;
            g_record_info.type = RECORD_TEXT_FILE;
            PlatformUtilsFreeEnv(file_name);
        }

        if (nullptr != export_type) {
            std::string string_export_type = export_type;
            PlatformUtilsFreeEnv(export_type);
            std::transform(string_export_type.begin(), string_export_type.end(), string_export_type.begin(),
                           [](unsigned char c) { return std::tolower(c); });

            if (string_export_type == "text") {
                if (g_record_info.file_name.size() > 0) {
                    g_record_info.type = RECORD_TEXT_FILE;
                } else {
                    g_record_info.type = RECORD_TEXT_COUT;
                }
            } else if (string_export_type == "html" && first_time) {
                g_record_info.type = RECORD_HTML_FILE;
                if (!ApiDumpLayerWriteHtmlHeader()) {
                    return XR_ERROR_INITIALIZATION_FAILED;
                }
            } else if (string_export_type == "code") {
                g_record_info.type = RECORD_CODE_FILE;
            }
        }

        // Validate the API layer info and next API layer info structures before we try to use them
        if (nullptr == apiLayerInfo || XR_LOADER_INTERFACE_STRUCT_API_LAYER_CREATE_INFO != apiLayerInfo->structType ||
            XR_API_LAYER_CREATE_INFO_STRUCT_VERSION > apiLayerInfo->structVersion ||
            sizeof(XrApiLayerCreateInfo) > apiLayerInfo->structSize || nullptr == apiLayerInfo->loaderInstance ||
            nullptr == apiLayerInfo->nextInfo ||
            XR_LOADER_INTERFACE_STRUCT_API_LAYER_NEXT_INFO != apiLayerInfo->nextInfo->structType ||
            XR_API_LAYER_NEXT_INFO_STRUCT_VERSION > apiLayerInfo->nextInfo->structVersion ||
            sizeof(XrApiLayerNextInfo) > apiLayerInfo->nextInfo->structSize ||
            0 != strcmp("XR_APILAYER_LUNARG_api_dump", apiLayerInfo->nextInfo->layerName) ||
            nullptr == apiLayerInfo->nextInfo->nextGetInstanceProcAddr ||
            nullptr == apiLayerInfo->nextInfo->nextCreateApiLayerInstance) {
            return XR_ERROR_INITIALIZATION_FAILED;
        }

        // Generate output for this command as if it were the standard xrCreateInstance
        std::vector<std::tuple<std::string, std::string, std::string>> contents;
        contents.push_back(std::make_tuple("XrResult", "xrCreateInstance", ""));
        contents.push_back(std::make_tuple("const XrInstanceCreateInfo*", "info", PointerToHexString(info)));
        if (nullptr != info) {
            std::string prefix = "info->";
            contents.push_back(std::make_tuple("XrStructureType", "info->type", std::to_string(info->type)));
            std::string next_prefix = prefix;
            next_prefix += "next";
            // Decode the next chain if it exists
            if (!ApiDumpDecodeNextChain(nullptr, info->next, next_prefix, contents)) {
                throw std::invalid_argument("Invalid Operation");
            }
            std::string flags_prefix = prefix;
            flags_prefix += "createFlags";
            contents.push_back(std::make_tuple("XrInstanceCreateFlags", flags_prefix, std::to_string(info->createFlags)));
            std::string applicationinfo_prefix = prefix;
            applicationinfo_prefix += "applicationInfo";
            if (!ApiDumpOutputXrStruct(nullptr, &info->applicationInfo, applicationinfo_prefix, "XrApplicationInfo", true,
                                       contents)) {
                throw std::invalid_argument("Invalid Operation");
            }
            std::string enabledapilayercount_prefix = prefix;
            enabledapilayercount_prefix += "enabledApiLayerCount";
            std::ostringstream oss_enabledApiLayerCount;
            oss_enabledApiLayerCount << "0x" << std::hex << (info->enabledApiLayerCount);
            contents.push_back(std::make_tuple("uint32_t", enabledapilayercount_prefix, oss_enabledApiLayerCount.str()));
            std::string enabledapilayernames_prefix = prefix;
            enabledapilayernames_prefix += "enabledApiLayerNames";
            std::ostringstream oss_enabledApiLayerNames_array;
            oss_enabledApiLayerNames_array << "0x" << std::hex << (info->enabledApiLayerNames);
            contents.push_back(
                std::make_tuple("const char* const*", enabledapilayernames_prefix, oss_enabledApiLayerNames_array.str()));
            for (uint32_t info_enabledapilayernames_inc = 0; info_enabledapilayernames_inc < info->enabledApiLayerCount;
                 ++info_enabledapilayernames_inc) {
                std::string enabledapilayernames_array_prefix = enabledapilayernames_prefix;
                enabledapilayernames_array_prefix += "[";
                enabledapilayernames_array_prefix += std::to_string(info_enabledapilayernames_inc);
                enabledapilayernames_array_prefix += "]";
                std::ostringstream oss_enabledApiLayerNames;
                oss_enabledApiLayerNames << "0x" << std::hex << (*info->enabledApiLayerNames[info_enabledapilayernames_inc]);
                contents.push_back(
                    std::make_tuple("const char* const*", enabledapilayernames_array_prefix, oss_enabledApiLayerNames.str()));
            }
            std::string enabledextensioncount_prefix = prefix;
            enabledextensioncount_prefix += "enabledExtensionCount";
            std::ostringstream oss_enabledExtensionCount;
            oss_enabledExtensionCount << "0x" << std::hex << (info->enabledExtensionCount);
            contents.push_back(std::make_tuple("uint32_t", enabledextensioncount_prefix, oss_enabledExtensionCount.str()));
            std::string enabledextensionnames_prefix = prefix;
            enabledextensionnames_prefix += "enabledExtensionNames";
            std::ostringstream oss_enabledExtensionNames_array;
            oss_enabledExtensionNames_array << "0x" << std::hex << (info->enabledExtensionNames);
            contents.push_back(
                std::make_tuple("const char* const*", enabledextensionnames_prefix, oss_enabledExtensionNames_array.str()));
            for (uint32_t info_enabledextensionnames_inc = 0; info_enabledextensionnames_inc < info->enabledExtensionCount;
                 ++info_enabledextensionnames_inc) {
                std::string enabledextensionnames_array_prefix = enabledextensionnames_prefix;
                enabledextensionnames_array_prefix += "[";
                enabledextensionnames_array_prefix += std::to_string(info_enabledextensionnames_inc);
                enabledextensionnames_array_prefix += "]";
                std::ostringstream oss_enabledExtensionNames;
                oss_enabledExtensionNames << "0x" << std::hex << (*info->enabledExtensionNames[info_enabledextensionnames_inc]);
                contents.push_back(
                    std::make_tuple("const char* const*", enabledextensionnames_array_prefix, oss_enabledExtensionNames.str()));
            }
        }

        contents.push_back(std::make_tuple("XrInstance*", "instance", PointerToHexString(instance)));
        ApiDumpLayerRecordContent(contents);

        // Copy the contents of the layer info struct, but then move the next info up by
        // one slot so that the next layer gets information.
        memcpy(&new_api_layer_info, apiLayerInfo, sizeof(XrApiLayerCreateInfo));
        new_api_layer_info.nextInfo = apiLayerInfo->nextInfo->next;

        // Get the function pointers we need
        next_get_instance_proc_addr = apiLayerInfo->nextInfo->nextGetInstanceProcAddr;
        next_create_api_layer_instance = apiLayerInfo->nextInfo->nextCreateApiLayerInstance;

        // Create the instance
        XrInstance returned_instance = *instance;
        XrResult result = next_create_api_layer_instance(info, &new_api_layer_info, &returned_instance);
        *instance = returned_instance;

        // Create the dispatch table to the next levels
        XrGeneratedDispatchTable *next_dispatch = new XrGeneratedDispatchTable();
        GeneratedXrPopulateDispatchTable(next_dispatch, returned_instance, next_get_instance_proc_addr);

        std::unique_lock<std::mutex> mlock(g_instance_dispatch_mutex);
        g_instance_dispatch_map[returned_instance] = next_dispatch;

        return result;
    } catch (...) {
        return XR_ERROR_INITIALIZATION_FAILED;
    }
}

XrResult ApiDumpLayerXrDestroyInstance(XrInstance instance) {
    // Generate output for this command
    std::vector<std::tuple<std::string, std::string, std::string>> contents;
    contents.push_back(std::make_tuple("XrResult", "xrDestroyInstance", ""));
    contents.push_back(std::make_tuple("XrInstance", "instance", HandleToHexString(instance)));
    ApiDumpLayerRecordContent(contents);

    std::unique_lock<std::mutex> mlock(g_instance_dispatch_mutex);
    XrGeneratedDispatchTable *next_dispatch = nullptr;
    auto map_iter = g_instance_dispatch_map.find(instance);
    if (map_iter != g_instance_dispatch_map.end()) {
        next_dispatch = map_iter->second;
    }
    mlock.unlock();

    if (nullptr == next_dispatch) {
        return XR_ERROR_HANDLE_INVALID;
    }

    next_dispatch->DestroyInstance(instance);
    ApiDumpCleanUpMapsForTable(next_dispatch);

    // Write out the HTML footer if we destroy the last instance
    if (g_instance_dispatch_map.size() == 0 && g_record_info.type == RECORD_HTML_FILE) {
        ApiDumpLayerWriteHtmlFooter();
    }
    return XR_SUCCESS;
}

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
        loaderInfo->maxXrVersion < XR_CURRENT_API_VERSION || loaderInfo->minXrVersion > XR_CURRENT_API_VERSION) {
        return XR_ERROR_INITIALIZATION_FAILED;
    }

    apiLayerRequest->layerInterfaceVersion = XR_CURRENT_LOADER_API_LAYER_VERSION;
    apiLayerRequest->layerXrVersion = XR_CURRENT_API_VERSION;
    apiLayerRequest->getInstanceProcAddr = reinterpret_cast<PFN_xrGetInstanceProcAddr>(ApiDumpLayerXrGetInstanceProcAddr);
    apiLayerRequest->createApiLayerInstance = reinterpret_cast<PFN_xrCreateApiLayerInstance>(ApiDumpLayerXrCreateApiLayerInstance);

    return XR_SUCCESS;
}

}  // extern "C"
