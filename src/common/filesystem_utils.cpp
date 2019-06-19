// Copyright (c) 2017 The Khronos Group Inc.
// Copyright (c) 2017 Valve Corporation
// Copyright (c) 2017 LunarG, Inc.
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
// Authors: Mark Young <marky@lunarg.com>
//          Nat Brown <natb@valvesoftware.com>
//

#include <cstring>
#include "xr_dependencies.h"

#if defined DISABLE_STD_FILESYSTEM
#define USE_EXPERIMENTAL_FS 0
#define USE_FINAL_FS 0

#else
// If the C++ macro is set to the version containing C++17, it must support
// the final C++17 package
#if __cplusplus >= 201703L
#define USE_EXPERIMENTAL_FS 0
#define USE_FINAL_FS 1

#elif defined(_MSC_VER) && _MSC_VER >= 1900

#if defined(_HAS_CXX17) && _HAS_CXX17
// When MSC supports c++17 use <filesystem> package.
#define USE_EXPERIMENTAL_FS 0
#define USE_FINAL_FS 1
#else
// MSC before c++17 need to use <experimental/filesystem> package.
#define USE_EXPERIMENTAL_FS 1
#define USE_FINAL_FS 0
#endif  // !_HAS_CXX17

// Right now, GCC still only supports the experimental filesystem items starting in GCC 6
#elif (__GNUC__ >= 6)
#define USE_EXPERIMENTAL_FS 1
#define USE_FINAL_FS 0

// If Clang, check for feature support
#elif defined(__clang__) && (__cpp_lib_filesystem || __cpp_lib_experimental_filesystem)
#if __cpp_lib_filesystem
#define USE_EXPERIMENTAL_FS 0
#define USE_FINAL_FS 1
#else
#define USE_EXPERIMENTAL_FS 1
#define USE_FINAL_FS 0
#endif

// If all above fails, fall back to standard C++ and OS-specific items
#else
#define USE_EXPERIMENTAL_FS 0
#define USE_FINAL_FS 0
#endif
#endif

#if USE_FINAL_FS == 1
#include <filesystem>
#define FS_PREFIX std::filesystem
#elif USE_EXPERIMENTAL_FS == 1
#if (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
#error "Windows universal application doesn't support system::experimental::filesystem"
#endif
#include <experimental/filesystem>
#define FS_PREFIX std::experimental::filesystem
#elif defined(XR_USE_PLATFORM_WIN32)
// Windows fallback includes
#include <stdint.h>
#include <direct.h>
#include <shlwapi.h>
#else
// Linux/Apple fallback includes
#include <sys/stat.h>
#include <sys/param.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <dirent.h>
#endif

#include "filesystem_utils.hpp"

#if defined(XR_USE_PLATFORM_WIN32)
#define PATH_SEPARATOR ';'
#define DIRECTORY_SYMBOL '\\'
#else
#define PATH_SEPARATOR ':'
#define DIRECTORY_SYMBOL '/'
#endif

#if (USE_FINAL_FS == 1) || (USE_EXPERIMENTAL_FS == 1)
// We can use one of the C++ filesystem packages

bool FileSysUtilsIsRegularFile(const std::string& path) {
    try {
        return FS_PREFIX::is_regular_file(path);
    } catch (...) {
        return false;
    }
}

bool FileSysUtilsIsDirectory(const std::string& path) {
    try {
        return FS_PREFIX::is_directory(path);
    } catch (...) {
        return false;
    }
}

bool FileSysUtilsPathExists(const std::string& path) {
    try {
        return FS_PREFIX::exists(path);
    } catch (...) {
        return false;
    }
}

bool FileSysUtilsIsAbsolutePath(const std::string& path) {
    try {
        FS_PREFIX::path file_path(path);
        return file_path.is_absolute();
    } catch (...) {
        return false;
    }
}

bool FileSysUtilsGetCurrentPath(std::string& path) {
    try {
        FS_PREFIX::path cur_path = FS_PREFIX::current_path();
        path = cur_path.string();
        return true;
    } catch (...) {
    }
    return false;
}

bool FileSysUtilsGetParentPath(const std::string& file_path, std::string& parent_path) {
    try {
        FS_PREFIX::path path_var(file_path);
        parent_path = path_var.parent_path().string();
        return true;
    } catch (...) {
    }
    return false;
}

