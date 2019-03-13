#!/usr/bin/python3 -i
#
# Copyright (c) 2013-2019 The Khronos Group Inc.
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

# Working-group-specific style conventions,
# used in generation.

from conventions import ConventionsBase


class OpenXRConventions(ConventionsBase):
    def formatExtension(self, name):
        """Mark up a name as an extension for the spec."""
        return '`<<{}>>`'.format(name)

    @property
    def null(self):
        """Preferred spelling of NULL."""
        return 'code:NULL'

    @property
    def struct_macro(self):
        return 'slink:'

    @property
    def external_macro(self):
        return 'code:'
