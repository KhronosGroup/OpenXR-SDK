#!/usr/bin/python3 -i
#
# Copyright (c) 2017-2019 The Khronos Group Inc.
# Copyright (c) 2017-2019 Valve Corporation
# Copyright (c) 2017-2019 LunarG, Inc.
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
#
# Author(s):    Mark Young <marky@lunarg.com>
#
# Purpose:      This file utilizes the content formatted in the
#               automatic_source_generator.py class to produce the
#               generated source code for the loader.

import os
import re
import sys
from automatic_source_generator import *
from collections import namedtuple


# The following commands should only exist in the loader, and only as a trampoline
# (i.e. Don't add it to the dispatch table)
NO_TRAMPOLINE_OR_TERMINATOR = [
    'xrEnumerateApiLayerProperties',
    'xrEnumerateInstanceExtensionProperties',
]

# UtilitySourceGeneratorOptions - subclass of AutomaticSourceGeneratorOptions.


class UtilitySourceGeneratorOptions(AutomaticSourceGeneratorOptions):
    def __init__(self,
                 conventions=None,
                 filename=None,
                 directory='.',
                 apiname=None,
                 profile=None,
                 versions='.*',
                 emitversions='.*',
                 defaultExtensions=None,
                 addExtensions=None,
                 removeExtensions=None,
                 emitExtensions=None,
                 sortProcedure=regSortFeatures,
                 prefixText="",
                 genFuncPointers=True,
                 protectFile=True,
                 protectFeature=True,
                 protectProto=None,
                 protectProtoStr=None,
                 apicall='',
                 apientry='',
                 apientryp='',
                 indentFuncProto=True,
                 indentFuncPointer=False,
                 alignFuncParam=0,
                 genEnumBeginEndRange=False):
        AutomaticSourceGeneratorOptions.__init__(self,
                                                 conventions=conventions,
                                                 filename=filename,
                                                 directory=directory,
                                                 apiname=apiname,
                                                 profile=profile,
                                                 versions=versions,
                                                 emitversions=emitversions,
                                                 defaultExtensions=defaultExtensions,
                                                 addExtensions=addExtensions,
                                                 removeExtensions=removeExtensions,
                                                 emitExtensions=emitExtensions,
                                                 sortProcedure=sortProcedure)
        # Instead of using prefixText, we write our own
        self.prefixText = None
        self.genFuncPointers = genFuncPointers
        self.protectFile = protectFile
        self.protectFeature = protectFeature
        self.protectProto = protectProto
        self.protectProtoStr = protectProtoStr
        self.apicall = apicall
        self.apientry = apientry
        self.apientryp = apientryp
        self.indentFuncProto = indentFuncProto
        self.indentFuncPointer = indentFuncPointer
        self.alignFuncParam = alignFuncParam
        self.genEnumBeginEndRange = genEnumBeginEndRange

# UtilitySourceOutputGenerator - subclass of AutomaticSourceOutputGenerator.