bool FileSysUtilsGetAbsolutePath(const std::string& path, std::string& absolute) {
    try {
        absolute = FS_PREFIX::absolute(path).string();
        return true;
    } catch (...) {
    }
    return false;
}

bool FileSysUtilsCombinePaths(const std::string& parent, const std::string& child, std::string& combined) {
    try {
        FS_PREFIX::path parent_path(parent);
        FS_PREFIX::path child_path(child);
        FS_PREFIX::path full_path = parent_path / child_path;
        combined = full_path.string();
        return true;
    } catch (...) {
    }
    return false;
}

bool FileSysUtilsParsePathList(std::string& path_list, std::vector<std::string>& paths) {
    try {
        std::string::size_type start = 0;
        std::string::size_type location = path_list.find(PATH_SEPARATOR);
        while (location != std::string::npos) {
            paths.push_back(path_list.substr(start, location));
            start = location + 1;
            location = path_list.find(PATH_SEPARATOR, start);
        }
        paths.push_back(path_list.substr(start, location));
        return true;
    } catch (...) {
    }
    return false;
}

bool FileSysUtilsFindFilesInPath(const std::string& path, std::vector<std::string>& files) {
    try {
        for (auto& dir_iter : FS_PREFIX::directory_iterator(path)) {
            files.push_back(dir_iter.path().filename().string());
        }
        return true;
    } catch (...) {
    }
    return false;
}

#elif defined(XR_OS_WINDOWS)

// Workaround for MS VS 2010/2013 don't support the experimental filesystem

bool FileSysUtilsIsRegularFile(const std::string& path) {
    try {
        return (1 != PathIsDirectoryA(path.c_str()));
    } catch (...) {
        return false;
    }
}

bool FileSysUtilsIsDirectory(const std::string& path) {
    try {
        return (1 == PathIsDirectoryA(path.c_str()));
    } catch (...) {
        return false;
    }
}

bool FileSysUtilsPathExists(const std::string& path) {
    try {
        return (1 == PathFileExistsA(path.c_str()));
    } catch (...) {
        return false;
    }
}

bool FileSysUtilsIsAbsolutePath(const std::string& path) {
    try {
        if ((path[0] == '\\') || (path[1] == ':' && (path[2] == '\\' || path[2] == '/'))) {
            return true;
        }
    } catch (...) {
    }
    return false;
}

bool FileSysUtilsGetCurrentPath(std::string& path) {
    try {
        char tmp_path[MAX_PATH];
        if (nullptr != _getcwd(tmp_path, MAX_PATH - 1)) {
            path = tmp_path;
            return true;
        }
    } catch (...) {
    }
    return false;
}

bool FileSysUtilsGetParentPath(const std::string& file_path, std::string& parent_path) {
    try {
        std::string full_path;
        if (FileSysUtilsGetAbsolutePath(file_path, full_path)) {
            std::string::size_type lastSeperator = full_path.find_last_of(DIRECTORY_SYMBOL);
            parent_path = (lastSeperator == 0) ? full_path : full_path.substr(0, lastSeperator - 1);
            return true;
        }
    } catch (...) {
    }
    return false;
}

bool FileSysUtilsGetAbsolutePath(const std::string& path, std::string& absolute) {
    try {
        char tmp_path[MAX_PATH];
        if (0 != GetFullPathNameA(path.c_str(), MAX_PATH, tmp_path, NULL)) {
            absolute = tmp_path;
            return true;
        }
    } catch (...) {
    }
    return false;
}

bool FileSysUtilsCombinePaths(const std::string& parent, const std::string& child, std::string& combined) {
    try {
        std::string::size_type parent_len = parent.length();
        if (0 == parent_len || "." == parent || ".\\" == parent || "./" == parent) {
            combined = child;
            return true;
        }
        char last_char = parent[parent_len - 1];
        if (last_char == DIRECTORY_SYMBOL) {
            parent_len--;
        }
        combined = parent.substr(0, parent_len) + DIRECTORY_SYMBOL + child;
        return true;
    } catch (...) {
    }
    return false;
}

bool FileSysUtilsParsePathList(std::string& path_list, std::vector<std::string>& paths) {
    try {
        std::string::size_type start = 0;
        std::string::size_type location = path_list.find(PATH_SEPARATOR);
        while (location != std::string::npos) {
            paths.push_back(path_list.substr(start, location));
            start = location + 1;
            location = path_list.find(PATH_SEPARATOR, start);
        }
        paths.push_back(path_list.substr(start, location));
        return true;
    } catch (...) {
    }
    return false;
}

