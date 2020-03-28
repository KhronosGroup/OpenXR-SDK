// Copyright (c) 2017 The Khronos Group Inc.
// Copyright (c) 2017 Valve Corporation
// Copyright (c) 2017 LunarG, Inc.
//
// SPDX-License-Identifier: Apache-2.0
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

#include <string>
#include <vector>

// Determine if the path indicates a regular file (not a directory or symbolic link)
bool FileSysUtilsIsRegularFile(const std::string& path);

// Determine if the path indicates a directory
bool FileSysUtilsIsDirectory(const std::string& path);

// Determine if the provided path exists on the filesystem
bool FileSysUtilsPathExists(const std::string& path);

// Get the current directory
bool FileSysUtilsGetCurrentPath(std::string& path);

// Get the parent path of a file
bool FileSysUtilsGetParentPath(const std::string& file_path, std::string& parent_path);

// Determine if the provided path is an absolute path
bool FileSysUtilsIsAbsolutePath(const std::string& path);

// Get the absolute path for a provided file
bool FileSysUtilsGetAbsolutePath(const std::string& path, std::string& absolute);

// Combine a parent and child directory
bool FileSysUtilsCombinePaths(const std::string& parent, const std::string& child, std::string& combined);

// Parse out individual paths in a path list
bool FileSysUtilsParsePathList(std::string& path_list, std::vector<std::string>& paths);

// Record all the filenames for files found in the provided path.
bool FileSysUtilsFindFilesInPath(const std::string& path, std::vector<std::string>& files);
