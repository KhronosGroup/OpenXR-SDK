#!/bin/bash
# Copyright (c) 2017-2019 The Khronos Group Inc.
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

set -e
(
    PREFERRED_CLANG_FORMAT=clang-format-5.0
    ACCEPTABLE_CLANG_FORMATS="${PREFERRED_CLANG_FORMAT} clang-format-6.0 clang-format-7 clang-format-8 clang-format"
    cd "$(dirname $0)"
    if [ ! "${CLANGFORMAT}" ]; then
        for tool in ${ACCEPTABLE_CLANG_FORMATS}; do
            if which $tool > /dev/null; then
                CLANGFORMAT=$tool
                break
            fi
        done
    fi
    if [ ! "${CLANGFORMAT}" ]; then
        echo "Could not find clang-format. Prefer ${PREFERRED_CLANG_FORMAT} but will accept newer." 1>&2
        echo "Looked for the names: ${ACCEPTABLE_CLANG_FORMATS}"
        exit 1
    fi
    echo "'Official' clang-format version recommended is ${PREFERRED_CLANG_FORMAT}. Currently using:"
    ${CLANGFORMAT} --version
    find . \( -wholename ./src/\* \) \
        -and -not \( -wholename ./src/external/\* \) \
        -and \( -name \*.hpp -or -name \*.h -or -name \*.cpp -or -name \*.c \) \
        -exec ${CLANGFORMAT} -i -style=file {} \;

)