bool FileSysUtilsFindFilesInPath(const std::string& path, std::vector<std::string>& files) {
    try {
        WIN32_FIND_DATAA file_data;
        HANDLE file_handle = FindFirstFileA(path.c_str(), &file_data);
        if (file_handle != INVALID_HANDLE_VALUE) {
            do {
                if (!(file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    files.push_back(file_data.cFileName);
                }
            } while (FindNextFileA(file_handle, &file_data));
            return true;
        }
    } catch (...) {
    }
    return false;
}

#else  // XR_OS_LINUX/XR_OS_APPLE fallback

#include <sys/stat.h>
#include <sys/param.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>

// simple POSIX-compatible implementation of the <filesystem> pieces used by OpenXR

bool FileSysUtilsIsRegularFile(const std::string& path) {
    try {
        struct stat path_stat;
        stat(path.c_str(), &path_stat);
        return S_ISREG(path_stat.st_mode);
    } catch (...) {
    }
    return false;
}

bool FileSysUtilsIsDirectory(const std::string& path) {
    try {
        struct stat path_stat;
        stat(path.c_str(), &path_stat);
        return S_ISDIR(path_stat.st_mode);
    } catch (...) {
    }
    return false;
}

bool FileSysUtilsPathExists(const std::string& path) {
    try {
        return (access(path.c_str(), F_OK) != -1);
    } catch (...) {
    }
    return false;
}

bool FileSysUtilsIsAbsolutePath(const std::string& path) {
    try {
        return (path[0] == DIRECTORY_SYMBOL);
    } catch (...) {
    }
    return false;
}

bool FileSysUtilsGetCurrentPath(std::string& path) {
    try {
        char tmp_path[PATH_MAX];
        if (nullptr != getcwd(tmp_path, PATH_MAX - 1)) {
            path = tmp_path;
            return true;
        }
    } catch (...) {
    }
    return false;
}

bool FileSysUtilsGetParentPath(const std::string& file_path, std::string& parent_path) {
    try {
        std::string full_path;
        if (FileSysUtilsGetAbsolutePath(file_path, full_path)) {
            std::string::size_type lastSeperator = full_path.find_last_of(DIRECTORY_SYMBOL);
            parent_path = (lastSeperator == 0) ? full_path : full_path.substr(0, lastSeperator - 1);
            return true;
        }
    } catch (...) {
    }
    return false;
}

bool FileSysUtilsGetAbsolutePath(const std::string& path, std::string& absolute) {
    try {
        char buf[PATH_MAX];
        if (nullptr != realpath(path.c_str(), buf)) {
            absolute = buf;
            return true;
        }
    } catch (...) {
    }
    return false;
}

bool FileSysUtilsCombinePaths(const std::string& parent, const std::string& child, std::string& combined) {
    try {
        std::string::size_type parent_len = parent.length();
        if (0 == parent_len || "." == parent || "./" == parent) {
            combined = child;
            return true;
        }
        char last_char = parent[parent_len - 1];
        if (last_char == DIRECTORY_SYMBOL) {
            parent_len--;
        }
        combined = parent.substr(0, parent_len) + DIRECTORY_SYMBOL + child;
        return true;
    } catch (...) {
    }
    return false;
}

bool FileSysUtilsParsePathList(std::string& path_list, std::vector<std::string>& paths) {
    try {
        std::string::size_type start = 0;
        std::string::size_type location = path_list.find(PATH_SEPARATOR);
        while (location != std::string::npos) {
            paths.push_back(path_list.substr(start, location));
            start = location + 1;
            location = path_list.find(PATH_SEPARATOR, start);
        }
        paths.push_back(path_list.substr(start, location));
        return true;
    } catch (...) {
    }
    return false;
}

bool FileSysUtilsFindFilesInPath(const std::string& path, std::vector<std::string>& files) {
    try {
        DIR* dir;
        struct dirent* entry;
        dir = opendir(path.c_str());
        while (dir && (entry = readdir(dir))) {
            files.push_back(entry->d_name);
        }
        closedir(dir);
        return true;
    } catch (...) {
    }
    return false;
}

#endif
