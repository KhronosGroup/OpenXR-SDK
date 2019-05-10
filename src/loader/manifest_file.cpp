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

#ifdef XR_OS_WINDOWS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "filesystem_utils.hpp"
#include "loader_platform.hpp"
#include "platform_utils.hpp"
#include "manifest_file.hpp"
#include "loader_logger.hpp"
#include "loader_instance.hpp"
#include "xr_generated_loader.hpp"

// OpenXR paths and registry key locations
#define OPENXR_RELATIVE_PATH "openxr/"
#define OPENXR_IMPLICIT_API_LAYER_RELATIVE_PATH "/api_layers/implicit.d"
#define OPENXR_EXPLICIT_API_LAYER_RELATIVE_PATH "/api_layers/explicit.d"
#ifdef XR_OS_WINDOWS
#define OPENXR_REGISTRY_LOCATION "SOFTWARE\\Khronos\\OpenXR\\"
#define OPENXR_IMPLICIT_API_LAYER_REGISTRY_LOCATION "\\ApiLayers\\Implicit"
#define OPENXR_EXPLICIT_API_LAYER_REGISTRY_LOCATION "\\ApiLayers\\Explicit"
#endif

// OpenXR Loader environment variables of interest
#define OPENXR_RUNTIME_JSON_ENV_VAR "XR_RUNTIME_JSON"
#define OPENXR_API_LAYER_PATH_ENV_VAR "XR_API_LAYER_PATH"

// Utility functions for finding files in the appropriate paths

