#!/usr/bin/python3
#
# Copyright (c) 2018-2019 The Khronos Group Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Used to update the version in the appropriate place.  Uses the
# 'current_version.txt' file in the the openxr/specification folder.
# To execute, run this script from the specification folder with a clean
# tree.

# Usage: python3 ./scripts/update_version.py

import sys
import re
import fileinput

# Get the current version from the 'curent_version.txt' file.
#
major_version='0'
minor_version='0'
patch_version='0'
version_file = open("current_version.txt", "r")
for line in version_file:
    split_string = line.split('=')
    if split_string[0] == 'MAJOR':
        major_version = split_string[1].rstrip()
    elif split_string[0] == 'MINOR':
        minor_version = split_string[1].rstrip()
    elif split_string[0] == 'PATCH':
        patch_version = split_string[1].rstrip()
version_file.close()

# Now update the version in the appropriate places in the
# registry file (registry/xr.xml).
#
print('Replacing version lines in the registry')
for line in fileinput.input('registry/xr.xml', inplace=True):
    printed = False
    if 'XR_CURRENT_API_VERSION' in line:
        if 'XR_MAKE_VERSION' in line:
            printed = True
            print('#define <name>XR_CURRENT_API_VERSION</name> <type>XR_MAKE_VERSION</type>(%s, %s, %s)</type>' % (major_version, minor_version, patch_version))
        if 'type name' in line:
            printed = True
            print('            <type name="XR_CURRENT_API_VERSION"/>')
    elif 'XR_VERSION_' in line and 'feature' in line and 'number' in line and 'openxr' in line:
        printed = True
        print('    <feature api="openxr" name="XR_VERSION_%s_%s" number="%s.%s">' % (major_version, minor_version, major_version, minor_version))
    elif 'XR_HEADER_VERSION' in line and 'define' in line:
        printed = True
        # NOTE: The space before the %s is needed or else it runs up against the "XR_HEADER_VERSION" define when
        #       exported to openxr.h.
        print('#define <name>XR_HEADER_VERSION</name> %s</type>' % patch_version)
    if not printed:
        print("%s" % (line), end='')

# Now update the version in the appropriate places in the
# specification make file (Makefile).
#
print('Replacing version lines in the specification Makefile')
for line in fileinput.input('Makefile', inplace=True):
    printed = False
    if 'SPECREVISION = ' in line:
        printed = True
        print('SPECREVISION = %s.%s.%s' % (major_version, minor_version, patch_version))
    if not printed:
        print("%s" % (line), end='')