class UtilitySourceOutputGenerator(AutomaticSourceOutputGenerator):
    """Generate loader source using XML element attributes from registry"""

    def __init__(self,
                 errFile=sys.stderr,
                 warnFile=sys.stderr,
                 diagFile=sys.stdout):
        AutomaticSourceOutputGenerator.__init__(
            self, errFile, warnFile, diagFile)

    # Override the base class header warning so the comment indicates this file.
    #   self            the UtilitySourceOutputGenerator object
    def outputGeneratedHeaderWarning(self):
        generated_warning = '// *********** THIS FILE IS GENERATED - DO NOT EDIT ***********\n'
        generated_warning += '//     See utility_source_generator.py for modifications\n'
        generated_warning += '// ************************************************************\n'
        write(generated_warning, file=self.outFile)

    # Call the base class to properly begin the file, and then add
    # the file-specific header information.
    #   self            the UtilitySourceOutputGenerator object
    #   gen_opts        the UtilitySourceGeneratorOptions object
    def beginFile(self, genOpts):
        AutomaticSourceOutputGenerator.beginFile(self, genOpts)
        preamble = ''
        if self.genOpts.filename == 'xr_generated_dispatch_table.h':
            preamble += '#pragma once\n'
            preamble += '#include "xr_dependencies.h"\n'
            preamble += '#include <openxr/openxr.h>\n'
            preamble += '#include <openxr/openxr_platform.h>\n\n'
        elif self.genOpts.filename == 'xr_generated_dispatch_table.c':
            preamble += '#include "xr_dependencies.h"\n'
            preamble += '#include <openxr/openxr.h>\n'
            preamble += '#include <openxr/openxr_platform.h>\n\n'
            preamble += '#include "xr_generated_dispatch_table.h"\n'
        elif self.genOpts.filename == 'xr_generated_utilities.h':
            preamble += '#pragma once\n\n'
            preamble += '#include <openxr/openxr.h>\n\n'
        elif self.genOpts.filename == 'xr_generated_utilities.c':
            preamble += '#ifdef _WIN32\n'
            preamble += '// Disable Windows warning about using strncpy_s instead of strncpy\n'
            preamble += '#define  _CRT_SECURE_NO_WARNINGS 1\n'
            preamble += '#endif // _WIN32\n\n'
            preamble += '#include "xr_generated_utilities.h"\n\n'
            preamble += '#include <openxr/openxr.h>\n\n'
            preamble += '#include <stdio.h>\n'
            preamble += '#include <string.h>\n\n'
        write(preamble, file=self.outFile)

    # Write out all the information for the appropriate file,
    # and then call down to the base class to wrap everything up.
    #   self            the UtiliitySourceOutputGenerator object
    def endFile(self):
        file_data = ''

        if self.genOpts.filename == 'xr_generated_dispatch_table.h':
            file_data += '#ifdef __cplusplus\n'
            file_data += 'extern "C" { \n'
            file_data += '#endif\n'
            file_data += self.outputDispatchTable()
            file_data += self.outputDispatchPrototypes()
            file_data += '\n'
            file_data += '#ifdef __cplusplus\n'
            file_data += '} // extern "C"\n'
            file_data += '#endif\n'
        elif self.genOpts.filename == 'xr_generated_dispatch_table.c':
            file_data += '#ifdef __cplusplus\n'
            file_data += 'extern "C" { \n'
            file_data += '#endif\n'
            file_data += self.outputDispatchTableHelper()
            file_data += '#ifdef __cplusplus\n'
            file_data += '} // extern "C"\n'
            file_data += '#endif\n'
        elif self.genOpts.filename == 'xr_generated_utilities.h':
            file_data += '#ifdef __cplusplus\n'
            file_data += 'extern "C" { \n'
            file_data += '#endif\n'
            file_data += self.outputUtilityPrototypes()
            file_data += '#ifdef __cplusplus\n'
            file_data += '} // extern "C"\n'
            file_data += '#endif\n'
            file_data += self.outputUtilityVersionDefine()

        elif self.genOpts.filename == 'xr_generated_utilities.c':
            file_data += '#ifdef __cplusplus\n'
            file_data += 'extern "C" { \n'
            file_data += '#endif\n'
            file_data += self.outputUtilityFuncs()
            file_data += '#ifdef __cplusplus\n'
            file_data += '} // extern "C"\n'
            file_data += '#endif\n'

        write(file_data, file=self.outFile)

        # Finish processing in superclass
        AutomaticSourceOutputGenerator.endFile(self)

    # Create a prototype for each of the utility objects.
    #   self            the UtiliitySourceOutputGenerator object
    def outputUtilityPrototypes(self):
        utility_prototypes = '\n'
        utility_prototypes += 'XrResult GeneratedXrUtilitiesResultToString(XrResult result,\n'
        utility_prototypes += '                                            char buffer[XR_MAX_RESULT_STRING_SIZE]);\n'
        utility_prototypes += 'XrResult GeneratedXrUtilitiesStructureTypeToString(XrStructureType struct_type,\n'
        utility_prototypes += '                                                   char buffer[XR_MAX_STRUCTURE_NAME_SIZE]);\n'
        return utility_prototypes

    # Generate a variable the loader can use to indicate what API version it is.
    #   self            the UtiliitySourceOutputGenerator object
    def outputUtilityVersionDefine(self):
        cur_loader_version = '\n// Current API version of the utililties\n#define XR_UTILITIES_API_VERSION '
        cur_loader_version += self.api_version_define
        cur_loader_version += '\n'
        return cur_loader_version

    # A special-case handling of the "xrResultToString" command.  Since we can actually
    # do the work in the loader, write the command to convert from a result to the
    # appropriate string.  We need the command information from automatic_source_generator
    # so we can use the correct names for each parameter when writing the output.
    #   self            the UtiliitySourceOutputGenerator object
    def outputResultToString(self):
        result_to_str = ''
        count = 0
        result_to_str += 'XrResult GeneratedXrUtilitiesResultToString(XrResult result,\n'
        result_to_str += '                                            char buffer[XR_MAX_RESULT_STRING_SIZE]) {\n'
        indent = 1
        result_to_str += self.writeIndent(indent)
        result_to_str += 'XrResult int_result = XR_SUCCESS;\n'
        result_to_str += self.writeIndent(indent)
        result_to_str += 'switch (result) {\n'
        indent = indent + 1
        for enum_tuple in self.api_enums:
            if enum_tuple.name == 'XrResult':
                if enum_tuple.protect_value:
                    result_to_str += '#if %s\n' % enum_tuple.protect_string
                for cur_value in enum_tuple.values:
                    if cur_value.protect_value and enum_tuple.protect_value != cur_value.protect_value:
                        result_to_str += '#if %s\n' % cur_value.protect_string
                    result_to_str += self.writeIndent(indent)
                    result_to_str += 'case %s:\n' % cur_value.name
                    indent = indent + 1
                    result_to_str += self.writeIndent(indent)
                    result_to_str += 'strncpy(buffer, "%s", XR_MAX_RESULT_STRING_SIZE);\n' % cur_value.name
                    result_to_str += self.writeIndent(indent)
                    result_to_str += 'break;\n'
                    indent = indent - 1
                    if cur_value.protect_value and enum_tuple.protect_value != cur_value.protect_value:
                        result_to_str += '#endif // %s\n' % cur_value.protect_string
                    count = count + 1
                if enum_tuple.protect_value:
                    result_to_str += '#endif // %s\n' % enum_tuple.protect_string
                break
        result_to_str += self.writeIndent(indent)
        result_to_str += 'default:\n'
        result_to_str += self.writeIndent(indent + 1)
        result_to_str += '// Unknown result type\n'
        result_to_str += self.writeIndent(indent + 1)
        result_to_str += 'if (XR_SUCCEEDED(result)) {\n'
        result_to_str += self.writeIndent(indent + 2)
        result_to_str += 'snprintf(buffer, XR_MAX_RESULT_STRING_SIZE, "XR_UNKNOWN_SUCCESS_%d", result);\n'
        result_to_str += self.writeIndent(indent + 1)
        result_to_str += '} else {\n'
        result_to_str += self.writeIndent(indent + 2)
        result_to_str += 'snprintf(buffer, XR_MAX_RESULT_STRING_SIZE, "XR_UNKNOWN_FAILURE_%d", result);\n'
        result_to_str += self.writeIndent(indent + 1)
        result_to_str += '}\n'
        result_to_str += self.writeIndent(indent + 1)
        result_to_str += 'int_result = XR_ERROR_VALIDATION_FAILURE;\n'
        result_to_str += self.writeIndent(indent + 1)
        result_to_str += 'break;\n'
        indent = indent - 1
        result_to_str += self.writeIndent(indent)
        result_to_str += '}\n'
        result_to_str += self.writeIndent(indent)
        result_to_str += 'return int_result;\n'
        result_to_str += '}\n\n'
        return result_to_str

    # A special-case handling of the "StructureTypeToString" command.  Since we can actually
    # do the work in the loader, write the command to convert from a structure type to the
    # appropriate string.  We need the command information from automatic_source_generator
    # so we can use the correct names for each parameter when writing the output.
    #   self            the UtiliitySourceOutputGenerator object
    def outputStructTypeToString(self):
        struct_to_str = ''
        count = 0
        struct_to_str = 'XrResult GeneratedXrUtilitiesStructureTypeToString(XrStructureType struct_type,\n'
        struct_to_str += '                                            char buffer[XR_MAX_STRUCTURE_NAME_SIZE]) {\n'
        indent = 1
        struct_to_str += self.writeIndent(indent)
        struct_to_str += 'XrResult int_result = XR_SUCCESS;\n'
        struct_to_str += self.writeIndent(indent)
        struct_to_str += 'switch (struct_type) {\n'
        indent = indent + 1
        for enum_tuple in self.api_enums:
            if enum_tuple.name == 'XrStructureType':
                if enum_tuple.protect_value:
                    struct_to_str += '#if %s\n' % enum_tuple.protect_string
                for cur_value in enum_tuple.values:
                    if cur_value.protect_value and enum_tuple.protect_value != cur_value.protect_value:
                        struct_to_str += '#if %s\n' % cur_value.protect_string
                    struct_to_str += self.writeIndent(indent)
                    struct_to_str += 'case %s:\n' % cur_value.name
                    indent = indent + 1
                    struct_to_str += self.writeIndent(indent)
                    struct_to_str += 'strncpy(buffer, "%s", XR_MAX_STRUCTURE_NAME_SIZE);\n' % cur_value.name
                    struct_to_str += self.writeIndent(indent)
                    struct_to_str += 'break;\n'
                    indent = indent - 1
                    if cur_value.protect_value and enum_tuple.protect_value != cur_value.protect_value:
                        struct_to_str += '#endif // %s\n' % cur_value.protect_string
                    count = count + 1
                if enum_tuple.protect_value:
                    struct_to_str += '#endif // %s\n' % enum_tuple.protect_string
                break
        struct_to_str += self.writeIndent(indent)
        struct_to_str += 'default:\n'
        struct_to_str += self.writeIndent(indent + 1)
        struct_to_str += '// Unknown structure type\n'
        struct_to_str += self.writeIndent(indent + 1)
        struct_to_str += 'snprintf(buffer, XR_MAX_STRUCTURE_NAME_SIZE, "XR_UNKNOWN_STRUCTURE_TYPE_%d", struct_type);\n'
        struct_to_str += self.writeIndent(indent + 1)
        struct_to_str += 'int_result = XR_ERROR_VALIDATION_FAILURE;\n'
        struct_to_str += self.writeIndent(indent + 1)
        struct_to_str += 'break;\n'
        indent = indent - 1
        struct_to_str += self.writeIndent(indent)
        struct_to_str += '}\n'
        struct_to_str += self.writeIndent(indent)
        struct_to_str += 'return int_result;\n'
        struct_to_str += '}\n\n'
        return struct_to_str

    # Output utility generated functions.
    #   self            the UtiliitySourceOutputGenerator object
    def outputUtilityFuncs(self):
        utility_funcs = '\n'
        utility_funcs += self.outputResultToString()
        utility_funcs += self.outputStructTypeToString()
        return utility_funcs

    # Write out a prototype for a C-style command to populate a Dispatch table
    #   self            the ApiDumpOutputGenerator object
    def outputDispatchPrototypes(self):
        table_helper = '\n'
        table_helper += '// Prototype for dispatch table helper function\n'
        table_helper += 'void GeneratedXrPopulateDispatchTable(struct XrGeneratedDispatchTable *table,\n'
        table_helper += '                                      XrInstance instance,\n'
        table_helper += '                                      PFN_xrGetInstanceProcAddr get_inst_proc_addr);\n'
        return table_helper

    # Write out a C-style structure used to store the Dispatch table information
    #   self            the ApiDumpOutputGenerator object
    def outputDispatchTable(self):
        commands = []
        table = ''
        cur_extension_name = ''

        table += '// Generated dispatch table\n'
        table += 'struct XrGeneratedDispatchTable {\n'

        # Loop through both core commands, and extension commands
        # Outputting the core commands first, and then the extension commands.
        for x in range(0, 2):
            if x == 0:
                commands = self.core_commands
            else:
                commands = self.ext_commands

            for cur_cmd in commands:
                # If we've switched to a new "feature" print out a comment on what it is.  Usually,
                # this is a group of core commands or a group of commands in an extension.
                if cur_cmd.ext_name != cur_extension_name:
                    if self.isCoreExtensionName(cur_cmd.ext_name):
                        table += '\n    // ---- Core %s commands\n' % cur_cmd.ext_name[11:].replace(
                            "_", ".")
                    else:
                        table += '\n    // ---- %s extension commands\n' % cur_cmd.ext_name
                    cur_extension_name = cur_cmd.ext_name

                # Remove 'xr' from proto name
                base_name = cur_cmd.name[2:]

                # If a protect statement exists, use it.
                if cur_cmd.protect_value:
                    table += '#if %s\n' % cur_cmd.protect_string

                # Write out each command using it's function pointer for each command
                table += '    PFN_%s %s;\n' % (cur_cmd.name, base_name)

                # If a protect statement exists, wrap it up.
                if cur_cmd.protect_value:
                    table += '#endif // %s\n' % cur_cmd.protect_string
        table += '};\n\n'
        return table

    # Write out the helper function that will populate a dispatch table using
    # an instance handle and a corresponding xrGetInstanceProcAddr command.
    #   self            the ApiDumpOutputGenerator object
    def outputDispatchTableHelper(self):
        commands = []
        table_helper = ''
        cur_extension_name = ''

        table_helper += '// Helper function to populate an instance dispatch table\n'
        table_helper += 'void GeneratedXrPopulateDispatchTable(struct XrGeneratedDispatchTable *table,\n'
        table_helper += '                                      XrInstance instance,\n'
        table_helper += '                                      PFN_xrGetInstanceProcAddr get_inst_proc_addr) {\n'

        # Loop through both core commands, and extension commands
        # Outputting the core commands first, and then the extension commands.
        for x in range(0, 2):
            if x == 0:
                commands = self.core_commands
            else:
                commands = self.ext_commands

            for cur_cmd in commands:
                # If the command is only manually implemented in the loader,
                # it is not needed anywhere else, so skip it.
                if cur_cmd.name in NO_TRAMPOLINE_OR_TERMINATOR:
                    continue

                # If we've switched to a new "feature" print out a comment on what it is.  Usually,
                # this is a group of core commands or a group of commands in an extension.
                if cur_cmd.ext_name != cur_extension_name:
                    if self.isCoreExtensionName(cur_cmd.ext_name):
                        table_helper += '\n    // ---- Core %s commands\n' % cur_cmd.ext_name[11:].replace(
                            "_", ".")
                    else:
                        table_helper += '\n    // ---- %s extension commands\n' % cur_cmd.ext_name
                    cur_extension_name = cur_cmd.ext_name

                # Remove 'xr' from proto name
                base_name = cur_cmd.name[2:]

                if cur_cmd.protect_value:
                    table_helper += '#if %s\n' % cur_cmd.protect_string

                if cur_cmd.name == 'xrGetInstanceProcAddr':
                    # If the command we're filling in is the xrGetInstanceProcAddr command, use
                    # the one passed into this helper function.
                    table_helper += '    table->GetInstanceProcAddr = get_inst_proc_addr;\n'
                else:
                    # Otherwise, fill in the dispatch table with an xrGetInstanceProcAddr call
                    # to the appropriate command.
                    table_helper += '    (get_inst_proc_addr(instance, "%s", (PFN_xrVoidFunction*)&table->%s));\n' % (
                        cur_cmd.name, base_name)

                if cur_cmd.protect_value:
                    table_helper += '#endif // %s\n' % cur_cmd.protect_string
        table_helper += '}\n\n'
        return table_helper