static inline bool StringEndsWith(std::string const &value, std::string const &ending) {
    if (ending.size() > value.size()) {
        return false;
    }
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

// If the file found is a manifest file name, add it to the out_files manifest list.
static void AddIfJson(ManifestFileType type, const std::string &full_file, std::vector<std::string> &manifest_files) {
    try {
        if (0 == full_file.size() || !StringEndsWith(full_file, ".json")) {
            return;
        }
        manifest_files.push_back(full_file);
    } catch (...) {
        LoaderLogger::LogErrorMessage("", "AddIfJson - unknown error occurred");
        throw;
    }
}

// Check the current path for any manifest files.  If the provided search_path is a directory, look for
// all included JSON files in that directory.  Otherwise, just check the provided search_path which should
// be a single filename.
static void CheckAllFilesInThePath(ManifestFileType type, const std::string &search_path, bool is_directory_list,
                                   std::vector<std::string> &manifest_files) {
    try {
        if (FileSysUtilsPathExists(search_path)) {
            std::string absolute_path;
            if (!is_directory_list) {
                // If the file exists, try to add it
                if (FileSysUtilsIsRegularFile(search_path)) {
                    FileSysUtilsGetAbsolutePath(search_path, absolute_path);
                    AddIfJson(type, absolute_path, manifest_files);
                }
            } else {
                std::vector<std::string> files;
                if (FileSysUtilsFindFilesInPath(search_path, files)) {
                    for (std::string &cur_file : files) {
                        std::string relative_path;
                        FileSysUtilsCombinePaths(search_path, cur_file, relative_path);
                        if (!FileSysUtilsGetAbsolutePath(relative_path, absolute_path)) {
                            continue;
                        }
                        AddIfJson(type, absolute_path, manifest_files);
                    }
                }
            }
        }
    } catch (...) {
        LoaderLogger::LogErrorMessage("", "CheckAllFilesInThePath - unknown error occurred");
        throw;
    }
}

// Add all manifest files in the provided paths to the manifest_files list.  If search_path
// is made up of directory listings (versus direct manifest file names) search each path for
// any manifest files.
static void AddFilesInPath(ManifestFileType type, const std::string &search_path, bool is_directory_list,
                           std::vector<std::string> &manifest_files) {
    std::size_t last_found = 0;
    std::size_t found = search_path.find_first_of(PATH_SEPARATOR);
    std::string cur_search;

    try {
        // Handle any path listings in the string (separated by the appropriate path separator)
        while (found != std::string::npos) {
            // substr takes a start index and length.
            std::size_t length = found - last_found;
            cur_search = search_path.substr(last_found, length);

            CheckAllFilesInThePath(type, cur_search, is_directory_list, manifest_files);

            // This works around issue if multiple path separator follow each other directly.
            last_found = found;
            while (found == last_found) {
                std::ostringstream stringStream2;
                last_found = found + 1;
                found = search_path.find_first_of(PATH_SEPARATOR, last_found);
            }
        }

        // If there's something remaining in the string, copy it over
        if (last_found < search_path.size()) {
            cur_search = search_path.substr(last_found);
            CheckAllFilesInThePath(type, cur_search, is_directory_list, manifest_files);
        }
    } catch (...) {
        LoaderLogger::LogErrorMessage("", "AddFilesInPath - unknown error occurred");
        throw;
    }
}

// Copy all paths listed in the cur_path string into output_path and append the appropriate relative_path onto the end of each.
static void CopyIncludedPaths(bool is_directory_list, const std::string &cur_path, const std::string &relative_path,
                              std::string &output_path) {
    if (0 != cur_path.size()) {
        std::size_t last_found = 0;
        std::size_t found = cur_path.find_first_of(PATH_SEPARATOR);

        try {
            // Handle any path listings in the string (separated by the appropriate path separator)
            while (found != std::string::npos) {
                std::size_t length = found - last_found;
                output_path += cur_path.substr(last_found, length);
                if (is_directory_list && (cur_path[found - 1] != '\\' && cur_path[found - 1] != '/')) {
                    output_path += DIRECTORY_SYMBOL;
                }
                output_path += relative_path;
                output_path += PATH_SEPARATOR;

                last_found = found;
                found = cur_path.find_first_of(PATH_SEPARATOR, found + 1);
            }

            // If there's something remaining in the string, copy it over
            size_t last_char = cur_path.size() - 1;
            if (last_found != last_char) {
                output_path += cur_path.substr(last_found);
                if (is_directory_list && (cur_path[last_char] != '\\' && cur_path[last_char] != '/')) {
                    output_path += DIRECTORY_SYMBOL;
                }
                output_path += relative_path;
                output_path += PATH_SEPARATOR;
            }
        } catch (...) {
            LoaderLogger::LogErrorMessage("", "CopyIncludedPaths - unknown error occurred");
            throw;
        }
    }
}

// Look for data files in the provided paths, but first check the environment override to determine if we should use that instead.
static void ReadDataFilesInSearchPaths(ManifestFileType type, const std::string &override_env_var, const std::string &relative_path,
                                       bool &override_active, std::vector<std::string> &manifest_files) {
    bool is_directory_list = true;
    bool is_runtime = (type == MANIFEST_TYPE_RUNTIME);
    char *override_env = nullptr;
    std::string override_path = "";
    std::string search_path = "";

    try {
        if (override_env_var.size() != 0) {
#ifndef XR_OS_WINDOWS
            if (geteuid() != getuid() || getegid() != getgid()) {
                // Don't allow setuid apps to use the env var:
                override_env = nullptr;
            } else
#endif
            {
                override_env = PlatformUtilsGetSecureEnv(override_env_var.c_str());
                if (nullptr != override_env) {
                    // The runtime override is actually a specific list of filenames, not directories
                    if (is_runtime) {
                        is_directory_list = false;
                    }
                    override_path = override_env;
                }
            }
        }

        if (nullptr != override_env && override_path.size() != 0) {
            CopyIncludedPaths(is_directory_list, override_path, "", search_path);
            PlatformUtilsFreeEnv(override_env);
            override_active = true;
        } else {
            override_active = false;
#ifndef XR_OS_WINDOWS
            bool xdg_conf_dirs_alloc = true;
            bool xdg_data_dirs_alloc = true;
            const char home_additional[] = ".local/share/";

            // Determine how much space is needed to generate the full search path
            // for the current manifest files.
            char *xdg_conf_dirs = PlatformUtilsGetSecureEnv("XDG_CONFIG_DIRS");
            char *xdg_data_dirs = PlatformUtilsGetSecureEnv("XDG_DATA_DIRS");
            char *xdg_data_home = PlatformUtilsGetSecureEnv("XDG_DATA_HOME");
            char *home = PlatformUtilsGetSecureEnv("HOME");

            if (nullptr == xdg_conf_dirs) {
                xdg_conf_dirs_alloc = false;
            }
            if (nullptr == xdg_data_dirs) {
                xdg_data_dirs_alloc = false;
            }

            if (nullptr == xdg_conf_dirs || xdg_conf_dirs[0] == '\0') {
                CopyIncludedPaths(true, FALLBACK_CONFIG_DIRS, relative_path, search_path);
            } else {
                CopyIncludedPaths(true, xdg_conf_dirs, relative_path, search_path);
            }

            CopyIncludedPaths(true, SYSCONFDIR, relative_path, search_path);
#if defined(EXTRASYSCONFDIR)
            CopyIncludedPaths(true, EXTRASYSCONFDIR, relative_path, search_path);
#endif

            if (xdg_data_dirs == nullptr || xdg_data_dirs[0] == '\0') {
                CopyIncludedPaths(true, FALLBACK_DATA_DIRS, relative_path, search_path);
            } else {
                CopyIncludedPaths(true, xdg_data_dirs, relative_path, search_path);
            }

            if (nullptr != xdg_data_home) {
                CopyIncludedPaths(true, xdg_data_home, relative_path, search_path);
            } else if (nullptr != home) {
                std::string relative_home_path = home_additional;
                relative_home_path += relative_path;
                CopyIncludedPaths(true, home, relative_home_path, search_path);
            }

            if (xdg_conf_dirs_alloc) {
                PlatformUtilsFreeEnv(xdg_conf_dirs);
            }
            if (xdg_data_dirs_alloc) {
                PlatformUtilsFreeEnv(xdg_data_dirs);
            }
            if (nullptr != xdg_data_home) {
                PlatformUtilsFreeEnv(xdg_data_home);
            }
            if (nullptr != home) {
                PlatformUtilsFreeEnv(home);
            }
#endif
        }

        // Now, parse the paths and add any manifest files found in them.
        AddFilesInPath(type, search_path, is_directory_list, manifest_files);
    } catch (...) {
        LoaderLogger::LogErrorMessage("", "ReadDataFilesInSearchPaths - unknown error occurred");
        throw;
    }
}

#ifdef XR_OS_WINDOWS

// Look for runtime data files in the provided paths, but first check the environment override to determine
// if we should use that instead.
static void ReadRuntimeDataFilesInRegistry(ManifestFileType type, const std::string &runtime_registry_location,
                                           const std::string &default_runtime_value_name,
                                           std::vector<std::string> &manifest_files) {
    try {
        HKEY hkey;
        DWORD access_flags;
        char value[1024];
        DWORD value_size = 1023;

        // Generate the full registry location for the registry information
        std::string full_registry_location = OPENXR_REGISTRY_LOCATION;
        full_registry_location += std::to_string(XR_VERSION_MAJOR(XR_CURRENT_API_VERSION));
        full_registry_location += runtime_registry_location;

        // Use 64 bit regkey for 64bit application, and use 32 bit regkey in WOW for 32bit application.
        access_flags = KEY_QUERY_VALUE;
        LONG open_value = RegOpenKeyExA(HKEY_LOCAL_MACHINE, full_registry_location.c_str(), 0, access_flags, &hkey);

        if (ERROR_SUCCESS != open_value) {
            std::string warning_message = "ReadLayerDataFilesInRegistry - failed to read registry location ";
            warning_message += full_registry_location;
            LoaderLogger::LogWarningMessage("", warning_message);
        } else if (ERROR_SUCCESS == RegQueryValueExA(hkey, default_runtime_value_name.c_str(), NULL, NULL,
                                                     reinterpret_cast<LPBYTE>(&value), &value_size) &&
                   value_size < 1024) {
            AddFilesInPath(type, value, false, manifest_files);
        }
    } catch (...) {
        LoaderLogger::LogErrorMessage("", "ReadLayerDataFilesInRegistry - unknown error occurred");
        throw;
    }
}

// Look for layer data files in the provided paths, but first check the environment override to determine
// if we should use that instead.
static void ReadLayerDataFilesInRegistry(ManifestFileType type, const std::string &registry_location,
                                         std::vector<std::string> &manifest_files) {
    try {
        HKEY hive[2] = {HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER};
        bool found[2] = {false, false};
        HKEY hkey;
        DWORD access_flags;
        LONG rtn_value;
        char name[1024];
        DWORD value;
        DWORD name_size = 1023;
        DWORD value_size = sizeof(value);

        for (uint8_t hive_index = 0; hive_index < 2; ++hive_index) {
            DWORD key_index = 0;
            std::string full_registry_location = OPENXR_REGISTRY_LOCATION;
            full_registry_location += std::to_string(XR_VERSION_MAJOR(XR_CURRENT_API_VERSION));
            full_registry_location += registry_location;

            access_flags = KEY_QUERY_VALUE;
            LONG open_value = RegOpenKeyExA(hive[hive_index], full_registry_location.c_str(), 0, access_flags, &hkey);
            if (ERROR_SUCCESS != open_value) {
                if (hive_index == 1 && !found[0]) {
                    std::string warning_message = "ReadLayerDataFilesInRegistry - failed to read registry location ";
                    warning_message += registry_location;
                    warning_message += " in either HKEY_LOCAL_MACHINE or KHEY_CURRENT_USER";
                    LoaderLogger::LogWarningMessage("", warning_message);
                }
                continue;
            }
            found[hive_index] = true;
            while (ERROR_SUCCESS ==
                   (rtn_value = RegEnumValueA(hkey, key_index++, name, &name_size, NULL, NULL, (LPBYTE)&value, &value_size))) {
                if (value_size == sizeof(value) && value == 0) {
                    std::string filename = name;
                    AddFilesInPath(type, filename, false, manifest_files);
                }
                // Reset some items for the next loop
                name_size = 1023;
            }
        }
    } catch (...) {
        LoaderLogger::LogErrorMessage("", "ReadLayerDataFilesInRegistry - unknown error occurred");
        throw;
    }
}

#endif  // XR_OS_WINDOWS

ManifestFile::ManifestFile(ManifestFileType type, const std::string &filename, const std::string &library_path)
    : _filename(filename), _type(type), _library_path(library_path) {}

ManifestFile::~ManifestFile() {}

bool ManifestFile::IsValidJson(Json::Value &root_node, JsonVersion &version) {
    try {
        if (root_node["file_format_version"].isNull() || !root_node["file_format_version"].isString()) {
            LoaderLogger::LogErrorMessage("", "ManifestFile::IsValidJson - JSON file missing \"file_format_version\"");
            return false;
        }
        std::string file_format = root_node["file_format_version"].asString();
        sscanf(file_format.c_str(), "%d.%d.%d", &version.major, &version.minor, &version.patch);

        // Only version 1.0.0 is defined currently.  Eventually we may have more version, but
        // some of the versions may only be valid for layers or runtimes specifically.
        if (version.major != 1 || version.minor != 0 || version.patch != 0) {
            std::string error_message = "ManifestFile::IsValidJson - JSON \"file_format_version\" ";
            error_message += std::to_string(version.major);
            error_message += ".";
            error_message += std::to_string(version.minor);
            error_message += ".";
            error_message += std::to_string(version.patch);
            error_message += " is not supported";
            LoaderLogger::LogErrorMessage("", error_message);
            return false;
        }
    } catch (...) {
        LoaderLogger::LogErrorMessage("", "ManifestFile::IsValidJson - unknown error occurred");
        throw;
    }

    return true;
}

// Return any instance extensions found in the manifest files in the proper form for
// OpenXR (XrExtensionProperties).
void ManifestFile::GetInstanceExtensionProperties(std::vector<XrExtensionProperties> &props) {
    try {
        size_t ext_count = _instance_extensions.size();
        size_t props_count = props.size();
        bool found = false;
        for (size_t ext = 0; ext < ext_count; ++ext) {
            for (size_t prop = 0; prop < props_count; ++prop) {
                if (!strcmp(props[prop].extensionName, _instance_extensions[ext].name.c_str())) {
                    if (props[prop].specVersion < _instance_extensions[ext].spec_version) {
                        props[prop].specVersion = _instance_extensions[ext].spec_version;
                        found = true;
                        break;
                    }
                }
            }
            if (!found) {
                XrExtensionProperties prop = {};
                prop.type = XR_TYPE_EXTENSION_PROPERTIES;
                prop.next = nullptr;
                strncpy(prop.extensionName, _instance_extensions[ext].name.c_str(), XR_MAX_EXTENSION_NAME_SIZE - 1);
                prop.extensionName[XR_MAX_EXTENSION_NAME_SIZE - 1] = '\0';
                prop.specVersion = _instance_extensions[ext].spec_version;
                props.push_back(prop);
            }
        }
    } catch (...) {
        LoaderLogger::LogErrorMessage("", "ManifestFile::GetInstanceExtensionProperties - unknown error occurred");
        throw;
    }
}

// Return any device extensions found in the manifest files in the proper form for
// OpenXR (XrExtensionProperties).
void ManifestFile::GetDeviceExtensionProperties(std::vector<XrExtensionProperties> &props) {
    try {
        size_t ext_count = _device_extensions.size();
        size_t props_count = props.size();
        bool found = false;
        for (size_t ext = 0; ext < ext_count; ++ext) {
            for (size_t prop = 0; prop < props_count; ++prop) {
                if (!strcmp(props[prop].extensionName, _device_extensions[ext].name.c_str())) {
                    if (props[prop].specVersion < _device_extensions[ext].spec_version) {
                        props[prop].specVersion = _device_extensions[ext].spec_version;
                        found = true;
                        break;
                    }
                }
            }
            if (!found) {
                XrExtensionProperties prop = {};
                prop.type = XR_TYPE_EXTENSION_PROPERTIES;
                prop.next = nullptr;
                strncpy(prop.extensionName, _device_extensions[ext].name.c_str(), XR_MAX_EXTENSION_NAME_SIZE - 1);
                prop.extensionName[XR_MAX_EXTENSION_NAME_SIZE - 1] = '\0';
                prop.specVersion = _device_extensions[ext].spec_version;
                props.push_back(prop);
            }
        }
    } catch (...) {
        LoaderLogger::LogErrorMessage("", "ManifestFile::GetDeviceExtensionProperties - unknown error occurred");
        throw;
    }
}

const std::string &ManifestFile::GetFunctionName(const std::string &func_name) {
    try {
        if (_functions_renamed.size() > 0) {
            auto found = _functions_renamed.find(func_name);
            if (found != _functions_renamed.end()) {
                return found->second;
            }
        }
    } catch (...) {
        LoaderLogger::LogErrorMessage("", "ManifestFile::GetFunctionName - unknown error occurred");
        throw;
    }
    return func_name;
}

RuntimeManifestFile::RuntimeManifestFile(const std::string &filename, const std::string &library_path)
    : ManifestFile(MANIFEST_TYPE_RUNTIME, filename, library_path) {}

RuntimeManifestFile::~RuntimeManifestFile() {}

void RuntimeManifestFile::CreateIfValid(std::string filename, std::vector<std::unique_ptr<RuntimeManifestFile>> &manifest_files) {
    try {
        std::ifstream json_stream = std::ifstream(filename, std::ifstream::in);
        if (!json_stream.is_open()) {
            std::string error_message = "RuntimeManifestFile::createIfValid failed to open ";
            error_message += filename;
            error_message += ".  Does it exist?";
            LoaderLogger::LogErrorMessage("", error_message);
            return;
        }
        Json::Reader reader;
        Json::Value root_node = Json::nullValue;
        Json::Value runtime_root_node = Json::nullValue;
        JsonVersion file_version = {};
        if (!reader.parse(json_stream, root_node, false) || root_node.isNull()) {
            std::string error_message = "RuntimeManifestFile::CreateIfValid failed to parse ";
            error_message += filename;
            error_message += ".  Is it a valid runtime manifest file?";
            LoaderLogger::LogErrorMessage("", error_message);
            return;
        }
        if (!ManifestFile::IsValidJson(root_node, file_version)) {
            std::string error_message = "RuntimeManifestFile::CreateIfValid isValidJson indicates ";
            error_message += filename;
            error_message += " is not a valid manifest file.";
            LoaderLogger::LogErrorMessage("", error_message);
            return;
        }
        runtime_root_node = root_node["runtime"];
        // The Runtime manifest file needs the "runtime" root as well as sub-nodes for "api_version" and
        // "library_path".  If any of those aren't there, fail.
        if (runtime_root_node.isNull() || runtime_root_node["library_path"].isNull() ||
            !runtime_root_node["library_path"].isString()) {
            std::string error_message = "RuntimeManifestFile::CreateIfValid ";
            error_message += filename;
            error_message += " is missing required fields.  Verify all proper fields exist.";
            LoaderLogger::LogErrorMessage("", error_message);
            return;
        }

        std::string lib_path = runtime_root_node["library_path"].asString();

        // If the library_path variable has no directory symbol, it's just a file name and should be accessible on the
        // global library path.
        if (lib_path.find("\\") != lib_path.npos || lib_path.find("/") != lib_path.npos) {
            // If the library_path is an absolute path, just use that if it exists
            if (FileSysUtilsIsAbsolutePath(lib_path)) {
                if (!FileSysUtilsPathExists(lib_path)) {
                    std::string error_message = "RuntimeManifestFile::CreateIfValid ";
                    error_message += filename;
                    error_message += " library ";
                    error_message += lib_path;
                    error_message += " does not appear to exist";
                    LoaderLogger::LogErrorMessage("", error_message);
                    return;
                }
                lib_path = lib_path;
            } else {
                // Otherwise, treat the library path as a relative path based on the JSON file.
                std::string combined_path;
                std::string file_parent;
                if (!FileSysUtilsGetParentPath(filename, file_parent) ||
                    !FileSysUtilsCombinePaths(file_parent, lib_path, combined_path) || !FileSysUtilsPathExists(combined_path)) {
                    std::string error_message = "RuntimeManifestFile::CreateIfValid ";
                    error_message += filename;
                    error_message += " library ";
                    error_message += combined_path;
                    error_message += " does not appear to exist";
                    LoaderLogger::LogErrorMessage("", error_message);
                    return;
                }
                lib_path = combined_path;
            }
        }

        // Add this runtime manifest file
        manifest_files.emplace_back(new RuntimeManifestFile(filename, lib_path));

        // Add any extensions to it after the fact.
        Json::Value dev_exts = runtime_root_node["device_extensions"];
        if (!dev_exts.isNull() && dev_exts.isArray()) {
            for (Json::ValueIterator dev_ext_it = dev_exts.begin(); dev_ext_it != dev_exts.end(); ++dev_ext_it) {
                Json::Value dev_ext = (*dev_ext_it);
                Json::Value dev_ext_name = dev_ext["name"];
                Json::Value dev_ext_version = dev_ext["spec_version"];
                Json::Value dev_ext_entries = dev_ext["entrypoints"];
                if (!dev_ext_name.isNull() && dev_ext_name.isString() && !dev_ext_version.isNull() && dev_ext_version.isUInt() &&
                    !dev_ext_entries.isNull() && dev_ext_entries.isArray()) {
                    ExtensionListing ext = {};
                    ext.name = dev_ext_name.asString();
                    ext.spec_version = dev_ext_version.asUInt();
                    for (Json::ValueIterator entry_it = dev_ext_entries.begin(); entry_it != dev_ext_entries.end(); ++entry_it) {
                        Json::Value entry = (*entry_it);
                        if (!entry.isNull() && entry.isString()) {
                            ext.entrypoints.push_back(entry.asString());
                        }
                    }
                    manifest_files.back()->_device_extensions.push_back(ext);
                }
            }
        }

        Json::Value inst_exts = runtime_root_node["instance_extensions"];
        if (!inst_exts.isNull() && inst_exts.isArray()) {
            for (Json::ValueIterator inst_ext_it = inst_exts.begin(); inst_ext_it != inst_exts.end(); ++inst_ext_it) {
                Json::Value inst_ext = (*inst_ext_it);
                Json::Value inst_ext_name = inst_ext["name"];
                Json::Value inst_ext_version = inst_ext["spec_version"];
                if (!inst_ext_name.isNull() && inst_ext_name.isString() && !inst_ext_version.isNull() &&
                    inst_ext_version.isUInt()) {
                    ExtensionListing ext = {};
                    ext.name = inst_ext_name.asString();
                    ext.spec_version = inst_ext_version.asUInt();
                    manifest_files.back()->_instance_extensions.push_back(ext);
                }
            }
        }

        Json::Value funcs_renamed = runtime_root_node["functions"];
        if (!funcs_renamed.isNull() && funcs_renamed.size() > 0) {
            for (Json::ValueIterator func_it = funcs_renamed.begin(); func_it != funcs_renamed.end(); ++func_it) {
                if (!(*func_it).isString()) {
                    std::string warning_message = "RuntimeManifestFile::CreateIfValid ";
                    warning_message += filename;
                    warning_message += " \"functions\" section contains non-string values.";
                    LoaderLogger::LogWarningMessage("", warning_message);
                    continue;
                }
                std::string original_name = func_it.key().asString();
                std::string new_name = (*func_it).asString();
                manifest_files.back()->_functions_renamed.insert(std::make_pair(original_name, new_name));
            }
        }
    } catch (...) {
        LoaderLogger::LogErrorMessage("", "RuntimeManifestFile::CreateIfValid - unknown error occurred");
        throw;
    }
}

// Find all manifest files in the appropriate search paths/registries for the given type.
XrResult RuntimeManifestFile::FindManifestFiles(ManifestFileType type,
                                                std::vector<std::unique_ptr<RuntimeManifestFile>> &manifest_files) {
    XrResult result = XR_SUCCESS;
    try {
        if (MANIFEST_TYPE_RUNTIME != type) {
            LoaderLogger::LogErrorMessage("", "RuntimeManifestFile::FindManifestFiles - unknown manifest file requested");
            throw std::runtime_error("invalid manifest type");
        }
        std::string filename;
        const char *override_path = PlatformUtilsGetSecureEnv(OPENXR_RUNTIME_JSON_ENV_VAR);
        if (override_path != nullptr && *override_path != '\0') {
            filename = override_path;
            std::string info_message = "RuntimeManifestFile::FindManifestFiles - using environment variable override runtime file ";
            info_message += filename;
            LoaderLogger::LogInfoMessage("", info_message);
        } else {
#ifdef XR_OS_WINDOWS
            std::vector<std::string> filenames;
            ReadRuntimeDataFilesInRegistry(type, "", "ActiveRuntime", filenames);
            if (filenames.size() == 0) {
                LoaderLogger::LogErrorMessage(
                    "", "RuntimeManifestFile::FindManifestFiles - failed to find active runtime file in registry");
                return XR_ERROR_FILE_ACCESS_ERROR;
            }
            if (filenames.size() > 1) {
                LoaderLogger::LogWarningMessage(
                    "", "RuntimeManifestFile::FindManifestFiles - found too many default runtime files in registry");
            }
            filename = filenames[0];
#else
            if (!PlatformGetGlobalRuntimeFileName(XR_VERSION_MAJOR(XR_CURRENT_API_VERSION), filename)) {
                LoaderLogger::LogErrorMessage(
                    "",
                    "RuntimeManifestFile::FindManifestFiles - failed to determine active runtime file path for this environment");
                return XR_ERROR_FILE_ACCESS_ERROR;
            }
#endif
            std::string info_message = "RuntimeManifestFile::FindManifestFiles - using global runtime file ";
            info_message += filename;
            LoaderLogger::LogInfoMessage("", info_message);
        }
        RuntimeManifestFile::CreateIfValid(filename, manifest_files);
    } catch (std::bad_alloc &) {
        LoaderLogger::LogErrorMessage("", "RuntimeManifestFile::FindManifestFiles - failed to allocate memory");
        return XR_ERROR_OUT_OF_MEMORY;
    } catch (...) {
        LoaderLogger::LogErrorMessage("", "RuntimeManifestFile::FindManifestFiles - unknown error occurred");
        return XR_ERROR_FILE_ACCESS_ERROR;
    }
    return result;
}

ApiLayerManifestFile::ApiLayerManifestFile(ManifestFileType type, const std::string &filename, const std::string &layer_name,
                                           const std::string &description, const JsonVersion &api_version,
                                           const uint32_t &implementation_version, const std::string &library_path)
    : ManifestFile(type, filename, library_path),
      _api_version(api_version),
      _layer_name(layer_name),
      _description(description),
      _implementation_version(implementation_version) {}

ApiLayerManifestFile::~ApiLayerManifestFile() {}

void ApiLayerManifestFile::CreateIfValid(ManifestFileType type, std::string filename,
                                         std::vector<std::unique_ptr<ApiLayerManifestFile>> &manifest_files) {
    try {
        std::ifstream json_stream = std::ifstream(filename, std::ifstream::in);
        Json::Reader reader;
        Json::Value root_node = Json::nullValue;
        JsonVersion file_version = {};
        JsonVersion api_version = {};
        std::string layer_name = "";
        std::string library_path = "";
        std::string description = "";
        if (!reader.parse(json_stream, root_node, false) || root_node.isNull()) {
            std::string error_message = "ApiLayerManifestFile::CreateIfValid failed to parse ";
            error_message += filename;
            error_message += ".  Is it a valid layer manifest file?";
            LoaderLogger::LogErrorMessage("", error_message);
            return;
        }
        if (!ManifestFile::IsValidJson(root_node, file_version)) {
            std::string error_message = "ApiLayerManifestFile::CreateIfValid isValidJson indicates ";
            error_message += filename;
            error_message += " is not a valid manifest file.";
            LoaderLogger::LogErrorMessage("", error_message);
            return;
        }

        Json::Value layer_root_node = root_node["api_layer"];

        // The API Layer manifest file needs the "api_layer" root as well as other sub-nodes.
        // If any of those aren't there, fail.
        if (layer_root_node.isNull() || layer_root_node["name"].isNull() || !layer_root_node["name"].isString() ||
            layer_root_node["api_version"].isNull() || !layer_root_node["api_version"].isString() ||
            layer_root_node["library_path"].isNull() || !layer_root_node["library_path"].isString() ||
            layer_root_node["implementation_version"].isNull() || !layer_root_node["implementation_version"].isString()) {
            std::string error_message = "ApiLayerManifestFile::CreateIfValid ";
            error_message += filename;
            error_message += " is missing required fields.  Verify all proper fields exist.";
            LoaderLogger::LogErrorMessage("", error_message);
            return;
        }
        if (MANIFEST_TYPE_IMPLICIT_API_LAYER == type) {
            bool enabled = true;
            // Implicit layers require the disable environment variable.
            if (layer_root_node["disable_environment"].isNull() || !layer_root_node["disable_environment"].isString()) {
                std::string error_message = "ApiLayerManifestFile::CreateIfValid Implicit layer ";
                error_message += filename;
                error_message += " is missing \"disable_environment\"";
                LoaderLogger::LogErrorMessage("", error_message);
                return;
            }
            // Check if there's an enable environment variable provided
            if (!layer_root_node["enable_environment"].isNull() && layer_root_node["enable_environment"].isString()) {
                char *enable_val = PlatformUtilsGetEnv(layer_root_node["enable_environment"].asString().c_str());
                // If it's not set in the environment, disable the layer
                if (NULL == enable_val) {
                    enabled = false;
                }
                PlatformUtilsFreeEnv(enable_val);
            }
            // Check for the disable environment variable, which must be provided in the JSON
            char *disable_val = PlatformUtilsGetEnv(layer_root_node["disable_environment"].asString().c_str());
            // If the envar is set, disable the layer. Disable envar overrides enable above
            if (NULL != disable_val) {
                enabled = false;
            }
            PlatformUtilsFreeEnv(disable_val);

            // Not enabled, so pretend like it isn't even there.
            if (!enabled) {
                std::string info_message = "ApiLayerManifestFile::CreateIfValid Implicit layer ";
                info_message += filename;
                info_message += " is disabled";
                LoaderLogger::LogInfoMessage("", info_message);
                return;
            }
        }
        layer_name = layer_root_node["name"].asString();
        std::string api_version_string = layer_root_node["api_version"].asString();
        sscanf(api_version_string.c_str(), "%d.%d", &api_version.major, &api_version.minor);
        api_version.patch = 0;

        if ((api_version.major == 0 && api_version.minor == 0) || api_version.major > XR_VERSION_MAJOR(XR_CURRENT_API_VERSION)) {
            std::string warning_message = "ApiLayerManifestFile::CreateIfValid layer ";
            warning_message += filename;
            warning_message += " has invalid API Version.  Skipping layer.";
            LoaderLogger::LogWarningMessage("", warning_message);
            return;
        }

        uint32_t implementation_version = atoi(layer_root_node["implementation_version"].asString().c_str());
        library_path = layer_root_node["library_path"].asString();

        // If the library_path variable has no directory symbol, it's just a file name and should be accessible on the
        // global library path.
        if (library_path.find("\\") != library_path.npos || library_path.find("/") != library_path.npos) {
            // If the library_path is an absolute path, just use that if it exists
            if (FileSysUtilsIsAbsolutePath(library_path)) {
                if (!FileSysUtilsPathExists(library_path)) {
                    std::string error_message = "ApiLayerManifestFile::CreateIfValid ";
                    error_message += filename;
                    error_message += " library ";
                    error_message += library_path;
                    error_message += " does not appear to exist";
                    LoaderLogger::LogErrorMessage("", error_message);
                    return;
                }
                library_path = library_path;
            } else {
                // Otherwise, treat the library path as a relative path based on the JSON file.
                std::string combined_path;
                std::string file_parent;
                if (!FileSysUtilsGetParentPath(filename, file_parent) ||
                    !FileSysUtilsCombinePaths(file_parent, library_path, combined_path) || !FileSysUtilsPathExists(combined_path)) {
                    std::string error_message = "ApiLayerManifestFile::CreateIfValid ";
                    error_message += filename;
                    error_message += " library ";
                    error_message += combined_path;
                    error_message += " does not appear to exist";
                    LoaderLogger::LogErrorMessage("", error_message);
                    return;
                }
                library_path = combined_path;
            }
        }

        if (!layer_root_node["description"].isNull() && layer_root_node["description"].isString()) {
            description = layer_root_node["description"].asString();
        }

        // Add this layer manifest file
        manifest_files.emplace_back(
            new ApiLayerManifestFile(type, filename, layer_name, description, api_version, implementation_version, library_path));

        // Add any extensions to it after the fact.
        Json::Value dev_exts = layer_root_node["device_extensions"];
        if (!dev_exts.isNull() && dev_exts.isArray()) {
            for (Json::ValueIterator dev_ext_it = dev_exts.begin(); dev_ext_it != dev_exts.end(); ++dev_ext_it) {
                Json::Value dev_ext = (*dev_ext_it);
                Json::Value dev_ext_name = dev_ext["name"];
                Json::Value dev_ext_version = dev_ext["spec_version"];
                Json::Value dev_ext_entries = dev_ext["entrypoints"];
                if (!dev_ext_name.isNull() && dev_ext_name.isString() && !dev_ext_version.isNull() && dev_ext_version.isString() &&
                    !dev_ext_entries.isNull() && dev_ext_entries.isArray()) {
                    ExtensionListing ext = {};
                    ext.name = dev_ext_name.asString();
                    ext.spec_version = atoi(dev_ext_version.asString().c_str());
                    for (Json::ValueIterator entry_it = dev_ext_entries.begin(); entry_it != dev_ext_entries.end(); ++entry_it) {
                        Json::Value entry = (*entry_it);
                        if (!entry.isNull() && entry.isString()) {
                            ext.entrypoints.push_back(entry.asString());
                        }
                    }
                    manifest_files.back()->_device_extensions.push_back(ext);
                }
            }
        }

        Json::Value inst_exts = layer_root_node["instance_extensions"];
        if (!inst_exts.isNull() && inst_exts.isArray()) {
            for (Json::ValueIterator inst_ext_it = inst_exts.begin(); inst_ext_it != inst_exts.end(); ++inst_ext_it) {
                Json::Value inst_ext = (*inst_ext_it);
                Json::Value inst_ext_name = inst_ext["name"];
                Json::Value inst_ext_version = inst_ext["spec_version"];
                if (!inst_ext_name.isNull() && inst_ext_name.isString() && !inst_ext_version.isNull() &&
                    inst_ext_version.isString()) {
                    ExtensionListing ext = {};
                    ext.name = inst_ext_name.asString();
                    ext.spec_version = atoi(inst_ext_version.asString().c_str());
                    manifest_files.back()->_instance_extensions.push_back(ext);
                }
            }
        }

        Json::Value funcs_renamed = layer_root_node["functions"];
        if (!funcs_renamed.isNull() && funcs_renamed.size() > 0) {
            for (Json::ValueIterator func_it = funcs_renamed.begin(); func_it != funcs_renamed.end(); ++func_it) {
                if (!(*func_it).isString()) {
                    std::string warning_message = "ApiLayerManifestFile::CreateIfValid ";
                    warning_message += filename;
                    warning_message += " \"functions\" section contains non-string values.";
                    LoaderLogger::LogWarningMessage("", warning_message);
                    continue;
                }
                std::string original_name = func_it.key().asString();
                std::string new_name = (*func_it).asString();
                manifest_files.back()->_functions_renamed.insert(std::make_pair(original_name, new_name));
            }
        }
    } catch (...) {
        LoaderLogger::LogErrorMessage("", "ApiLayerManifestFile::CreateIfValid - unknown error occurred");
        throw;
    }
}

XrApiLayerProperties ApiLayerManifestFile::GetApiLayerProperties() {
    try {
        XrApiLayerProperties props = {};
        props.type = XR_TYPE_API_LAYER_PROPERTIES;
        props.next = nullptr;
        props.implementationVersion = _implementation_version;
        props.specVersion = XR_MAKE_VERSION(_api_version.major, _api_version.minor, _api_version.patch);
        strncpy(props.layerName, _layer_name.c_str(), XR_MAX_API_LAYER_NAME_SIZE - 1);
        if (_layer_name.size() >= XR_MAX_API_LAYER_NAME_SIZE - 1) {
            props.layerName[XR_MAX_API_LAYER_NAME_SIZE - 1] = '\0';
        }
        strncpy(props.description, _description.c_str(), XR_MAX_API_LAYER_DESCRIPTION_SIZE - 1);
        if (_description.size() >= XR_MAX_API_LAYER_DESCRIPTION_SIZE - 1) {
            props.description[XR_MAX_API_LAYER_DESCRIPTION_SIZE - 1] = '\0';
        }
        return props;
    } catch (...) {
        LoaderLogger::LogErrorMessage("", "ApiLayerManifestFile::GetApiLayerProperties - unknown error occurred");
        throw;
    }
}

// Find all layer manifest files in the appropriate search paths/registries for the given type.
XrResult ApiLayerManifestFile::FindManifestFiles(ManifestFileType type,
                                                 std::vector<std::unique_ptr<ApiLayerManifestFile>> &manifest_files) {
    try {
        std::string relative_path;
        std::string override_env_var;
        std::string registry_location = "";

        // Add the appropriate top-level folders for the relative path.  These should be
        // the string "openxr/" followed by the API major version as a string.
        relative_path = OPENXR_RELATIVE_PATH;
        relative_path += std::to_string(XR_VERSION_MAJOR(XR_CURRENT_API_VERSION));

        switch (type) {
            case MANIFEST_TYPE_IMPLICIT_API_LAYER:
                relative_path += OPENXR_IMPLICIT_API_LAYER_RELATIVE_PATH;
                override_env_var = "";
#ifdef XR_OS_WINDOWS
                registry_location = OPENXR_IMPLICIT_API_LAYER_REGISTRY_LOCATION;
#endif
                break;
            case MANIFEST_TYPE_EXPLICIT_API_LAYER:
                relative_path += OPENXR_EXPLICIT_API_LAYER_RELATIVE_PATH;
                override_env_var = OPENXR_API_LAYER_PATH_ENV_VAR;
#ifdef XR_OS_WINDOWS
                registry_location = OPENXR_EXPLICIT_API_LAYER_REGISTRY_LOCATION;
#endif
                break;
            default:
                LoaderLogger::LogErrorMessage("", "ApiLayerManifestFile::FindManifestFiles - unknown manifest file requested");
                throw std::runtime_error("invalid manifest type");
        }

        bool override_active = false;
        std::vector<std::string> filenames;
        ReadDataFilesInSearchPaths(type, override_env_var, relative_path, override_active, filenames);

#ifdef XR_OS_WINDOWS
        // Read the registry if the override wasn't active.
        if (!override_active) {
            ReadLayerDataFilesInRegistry(type, registry_location, filenames);
        }
#endif

        switch (type) {
            case MANIFEST_TYPE_IMPLICIT_API_LAYER:
            case MANIFEST_TYPE_EXPLICIT_API_LAYER:
                for (std::string &cur_file : filenames) {
                    ApiLayerManifestFile::CreateIfValid(type, cur_file, manifest_files);
                }
                break;
            default:
                break;
        }

    } catch (std::bad_alloc &) {
        LoaderLogger::LogErrorMessage("", "ApiLayerManifestFile::FindManifestFiles - memory allocation failed");
        return XR_ERROR_OUT_OF_MEMORY;
    } catch (...) {
        LoaderLogger::LogErrorMessage("", "ApiLayerManifestFile::FindManifestFiles - unknown error occurred");
        return XR_ERROR_FILE_ACCESS_ERROR;
    }
    return XR_SUCCESS;
}
