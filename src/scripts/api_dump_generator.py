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
#               generated source code for the API Dump layer.

import os
import re
import sys
from automatic_source_generator import *
from collections import namedtuple

# The following commands should not be generated for the layer
MANUALLY_DEFINED_IN_LAYER = [
    'xrCreateInstance',
    'xrDestroyInstance',
]

DONT_GEN_IN_LAYER = [
    'xrEnumerateApiLayerProperties',
    'xrEnumerateInstanceExtensionProperties',
]

# ApiDumpGeneratorOptions - subclass of AutomaticSourceGeneratorOptions.


class ApiDumpGeneratorOptions(AutomaticSourceGeneratorOptions):
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
        AutomaticSourceGeneratorOptions.__init__(self, conventions, filename, directory, apiname, profile,
                                                 versions, emitversions, defaultExtensions,
                                                 addExtensions, removeExtensions,
                                                 emitExtensions, sortProcedure)

# ApiDumpOutputGenerator - subclass of AutomaticSourceOutputGenerator.


class ApiDumpOutputGenerator(AutomaticSourceOutputGenerator):
    """Generate API Dump layer source using XML element attributes from registry"""

    def __init__(self,
                 errFile=sys.stderr,
                 warnFile=sys.stderr,
                 diagFile=sys.stdout):
        AutomaticSourceOutputGenerator.__init__(
            self, errFile, warnFile, diagFile)

    # Override the base class header warning so the comment indicates this file.
    #   self            the AutomaticSourceOutputGenerator object
    def outputGeneratedHeaderWarning(self):
        # File Comment
        generated_warning = '// *********** THIS FILE IS GENERATED - DO NOT EDIT ***********\n'
        generated_warning += '//     See api_dump_generator.py for modifications\n'
        generated_warning += '// ************************************************************\n'
        write(generated_warning, file=self.outFile)

    # Call the base class to properly begin the file, and then add
    # the file-specific header information.
    #   self            the ApiDumpOutputGenerator object
    #   gen_opts        the ApiDumpGeneratorOptions object
    def beginFile(self, genOpts):
        AutomaticSourceOutputGenerator.beginFile(self, genOpts)
        preamble = ''
        if self.genOpts.filename == 'xr_generated_api_dump.hpp':
            preamble += '#pragma once\n\n'
            preamble += '#include <string>\n'
            preamble += '#include <vector>\n'
            preamble += '#include <tuple>\n\n'
            preamble += '#include "api_layer_platform_defines.h"\n'
            preamble += '#include <openxr/openxr.h>\n'
            preamble += '#include <openxr/openxr_platform.h>\n\n'
            preamble += '#include "xr_generated_dispatch_table.h"\n'
        elif self.genOpts.filename == 'xr_generated_api_dump.cpp':
            preamble += '#include <cstring>\n'
            preamble += '#include <mutex>\n'
            preamble += '#include <sstream>\n'
            preamble += '#include <iomanip>\n'
            preamble += '#include <unordered_map>\n\n'
            preamble += '#include "xr_generated_api_dump.hpp"\n'
            preamble += '#include "hex_and_handles.h"\n'
        write(preamble, file=self.outFile)

    # Write out all the information for the appropriate file,
    # and then call down to the base class to wrap everything up.
    #   self            the ApiDumpOutputGenerator object
    def endFile(self):
        file_data = ''
        if self.genOpts.filename == 'xr_generated_api_dump.hpp':
            file_data += self.outputLayerHeaderPrototypes()
            file_data += self.outputApiDumpExterns()

        elif self.genOpts.filename == 'xr_generated_api_dump.cpp':
            file_data += self.outputApiDumpMapMutexItems()
            file_data += self.writeApiDumpUnionStructFuncs()
            file_data += self.outputLayerCommands()

        write(file_data, file=self.outFile)

        # Finish processing in superclass
        AutomaticSourceOutputGenerator.endFile(self)

    # Output the externs required by the manual code to work with the API Dump
    # gnerated code.
    #   self            the ApiDumpOutputGenerator object
    def outputApiDumpExterns(self):
        externs = '\n// Externs for API dump\n'
        for handle in self.api_handles:
            base_handle_name = undecorate(handle.name)
            if handle.protect_value is not None:
                externs += '#if %s\n' % handle.protect_string
            externs += 'extern std::unordered_map<%s, XrGeneratedDispatchTable*> g_%s_dispatch_map;\n' % (
                handle.name, base_handle_name)
            externs += 'extern std::mutex g_%s_dispatch_mutex;\n' % base_handle_name
            if handle.protect_value is not None:
                externs += '#endif // %s\n' % handle.protect_string
        externs += 'void ApiDumpCleanUpMapsForTable(XrGeneratedDispatchTable *table);\n'
        return externs

    # Output the externs manually implemented by the API Dump layer so that the generated code
    # can access them.
    #   self            the ApiDumpOutputGenerator object
    def outputLayerHeaderPrototypes(self):
        generated_prototypes = '// Layer\'s xrGetInstanceProcAddr\n'
        generated_prototypes += 'XrResult ApiDumpLayerXrGetInstanceProcAddr(XrInstance instance,\n'
        generated_prototypes += '                                          const char* name, PFN_xrVoidFunction* function);\n\n'
        generated_prototypes += '// Api Dump Log Command\n'
        generated_prototypes += 'bool ApiDumpLayerRecordContent(std::vector<std::tuple<std::string, std::string, std::string>> contents);\n\n'
        generated_prototypes += '// Api Dump Manual Functions\n'
        generated_prototypes += 'XrInstance FindInstanceFromDispatchTable(XrGeneratedDispatchTable* dispatch_table);\n'
        generated_prototypes += 'XrResult ApiDumpLayerXrCreateInstance(const XrInstanceCreateInfo *info,\n'
        generated_prototypes += '                                      XrInstance *instance);\n'
        generated_prototypes += 'XrResult ApiDumpLayerXrDestroyInstance(XrInstance instance);\n'
        generated_prototypes += '\n//Dump utility functions\n'
        generated_prototypes += 'bool ApiDumpDecodeNextChain(XrGeneratedDispatchTable* gen_dispatch_table, const void* value, std::string prefix,\n'
        generated_prototypes += '                            std::vector<std::tuple<std::string, std::string, std::string>> &contents);\n'
        generated_prototypes += '\n// Union/Structure Output Helper function prototypes\n'
        for xr_union in self.api_unions:
            if xr_union.protect_value:
                generated_prototypes += '#if %s\n' % xr_union.protect_string
            generated_prototypes += 'bool ApiDumpOutputXrUnion(XrGeneratedDispatchTable* gen_dispatch_table, const %s* value,\n' % xr_union.name
            generated_prototypes += '                          std::string prefix, std::string type_string, bool is_pointer,\n'
            generated_prototypes += '                          std::vector<std::tuple<std::string, std::string, std::string>> &contents);\n'
            if xr_union.protect_value:
                generated_prototypes += '#endif // %s\n' % xr_union.protect_string
        for xr_struct in self.api_structures:
            if xr_struct.protect_value:
                generated_prototypes += '#if %s\n' % xr_struct.protect_string
            generated_prototypes += 'bool ApiDumpOutputXrStruct(XrGeneratedDispatchTable* gen_dispatch_table, const %s* value,\n' % xr_struct.name
            generated_prototypes += '                           std::string prefix, std::string type_string, bool is_pointer,\n'
            generated_prototypes += '                           std::vector<std::tuple<std::string, std::string, std::string>> &contents);\n'
            if xr_struct.protect_value:
                generated_prototypes += '#endif // %s\n' % xr_struct.protect_string
        return generated_prototypes

    # Output the unordered_map's required to track all the data we need per handle type.  Also, create
    # a mutex that allows us to access the unordered_maps in a thread-safe manner.  Finally, wrap it
    # all up by creating utility functions for cleaning up a dispatch table when it's instance has
    # been deleted.
    #   self            the ApiDumpOutputGenerator object
    def outputApiDumpMapMutexItems(self):
        maps_mutexes = ''
        for handle in self.api_handles:
            base_handle_name = undecorate(handle.name)
            if handle.protect_value:
                maps_mutexes += '#if %s\n' % handle.protect_string
            maps_mutexes += 'std::unordered_map<%s, XrGeneratedDispatchTable*> g_%s_dispatch_map;\n' % (
                handle.name, base_handle_name)
            maps_mutexes += 'std::mutex g_%s_dispatch_mutex;\n' % base_handle_name
            if handle.protect_value:
                maps_mutexes += '#endif // %s\n' % handle.protect_string
        maps_mutexes += '\n'
        maps_mutexes += '// Template function to reduce duplicating the map locking, searching, and deleting.`\n'
        maps_mutexes += 'template <typename MapType>\n'
        maps_mutexes += 'void eraseAllTableMapElements(MapType &search_map, std::mutex &mutex, XrGeneratedDispatchTable *search_value) {\n'
        maps_mutexes += '    std::unique_lock<std::mutex> lock(mutex);\n'
        maps_mutexes += '    for (auto it = search_map.begin(); it != search_map.end();) {\n'
        maps_mutexes += '        if (it->second == search_value) {\n'
        maps_mutexes += '            search_map.erase(it++);\n'
        maps_mutexes += '        } else {\n'
        maps_mutexes += '            ++it;\n'
        maps_mutexes += '        }\n'
        maps_mutexes += '    }\n'
        maps_mutexes += '}\n'
        maps_mutexes += '\n'
        maps_mutexes += '// Function used to clean up any residual map values that point to an instance prior to that\n'
        maps_mutexes += '// instance being deleted.\n'
        maps_mutexes += 'void ApiDumpCleanUpMapsForTable(XrGeneratedDispatchTable *table) {\n'
        # Call each handle's erase utility function using the template we defined above.
        for handle in self.api_handles:
            base_handle_name = undecorate(handle.name)
            if handle.protect_value:
                maps_mutexes += '#if %s\n' % handle.protect_string
            maps_mutexes += '    eraseAllTableMapElements<std::unordered_map<%s, XrGeneratedDispatchTable*>>' % handle.name
            maps_mutexes += '(g_%s_dispatch_map, g_%s_dispatch_mutex, table);\n' % (
                base_handle_name, base_handle_name)
            if handle.protect_value:
                maps_mutexes += '#endif // %s\n' % handle.protect_string
        maps_mutexes += '}\n'
        maps_mutexes += '\n'
        return maps_mutexes

    # Generate a short version of the parameter name that we can use as a variable.
    #   self            the ApiDumpOutputGenerator object
    #   param_name      the name of the parameter to parse
    def generateShortParamVariableName(self, param_name):
        # Remove any structure dereferences or array brackets to make the
        # full name of the parameter something we can use as a variable.
        # Then cut it to no more than 3 word blocks to make sure it's a
        # unique enough name.
        short_param_name = param_name
        short_param_name = short_param_name.replace(".", "_")
        short_param_name = short_param_name.replace("->", "_")
        # The short name is no more than everything after the 3rd
        # underscore from the end of the parameter name.
        underscore_pos = short_param_name.rfind("_")
        if underscore_pos > 0:
            underscore_pos = short_param_name.rfind("_", 0, underscore_pos - 1)
            if underscore_pos > 0:
                underscore_pos = short_param_name.rfind(
                    "_", 0, underscore_pos - 1)
            short_param_name = short_param_name[underscore_pos + 1:]
        # Replace the array brackets after we get the short name because
        # we don't want too short of a name
        short_param_name = short_param_name.replace("[", "_")
        short_param_name = short_param_name.replace("]", "_")
        short_param_name = short_param_name.replace("__", "_")
        return short_param_name

    # Return the part of a parameter prior to a pointer/structure dereference.
    #   self            the ApiDumpOutputGenerator object
    #   param_name      the name of the parameter to parse
    def getParamPrefix(self, param_name):
        period_pos = param_name.rfind(".")
        arrow_pos = param_name.rfind(">")
        param_name_prefix = ''
        if arrow_pos > period_pos:
            param_name_prefix = param_name[:arrow_pos + 1]
        elif period_pos > arrow_pos:
            param_name_prefix = param_name[:period_pos + 1]
        return param_name_prefix

    # We don't know what to do with external graphics handles, so we need
    # to detect them here.
    #   self            the ApiDumpOutputGenerator object
    #   name            the name of the type to check
    def isExternalGraphicsApiHandle(self, name):
        if (name.startswith('Vk') or name.startswith('GLX') or name.startswith('ID3D') or
            name.startswith('wl_') or name.startswith('Mir') or name.startswith('xcb_') or
                name == 'HGLRC' or name == 'HDC'):
            return True
        return False

    # Output a single entry's C++ output code.  This will generate the final resulting
    # entry in the Api Dump content vector which is used to record the data to a file.
    #   self            the ApiDumpOutputGenerator object
    #   indent          the number of "tabs" to space in for the resulting C+ code.
    #   allow_deref     Boolean indicating if we want to allow a dereference
    #   member_param    the structure from automatic_source_generator for the member or parameter.
    #   base_type       the base type of the parameter being recorded
    #   description     a description of the member/parameter
    #   full_name       a full name of the parameter in C++ parlance including any structure/union/pointer dereferences
    #   cdecl           the C-style declaration for the parameter
    def outputSingleEntry(self, indent, allow_deref, member_param, base_type, description, full_name, cdecl):

        # Initialization of internal variables
        int_short_param_name = ''
        is_standard_type = False
        is_char = False
        use_stream = False
        pointer_count = member_param.pointer_count
        is_array = member_param.is_array

        # If this is an array defined by a pointer, we need to drop the pointer count for it
        if is_array and not member_param.is_static_array and len(member_param.pointer_count_var) > 0:
            pointer_count = pointer_count - 1

        full_type = cdecl[0:cdecl.rfind(' ')].strip()
        if member_param.is_static_array:
            full_type += '*'

        # Only attempt to dereference an object at this point if it's a const and
        # not a pointer.
        # NOTE: Structures/Unions should be handled in other utility calls.  This
        #       is only encountered if it can't extract the information through
        #       those if it's a type that could be expanded.
        can_dereference = member_param.is_const or not pointer_count > 0
        if not allow_deref:
            can_dereference = False

        # We can't dereference a void, so don't even try.
        # NOTE: The 'next' chain is actually handled elsewhere.
        if 'void' == base_type:
            can_dereference = False

        # Is this one of the standard types?
        if ('char' == base_type[0:4].lower() or 'float' == base_type[0:5].lower() or
            'double' == base_type[0:6].lower() or 'unsigned ' in base_type.lower() or
                'uint' in base_type.lower()):
            is_standard_type = True
            # Characters are a special case because we can write them directly out.  As long as
            # they're a character array or pointer.  If they're a pointer, just drop one from it
            # since we don't want to write the pointer, but the contents of it
            if 'char' == base_type[0:4].lower():
                is_char = True
                if not can_dereference or ((is_array and pointer_count > 0) or pointer_count > 1):
                    use_stream = True
                elif pointer_count > 0:
                    pointer_count -= 1
            else:
                # If it's a non-character standard type, we want to output the information
                # using a string stream instead of the standard output path.
                use_stream = True

        # If this is a pointer and not a character, or it's a handle, we also want to
        # use a stream.  This is because we can output hex values through a stream.
        if (self.isHandle(base_type) or self.isExternalGraphicsApiHandle(base_type) or
                ((is_array or pointer_count > 0) and not is_char)):
            use_stream = True

        # Create a short name for use as a variable
        int_short_param_name = self.generateShortParamVariableName(
            member_param.name)
        if is_array and not can_dereference:
            int_short_param_name += '_array'

        write_string = ''

        if base_type == 'GUID':
            write_string += '{\n'
            indent = indent + 1
            write_string += self.writeIndent(indent)
            write_string += 'std::ostringstream oss_%s;\n' % int_short_param_name
            write_string += self.writeIndent(indent)
            write_string += 'oss_%s << std::uppercase;\n' % int_short_param_name
            write_string += self.writeIndent(indent)
            write_string += 'oss_%s.width(8);\n' % int_short_param_name
            write_string += self.writeIndent(indent)
            write_string += 'oss_%s << std::hex << %s.Data1 << \'-\';\n' % (int_short_param_name, full_name)
            write_string += self.writeIndent(indent)
            write_string += 'oss_%s.width(4);\n' % int_short_param_name
            write_string += self.writeIndent(indent)
            write_string += 'oss_%s << std::hex << %s.Data2 << \'-\';\n' % (int_short_param_name, full_name)
            write_string += self.writeIndent(indent)
            write_string += 'oss_%s.width(4);\n' % int_short_param_name
            write_string += self.writeIndent(indent)
            write_string += 'oss_%s << std::hex << %s.Data3 << \'-\';\n' % (int_short_param_name, full_name)
            write_string += self.writeIndent(indent)
            write_string += 'oss_%s.width(2);\n' % int_short_param_name
            write_string += self.writeIndent(indent)
            write_string += 'oss_%s << std::hex\n' % int_short_param_name
            write_string += self.writeIndent(indent)
            write_string += '    << static_cast<short>(%s.Data4[0])\n' % full_name
            write_string += self.writeIndent(indent)
            write_string += '    << static_cast<short>(%s.Data4[1])\n' % full_name
            write_string += self.writeIndent(indent)
            write_string += '    << \'-\'\n'
            write_string += self.writeIndent(indent)
            write_string += '    << static_cast<short>(%s.Data4[2])\n' % full_name
            write_string += self.writeIndent(indent)
            write_string += '    << static_cast<short>(%s.Data4[3])\n' % full_name
            write_string += self.writeIndent(indent)
            write_string += '    << static_cast<short>(%s.Data4[4])\n' % full_name
            write_string += self.writeIndent(indent)
            write_string += '    << static_cast<short>(%s.Data4[5])\n' % full_name
            write_string += self.writeIndent(indent)
            write_string += '    << static_cast<short>(%s.Data4[6])\n' % full_name
            write_string += self.writeIndent(indent)
            write_string += '    << static_cast<short>(%s.Data4[7]);\n' % full_name
            write_string += self.writeIndent(indent)
            write_string += 'oss_%s << std::nouppercase;\n' % int_short_param_name
            write_string += self.writeIndent(indent)
            write_string += 'contents.emplace_back("%s", %s' % (full_type, description)
            write_string += ', oss_%s.str());\n' % int_short_param_name
            indent = indent - 1
            write_string += self.writeIndent(indent)
            write_string += '}\n'
        elif base_type == 'LUID':
            write_string += self.writeIndent(indent)
            write_string += '{\n'
            indent = indent + 1
            write_string += self.writeIndent(indent)
            write_string += 'std::ostringstream oss_%s;\n' % int_short_param_name
            write_string += self.writeIndent(indent)
            write_string += 'oss_%s << std::uppercase;\n' % int_short_param_name
            write_string += self.writeIndent(indent)
            write_string += 'oss_%s.width(8);\n' % int_short_param_name
            write_string += self.writeIndent(indent)
            write_string += 'oss_%s << std::hex << %s.LowPart;\n' % (int_short_param_name, full_name)
            write_string += self.writeIndent(indent)
            write_string += 'oss_%s << std::hex << %s.HighPart;\n' % (int_short_param_name, full_name)
            write_string += self.writeIndent(indent)
            write_string += 'oss_%s << std::nouppercase;\n' % int_short_param_name
            write_string += self.writeIndent(indent)
            write_string += 'contents.emplace_back("%s", %s' % (full_type, description)
            write_string += ', oss_%s.str());\n' % int_short_param_name
            indent = indent - 1
            write_string += self.writeIndent(indent)
            write_string += '}\n'
        elif base_type == 'LARGE_INTEGER':
            # Unbeknownst to XR, this is actually a union. Append '.QuadPart' to get the entirety
            write_string += self.writeIndent(indent)
            write_string += 'std::ostringstream oss_%s;\n' % int_short_param_name
            write_string += self.writeIndent(indent)
            write_string += 'oss_%s << ' % int_short_param_name
            write_string += 'std::hex << reinterpret_cast<const void*>( ('
            if pointer_count > 0:   # Ignore can_dereference, must deref to access union member
                write_string += '*' * pointer_count
            write_string += '%s).QuadPart );\n' % full_name
            write_string += self.writeIndent(indent)
            write_string += 'contents.emplace_back("%s", %s' % (full_type, description)
            write_string += ', oss_%s.str());\n' % int_short_param_name
        elif base_type == 'timespec':
            # Unbeknownst to XR, this is actually a struct.
            write_string += self.writeIndent(indent)
            write_string += 'std::ostringstream oss_%s;\n' % int_short_param_name
            write_string += self.writeIndent(indent)
            deref = '' + '*' * pointer_count

            # Write whole seconds
            write_string += 'oss_%s << (' % int_short_param_name
            write_string += deref
            write_string += '%s).tv_sec << ".";\n' % full_name

            # Write nanoseconds as a decimal
            write_string += self.writeIndent(indent)
            write_string += "oss_%s << std::setw(9) << std::setfill('0') << (" % int_short_param_name
            write_string += deref
            write_string += '%s).tv_nsec << "s";\n' % full_name

            write_string += self.writeIndent(indent)
            write_string += 'contents.emplace_back("%s", %s' % (
                full_type, description)
            write_string += ', oss_%s.str());\n' % int_short_param_name
        else:
            if base_type == 'XrResult':
                write_string += self.writeIndent(indent)
                write_string += 'if (nullptr != gen_dispatch_table) {\n'
                indent = indent + 1
                write_string += self.writeIndent(indent)
                write_string += 'char %s_string[XR_MAX_RESULT_STRING_SIZE];\n' % int_short_param_name
                write_string += self.writeIndent(indent)
                write_string += 'gen_dispatch_table->ResultToString(FindInstanceFromDispatchTable(gen_dispatch_table),\n'
                write_string += self.writeIndent(indent)
                write_string += '                                   %s, %s_string);\n' % (full_name, int_short_param_name)
                write_string += self.writeIndent(indent)
                write_string += 'contents.emplace_back("%s", %s, %s_string);\n' % (full_type, description, int_short_param_name)
                write_string += self.writeIndent(indent - 1)
                write_string += '} else {\n'
                write_string += self.writeIndent(indent)
            elif base_type == 'XrStructureType':
                write_string += self.writeIndent(indent)
                write_string += 'if (nullptr != gen_dispatch_table) {\n'
                indent = indent + 1
                write_string += self.writeIndent(indent)
                write_string += 'char %s_string[XR_MAX_STRUCTURE_NAME_SIZE];\n' % int_short_param_name
                write_string += self.writeIndent(indent)
                write_string += 'gen_dispatch_table->StructureTypeToString(FindInstanceFromDispatchTable(gen_dispatch_table),\n'
                write_string += self.writeIndent(indent)
                write_string += '                                          %s, %s_string);\n' % (full_name, int_short_param_name)
                write_string += self.writeIndent(indent)
                write_string += 'contents.emplace_back("%s", %s, %s_string);\n' % (full_type, description, int_short_param_name)
                write_string += self.writeIndent(indent - 1)
                write_string += '} else {\n'
                write_string += self.writeIndent(indent)

            # If we're outputting using a string stream, determine the type of information
            # we're generating and format it appropriately.
            if use_stream:
                write_string += self.writeIndent(indent)
                write_string += 'std::ostringstream oss_%s;\n' % int_short_param_name
                write_string += self.writeIndent(indent)
                # Output the standard type information if we can, except for characters because the
                # hex converter will try to use the string value in the char.
                if is_standard_type and not is_char:
                    if 'float' in base_type:
                        precision = '32'
                        if '64' in base_type:
                            precision = '64'
                        elif '16' in base_type:
                            precision = '16'
                        write_string += 'oss_%s << std::setprecision(%s) << (' % (int_short_param_name, precision)
                    elif 'double' in base_type:
                        write_string += 'oss_%s << std::setprecision(64) << (' % int_short_param_name
                    else:
                        write_string += 'oss_%s << ' % int_short_param_name
                        if member_param.pointer_count == 0:
                            write_string += '"0x" << '
                        write_string += 'std::hex << ('
                else:
                    write_string += 'oss_%s << ' % int_short_param_name
                    write_string += 'std::hex << reinterpret_cast<const void*>('
                if can_dereference and pointer_count > 0:
                    write_string += '*' * pointer_count
                write_string += '%s);\n' % full_name
                write_string += self.writeIndent(indent)
                write_string += 'contents.emplace_back("%s", %s' % (full_type, description)
                write_string += ', oss_%s.str());\n' % int_short_param_name

            else:
                write_string += self.writeIndent(indent)
                write_string += 'contents.emplace_back("%s", %s, ' % (full_type, description)
                if not is_char:
                    write_string += 'std::to_string('
                if can_dereference:
                    write_string += '*' * pointer_count
                write_string += '%s' % full_name
                # Close std::to_string
                if not is_char:
                    write_string += ')'
                write_string += ');\n'

            if base_type == 'XrResult' or base_type == 'XrStructureType':
                indent = indent - 1
                write_string += self.writeIndent(indent)
                write_string += '}\n'
        return write_string

    # Output a single parameter/member.
    #   self                The ApiDumpOutputGenerator object
    #   base_type           The base type of the parameter
    #   is_pointer          Boolean indicating whether or not the contents of the arrays are pointers
    #   pointer_count       The number of pointers per variable (void*[] would be one, void**[] would be two)
    #   member_param        The structure from automatic_source_generator for the member or parameter.
    #   member_param_prefix The prefix to place in front of the member/param items
    #   member_param_name   The prefixed name of this member/param
    #   has_prefix          Boolean indicates that there's an incoming C++ prefix that needs to be added to the variable.
    #   prefix_string1      The first prefix string to add prior to writing out the variable information
    #   prefix_string1      The second prefix string to add prior to writing out the variable information
    #   expand              Boolean indicates whether or not to try to expand/derefernce the contents of this parameter
    #   indent              the number of "tabs" to space in for the resulting C+ code.
    def writeExpandedMember(self, base_type, is_pointer, pointer_count, member_param, member_param_prefix, member_param_name, has_prefix, prefix_string1, prefix_string2, expand, indent):
        member_string = ''
        derefernce_str = ''
        if not is_pointer:
            derefernce_str = '&'

        prefix_string = ''
        if has_prefix:
            prefix_string += self.writeIndent(indent)
            prefix_string += prefix_string1
            prefix_string += self.writeIndent(indent)
            prefix_string += prefix_string2

        # If it's a structure or union, we can also only expand it if it's not
        # return-only
        # If this is a structure or union, save that info for easier use later
        is_struct_union = False
        member_param_struct = self.getStruct(member_param.type)
        member_param_union = self.getUnion(member_param.type)
        if member_param_struct or member_param_union:
            is_struct_union = True

        # Type for output structure call
        full_type = member_param.cdecl[0:member_param.cdecl.rfind(' ')].strip()
        pointer_string = 'false'
        if is_pointer:
            pointer_string = 'true'
        if member_param.name == 'next':
            member_string += prefix_string
            member_string += self.writeIndent(indent)
            member_string += '// Decode the next chain if it exists\n'
            member_string += self.writeIndent(indent)
            member_string += 'if (!ApiDumpDecodeNextChain(gen_dispatch_table, %s%s, %s, contents)) {\n' % (derefernce_str,
                                                                                                           member_param_name,
                                                                                                           member_param_prefix)
            member_string += self.writeIndent(indent + 1)
            member_string += 'throw std::invalid_argument("Invalid Operation");\n'
            member_string += self.writeIndent(indent)
            member_string += '}\n'
        elif is_struct_union and expand:
            member_string += prefix_string
            member_string += self.writeIndent(indent)
            # If it's optional, we still want to dump out NULL if it's present just so it's logged
            if member_param.is_optional and is_pointer:
                member_string += 'if (nullptr == %s) {\n' % member_param_name
                member_string += self.outputSingleEntry(indent + 1,
                                                        False,
                                                        member_param,
                                                        base_type,
                                                        member_param_prefix,
                                                        member_param_name,
                                                        member_param.cdecl)
                member_string += self.writeIndent(indent)
                member_string += '} else '
            # Otherwise, if it's not NULL, print out the contents
            if member_param_struct:
                member_string += 'if (!ApiDumpOutputXrStruct(gen_dispatch_table, '
            else:
                member_string += 'if (!ApiDumpOutputXrUnion(gen_dispatch_table, '
            member_string += '%s%s, %s, "%s", %s, contents)) {\n' % (derefernce_str,
                                                                     member_param_name,
                                                                     member_param_prefix,
                                                                     full_type,
                                                                     pointer_string)
            member_string += self.writeIndent(indent + 1)
            member_string += 'throw std::invalid_argument("Invalid Operation");\n'
            member_string += self.writeIndent(indent)
            member_string += '}\n'
        else:
            valid_extension_structs = None
            if member_param_struct or member_param_union:
                valid_extension_structs = member_param.valid_extension_structs
            member_string += prefix_string

            tmp_member_param = self.MemberOrParam(type=member_param.type,
                                                  name=member_param.name,
                                                  is_const=member_param.is_const,
                                                  is_handle=member_param.is_handle,
                                                  is_bool=member_param.is_bool,
                                                  is_optional=member_param.is_optional,
                                                  no_auto_validity=member_param.no_auto_validity,
                                                  valid_extension_structs=valid_extension_structs,
                                                  is_array=member_param.is_array,
                                                  is_static_array=member_param.is_static_array,
                                                  static_array_sizes=member_param.static_array_sizes,
                                                  array_dimen=member_param.array_dimen,
                                                  array_count_var=member_param.array_count_var,
                                                  array_length_for=member_param.array_length_for,
                                                  pointer_count=pointer_count,
                                                  is_null_terminated=member_param.is_null_terminated,
                                                  pointer_count_var=member_param.pointer_count_var,
                                                  cdecl=member_param.cdecl,
                                                  values=member_param.values)

            if member_param.is_optional and member_param.is_const and member_param.pointer_count > 0:
                member_string += self.writeIndent(indent)
                member_string += 'if (nullptr == %s) {\n' % member_param_name
                indent += 1
                member_string += self.outputSingleEntry(indent,
                                                        False,
                                                        tmp_member_param,
                                                        base_type,
                                                        member_param_prefix,
                                                        member_param_name,
                                                        member_param.cdecl)
                member_string += self.writeIndent(indent - 1)
                member_string += '} else {\n'
            member_string += self.outputSingleEntry(indent,
                                                    True,
                                                    tmp_member_param,
                                                    base_type,
                                                    member_param_prefix,
                                                    member_param_name,
                                                    member_param.cdecl)
            if member_param.is_optional and member_param.is_const and member_param.pointer_count > 0:
                indent -= 1
                member_string += self.writeIndent(indent)
                member_string += '}\n'
        return member_string

    # Output an array of parameters/members.
    #   self                The ApiDumpOutputGenerator object
    #   base_type           The base type of the parameter
    #   is_pointer          Boolean indicating whether or not the contents of the arrays are pointers
    #   pointer_count       The number of pointers per variable (void*[] would be one, void**[] would be two)
    #   member_param        The structure from automatic_source_generator for the member or parameter.
    #   array_param         The member/parameter used to indicate the size of the array (or None)
    #   member_param_prefix The prefix to place in front of the member/param items
    #   member_param_name   The prefixed name of this member/param
    #   has_prefix          Boolean indicates that there's an incoming C++ prefix that needs to be added to the variable.
    #   prefix_string1      The first prefix string to add prior to writing out the variable information
    #   prefix_string1      The second prefix string to add prior to writing out the variable information
    #   indent              the number of "tabs" to space in for the resulting C+ code.
    def writeExpandedArray(self, base_type, is_pointer, pointer_count, member_param, array_param, member_param_prefix, member_param_name, has_prefix, prefix_string1, prefix_string2, indent):
        member_array_string = ''
        loop_count_name = ''
        loop_param_name = ''
        if self.isAllNumbers(array_param) or self.isAllUpperCase(array_param):
            loop_count_name = array_param
        else:
            loop_count_name = ''
            if has_prefix:
                loop_count_name = 'value->'
            loop_count_name += array_param
        if has_prefix:
            loop_param_name = 'value_'
        loop_param_name += member_param.name.lower()
        loop_param_name += '_inc'
        member_array_string += self.writeIndent(indent)
        member_array_string += prefix_string1
        prefix_string1 = ''
        member_array_string += self.writeIndent(indent)
        member_array_string += prefix_string2
        prefix_string2 = ''
        member_array_string += self.outputSingleEntry(indent,
                                                      False,
                                                      member_param,
                                                      base_type,
                                                      member_param_prefix,
                                                      member_param_name,
                                                      member_param.cdecl)
        member_array_string += self.writeIndent(indent)
        member_array_string += 'for (uint32_t %s = 0; %s < %s; ++%s) {\n' % (loop_param_name,
                                                                             loop_param_name,
                                                                             loop_count_name,
                                                                             loop_param_name)
        indent = indent + 1

        # Make sure we set that we need a prefix (even if we didn't need one before) because
        # of the array looping.
        paranet_param_prefix = member_param_prefix
        member_param_prefix = '%s_array_prefix' % member_param.name.lower()
        prefix_string1 = 'std::string %s = %s;\n' % (
            member_param_prefix, paranet_param_prefix)
        prefix_string2 += '%s += "[";\n' % member_param_prefix
        prefix_string2 += self.writeIndent(indent)
        prefix_string2 += '%s += std::to_string(%s);\n' % (
            member_param_prefix, loop_param_name)
        prefix_string2 += self.writeIndent(indent)
        prefix_string2 += '%s += "]' % member_param_prefix
        prefix_string2 += '";\n'
        member_param_name += "[%s]" % loop_param_name
        array_dimen = member_param.array_dimen - 1
        static_array_sizes = []
        is_array = member_param.is_array
        is_static_array = member_param.is_static_array
        array_count_var = member_param.array_count_var
        pointer_count_var = member_param.pointer_count_var
        array_length_for = member_param.array_length_for
        if array_dimen <= 0:
            is_array = False
            is_static_array = False
            array_count_var = ''
            pointer_count_var = ''
            array_length_for = ''
        else:
            static_array_sizes = member_param.static_array_sizes[1:]

        tmp_member_param = self.MemberOrParam(type=member_param.type,
                                              name=member_param.name,
                                              is_const=member_param.is_const,
                                              is_handle=member_param.is_handle,
                                              is_bool=member_param.is_bool,
                                              is_optional=member_param.is_optional,
                                              no_auto_validity=member_param.no_auto_validity,
                                              valid_extension_structs=member_param.valid_extension_structs,
                                              is_array=is_array,
                                              is_static_array=is_static_array,
                                              static_array_sizes=static_array_sizes,
                                              array_dimen=array_dimen,
                                              array_count_var=array_count_var,
                                              array_length_for=array_length_for,
                                              pointer_count=pointer_count,
                                              is_null_terminated=member_param.is_null_terminated,
                                              pointer_count_var=pointer_count_var,
                                              cdecl=member_param.cdecl,
                                              values=member_param.values)

        member_array_string += self.writeExpandedMember(base_type, is_pointer, pointer_count, tmp_member_param,
                                                        member_param_prefix, member_param_name, True, prefix_string1, prefix_string2, True, indent)
        indent = indent - 1
        member_array_string += self.writeIndent(indent)
        member_array_string += '}\n'
        return member_array_string

    # Output a single parameter or member based on whether it is an array or not
    #   self            the ApiDumpOutputGenerator object
    #   member_param    the structure from automatic_source_generator for the member or parameter.
    #   has_prefix      Boolean indicates that there's an incoming C++ prefix that needs to be added to the variable.
    #   expand_parent   Boolean indicating that the parent could or could not be expanded.
    #   indent          the number of "tabs" to space in for the resulting C+ code.
    def writeParamMember(self, member_param, has_prefix, expand_parent, indent):
        member_param_string = ''
        # Can only expand non-pointers or constant pointer values and only if we can
        # expand the parent
        can_expand = False
        if expand_parent:
            can_expand = member_param.is_const or member_param.pointer_count == 0

        # If it's a structure or union, we can also only expand it if it's not
        # return-only
        member_param_struct = self.getStruct(member_param.type)
        member_param_union = self.getUnion(member_param.type)
        if ((member_param_struct and member_param_struct.returned_only) or
                (member_param_union and member_param_union.returned_only)):
            can_expand = False
        elif member_param.type == 'void' and member_param.name != 'next':
            can_expand = False

        is_pointer = False
        is_array = member_param.is_static_array
        base_type = self.getRawType(member_param.type)
        array_param = ''
        pointer_count = member_param.pointer_count
        if len(member_param.array_count_var) != 0:
            array_param = member_param.array_count_var
            is_array = True
            if pointer_count > 0:
                is_pointer = True
        elif len(member_param.pointer_count_var) != 0:
            array_param = member_param.pointer_count_var
            is_array = True
            pointer_count -= 1
            if pointer_count > 0:
                is_pointer = True
        elif member_param.is_static_array:
            array_param = member_param.static_array_sizes[0]
        elif pointer_count > 0:
            is_pointer = True
        # If this is a character array, it's really a string.  So,
        # only treat it as an array if it also still has a pointer
        # in addition to the array itself.
        if base_type == 'char' and is_array and not is_pointer:
            is_array = False

        member_param_prefix = ''
        prefix_string1 = ''
        prefix_string2 = ''
        member_param_name = member_param.name
        if has_prefix:
            member_param_prefix = '%s_prefix' % member_param.name.lower()
            prefix_string1 = 'std::string %s = prefix;\n' % member_param_prefix
            prefix_string2 = '%s += "' % member_param_prefix
            member_param_name = "value->%s" % member_param.name
            prefix_string2 += '%s";\n' % member_param.name
        else:
            member_param_prefix = '"%s"' % member_param.name

        if can_expand and is_array:
            is_relation_group = False
            relation_group = None
            # We only care about a relation group at this point if it's an array
            # that isn't a pointer
            if member_param_struct and pointer_count == 0:
                # Check to see if this struct is the base of a relation group
                for cur_rel_group in self.struct_relation_groups:
                    if cur_rel_group.generic_struct_name == member_param_struct.name:
                        relation_group = cur_rel_group
                        is_relation_group = True
                        break
            # If this struct is the base of a relation group, check to see if this call really should go to any one of
            # it's children instead of itself.
            if is_relation_group:
                member_param_string += self.writeIndent(indent)
                decoded_var = '%s_decoded' % member_param.name.lower()
                member_param_string += 'bool %s = false;\n' % decoded_var
                for child in relation_group.child_struct_names:
                    child_struct = self.getStruct(child)
                    if child_struct.protect_value:
                        member_param_string += '#if %s\n' % child_struct.protect_string
                    member_param_string += self.writeIndent(indent)
                    member_param_string += 'if (%s[0].type == %s) {\n' % (
                        member_param_name, self.genXrStructureType(child))
                    member_param_string += self.writeExpandedArray(base_type, is_pointer, pointer_count, member_param, array_param,
                                                                   member_param_prefix, member_param_name, has_prefix, prefix_string1, prefix_string2, indent + 1)
                    member_param_string += self.writeIndent(indent + 1)
                    member_param_string += '%s = true;\n' % decoded_var
                    member_param_string += self.writeIndent(indent)
                    member_param_string += '}\n'
                    if child_struct.protect_value:
                        member_param_string += '#endif // %s\n' % child_struct.protect_string
                member_param_string += self.writeIndent(indent)
                member_param_string += 'if (!%s) {\n' % decoded_var
                indent += 1
            member_param_string += self.writeExpandedArray(base_type, is_pointer, pointer_count, member_param,
                                                           array_param, member_param_prefix, member_param_name, has_prefix, prefix_string1, prefix_string2, indent)
            if is_relation_group:
                indent -= 1
                member_param_string += self.writeIndent(indent)
                member_param_string += '}\n'
        else:
            member_param_string += self.writeExpandedMember(base_type, is_pointer, pointer_count, member_param,
                                                            member_param_prefix, member_param_name, has_prefix, prefix_string1, prefix_string2, can_expand, indent)
        return member_param_string

    # Generate the C++ output code for each member of a union or structure.
    #   self            the ApiDumpOutputGenerator object
    #   union_struct    the structure from automatic_source_generator for the XR union or structure.
    #   indent          the number of "tabs" to space in for the resulting C+ code.
    def writeUnionStructMembers(self, union_struct, indent):
        struct_union_member = ''
        for member in union_struct.members:
            struct_union_member += self.writeParamMember(
                member, True, True, indent)
        return struct_union_member

    # Write the C++ Api Dump function for every union and structure we know about.
    #   self            the ApiDumpOutputGenerator object
    def writeApiDumpUnionStructFuncs(self):
        struct_union_check = '\n// Union/Structure Output Helper functions\n'
        for xr_union in self.api_unions:
            if xr_union.protect_value:
                struct_union_check += '#if %s\n' % xr_union.protect_string
            struct_union_check += 'bool ApiDumpOutputXrUnion(XrGeneratedDispatchTable* gen_dispatch_table, const %s* value,\n' % xr_union.name
            struct_union_check += '                          std::string prefix, std::string type_string, bool is_pointer,\n'
            struct_union_check += '                          std::vector<std::tuple<std::string, std::string, std::string>> &contents) {\n'
            struct_union_check += self.writeIndent(1)
            struct_union_check += 'try {\n'
            struct_union_check += self.writeIndent(2)
            struct_union_check += 'contents.emplace_back(type_string, prefix, PointerToHexString(value));\n'
            struct_union_check += self.writeIndent(2)
            struct_union_check += 'if (is_pointer) {\n'
            struct_union_check += self.writeIndent(3)
            struct_union_check += 'prefix += "->";\n'
            struct_union_check += self.writeIndent(2)
            struct_union_check += '} else {\n'
            struct_union_check += self.writeIndent(3)
            struct_union_check += 'prefix += ".";\n'
            struct_union_check += self.writeIndent(2)
            struct_union_check += '}\n'
            struct_union_check += self.writeUnionStructMembers(xr_union, 2)
            struct_union_check += self.writeIndent(2)
            struct_union_check += 'return true;\n'
            struct_union_check += self.writeIndent(1)
            struct_union_check += '} catch(...) {\n'
            struct_union_check += self.writeIndent(1)
            struct_union_check += '}\n'
            struct_union_check += self.writeIndent(1)
            struct_union_check += 'return false;\n'
            struct_union_check += '}\n\n'
            if xr_union.protect_value:
                struct_union_check += '#endif // %s\n' % xr_union.protect_string
        for xr_struct in self.api_structures:
            if xr_struct.protect_value:
                struct_union_check += '#if %s\n' % xr_struct.protect_string
            struct_union_check += 'bool ApiDumpOutputXrStruct(XrGeneratedDispatchTable* gen_dispatch_table, const %s* value,\n' % xr_struct.name
            struct_union_check += '                           std::string prefix, std::string type_string, bool is_pointer,\n'
            struct_union_check += '                           std::vector<std::tuple<std::string, std::string, std::string>> &contents) {\n'
            indent = 1
            struct_union_check += self.writeIndent(indent)
            struct_union_check += 'try {\n'
            indent = indent + 1
            is_relation_group = False
            relation_group = None
            # Check to see if this struct is the base of a relation group
            for cur_rel_group in self.struct_relation_groups:
                if cur_rel_group.generic_struct_name == xr_struct.name:
                    relation_group = cur_rel_group
                    is_relation_group = True
                    break
            # If this struct is the base of a relation group, check to see if this call really should go to any one of
            # it's children instead of itself.
            if is_relation_group:
                for child in relation_group.child_struct_names:
                    child_struct = self.getStruct(child)
                    if child_struct.protect_value:
                        struct_union_check += '#if %s\n' % child_struct.protect_string
                    struct_union_check += self.writeIndent(indent)
                    struct_union_check += 'if (value->type == %s) {\n' % self.genXrStructureType(
                        child)
                    struct_union_check += self.writeIndent(indent + 1)
                    struct_union_check += 'const %s* new_value = reinterpret_cast<const %s*>(value);\n' % (
                        child, child)
                    struct_union_check += self.writeIndent(indent + 1)
                    struct_union_check += 'return ApiDumpOutputXrStruct(gen_dispatch_table, new_value, prefix, type_string, is_pointer, contents);\n'
                    struct_union_check += self.writeIndent(indent)
                    struct_union_check += '}\n'
                    if child_struct.protect_value:
                        struct_union_check += '#endif // %s\n' % child_struct.protect_string
                struct_union_check += self.writeIndent(indent)
                struct_union_check += '// Fallback path - Just output generic information about the base struct\n'
            struct_union_check += self.writeIndent(indent)
            struct_union_check += 'contents.emplace_back(type_string, prefix, PointerToHexString(value));\n'
            struct_union_check += self.writeIndent(indent)
            struct_union_check += 'if (is_pointer) {\n'
            struct_union_check += self.writeIndent(indent + 1)
            struct_union_check += 'prefix += "->";\n'
            struct_union_check += self.writeIndent(indent)
            struct_union_check += '} else {\n'
            struct_union_check += self.writeIndent(indent + 1)
            struct_union_check += 'prefix += ".";\n'
            struct_union_check += self.writeIndent(indent)
            struct_union_check += '}\n'
            struct_union_check += self.writeUnionStructMembers(
                xr_struct, indent)
            struct_union_check += self.writeIndent(indent)
            struct_union_check += 'return true;\n'
            indent = indent - 1
            struct_union_check += self.writeIndent(indent)
            struct_union_check += '} catch(...) {\n'
            struct_union_check += self.writeIndent(indent)
            struct_union_check += '}\n'
            struct_union_check += self.writeIndent(indent)
            struct_union_check += 'return false;\n'
            struct_union_check += '}\n'
            if xr_struct.protect_value:
                struct_union_check += '#endif // %s\n' % xr_struct.protect_string
            struct_union_check += '\n'
        struct_union_check += 'bool ApiDumpDecodeNextChain(XrGeneratedDispatchTable* gen_dispatch_table, const void* value, std::string prefix,\n'
        struct_union_check += '                            std::vector<std::tuple<std::string, std::string, std::string>> &contents) {\n'
        struct_union_check += '    try {\n'
        struct_union_check += '        contents.emplace_back("const void *", prefix, PointerToHexString(value));\n'
        struct_union_check += '        if (nullptr == value) {\n'
        struct_union_check += '            return true;\n'
        struct_union_check += '        }\n'
        struct_union_check += self.writeIndent(2)
        struct_union_check += 'const XrBaseInStructure* next_header = reinterpret_cast<const XrBaseInStructure*>(value);\n'
        # Validate the rest of this struct
        struct_union_check += self.writeIndent(2)
        struct_union_check += 'switch (next_header->type) {\n'
        for enum_tuple in self.api_enums:
            if enum_tuple.name == 'XrStructureType':
                if enum_tuple.protect_value:
                    struct_union_check += '#if %s\n' % enum_tuple.protect_string
                for cur_value in enum_tuple.values:
                    struct_define_name = self.genXrStructureName(
                        cur_value.name)
                    if len(struct_define_name) > 0:
                        cur_struct = self.getStruct(struct_define_name)
                        if cur_struct.protect_value:
                            struct_union_check += '#if %s\n' % cur_struct.protect_string
                        struct_union_check += self.writeIndent(3)
                        struct_union_check += 'case %s:\n' % cur_value.name
                        struct_union_check += self.writeIndent(4)
                        struct_union_check += 'if (!ApiDumpOutputXrStruct(gen_dispatch_table, reinterpret_cast<const %s*>(value), prefix, "const %s*", true, contents)) {\n' % (
                            struct_define_name, struct_define_name)
                        struct_union_check += self.writeIndent(5)
                        struct_union_check += 'return false;\n'
                        struct_union_check += self.writeIndent(4)
                        struct_union_check += '}\n'
                        struct_union_check += self.writeIndent(4)
                        struct_union_check += 'return true;\n'
                        if cur_struct.protect_value:
                            struct_union_check += '#endif // %s\n' % cur_struct.protect_string
                if enum_tuple.protect_value:
                    struct_union_check += '#endif // %s\n' % enum_tuple.protect_string
        struct_union_check += self.writeIndent(3)
        struct_union_check += 'default:\n'
        struct_union_check += self.writeIndent(4)
        struct_union_check += 'return false;\n'
        struct_union_check += self.writeIndent(2)
        struct_union_check += '}\n'
        struct_union_check += '    } catch(...) {\n'
        struct_union_check += '    }\n'
        struct_union_check += '    return false;\n'
        struct_union_check += '}\n\n'
        return struct_union_check

    # Write the C++ Api Dump function for every command we know about
    #   self            the ApiDumpOutputGenerator object
    def outputLayerCommands(self):
        cur_extension_name = ''
        generated_commands = '\n// Automatically generated api_dump layer commands\n'
        for x in range(0, 2):
            if x == 0:
                commands = self.core_commands
            else:
                commands = self.ext_commands

            for cur_cmd in commands:
                if cur_cmd.ext_name != cur_extension_name:
                    if self.isCoreExtensionName(cur_cmd.ext_name):
                        generated_commands += '\n// ---- Core %s commands\n' % cur_cmd.ext_name[11:].replace(
                            "_", ".")
                    else:
                        generated_commands += '\n// ---- %s extension commands\n' % cur_cmd.ext_name
                    cur_extension_name = cur_cmd.ext_name

                if cur_cmd.name in DONT_GEN_IN_LAYER or cur_cmd.name in MANUALLY_DEFINED_IN_LAYER:
                    continue

                # We fill in the GetInstanceProcAddr manually at the end
                if cur_cmd.name == 'xrGetInstanceProcAddr':
                    continue

                is_create = False
                is_destroy = False
                has_return = False

                if ('xrCreate' in cur_cmd.name or 'xrConnect' in cur_cmd.name) and cur_cmd.params[-1].is_handle:
                    is_create = True
                    has_return = True
                elif ('xrDestroy' in cur_cmd.name or 'xrDisconnect' in cur_cmd.name) and cur_cmd.params[-1].is_handle:
                    is_destroy = True
                    has_return = True
                elif cur_cmd.return_type is not None:
                    has_return = True

                base_name = cur_cmd.name[2:]

                if cur_cmd.protect_value:
                    generated_commands += '#if %s\n' % cur_cmd.protect_string

                prototype = cur_cmd.cdecl.replace(" xr", " ApiDumpLayerXr")
                prototype = prototype.replace("XRAPI_ATTR ", "")
                prototype = prototype.replace(" XRAPI_CALL ", " ")
                prototype = prototype.replace(";", " {\n")
                generated_commands += prototype

                if has_return:
                    return_prefix = '    '
                    return_prefix += cur_cmd.return_type.text
                    return_prefix += ' result'
                    if cur_cmd.return_type.text == 'XrResult':
                        return_prefix += ' = XR_SUCCESS;\n'
                    else:
                        return_prefix += ';\n'
                    generated_commands += return_prefix

                generated_commands += '    try {\n'
                generated_commands += '        // Generate output for this command\n'
                generated_commands += '        std::vector<std::tuple<std::string, std::string, std::string>> contents;\n'

                # Next, we have to call down to the next implementation of this command in the call chain.
                # Before we can do that, we have to figure out what the dispatch table is
                if cur_cmd.params[0].is_handle:
                    handle_param = cur_cmd.params[0]
                    base_handle_name = undecorate(handle_param.type)
                    first_handle_name = self.getFirstHandleName(handle_param)
                    generated_commands += '        std::unique_lock<std::mutex> mlock(g_%s_dispatch_mutex);\n' % base_handle_name
                    generated_commands += '        auto map_iter = g_%s_dispatch_map.find(%s);\n' % (base_handle_name, first_handle_name)
                    generated_commands += '        mlock.unlock();\n\n'
                    generated_commands += '        if (map_iter == g_%s_dispatch_map.end()) return XR_ERROR_VALIDATION_FAILURE;\n' % base_handle_name
                    generated_commands += '        XrGeneratedDispatchTable *gen_dispatch_table = map_iter->second;\n'
                else:
                    generated_commands += self.printCodeGenErrorMessage(
                        'Command %s does not have an OpenXR Object handle as the first parameter.' % cur_cmd.name)

                # Print out a tuple for the header
                if has_return:
                    generated_commands += '        contents.emplace_back("%s", "%s", "");\n' % (
                        cur_cmd.return_type.text, cur_cmd.name)
                else:
                    generated_commands += '        contents.emplace_back("void", "%s", "");\n' % cur_cmd.name
                # Print out information for each parameter
                for param in cur_cmd.params:
                    can_expand = False
                    # TODO handle array of handles here?
                    if ((self.isStruct(param.type) or self.isUnion(param.type)) and
                            (param.is_const or param.pointer_count == 0)):
                        can_expand = True
                    generated_commands += self.writeParamMember(
                        param, False, can_expand, 2)

                # Now record the information
                generated_commands += '        ApiDumpLayerRecordContent(contents);\n\n'

                # Call down, looking for the returned result if required.
                generated_commands += '        '
                if has_return:
                    generated_commands += 'result = '
                generated_commands += 'gen_dispatch_table->%s(' % base_name

                count = 0
                for param in cur_cmd.params:
                    if count > 0:
                        generated_commands += ', '
                    generated_commands += param.name
                    count = count + 1
                generated_commands += ');\n'

                # If this is a create command, we have to create an entry in the appropriate
                # unordered_map pointing to the correct dispatch table for the newly created
                # object.  Likewise, if it's a delete command, we have to remove the entry
                # for the dispatch table from the unordered_map
                second_base_handle_name = ''
                if cur_cmd.params[-1].is_handle and (is_create or is_destroy):
                    second_base_handle_name = undecorate(cur_cmd.params[-1].type)
                    if is_create:
                        generated_commands += '        if (XR_SUCCESS == result && nullptr != %s) {\n' % cur_cmd.params[-1].name
                        generated_commands += '            auto exists = g_%s_dispatch_map.find(*%s);\n' % (
                            second_base_handle_name, cur_cmd.params[-1].name)
                        generated_commands += '            if (exists == g_%s_dispatch_map.end()) {\n' % second_base_handle_name
                        generated_commands += '                std::unique_lock<std::mutex> lock(g_%s_dispatch_mutex);\n' % second_base_handle_name
                        generated_commands += '                g_%s_dispatch_map[*%s] = gen_dispatch_table;\n' % (
                            second_base_handle_name, cur_cmd.params[-1].name)
                        generated_commands += '            }\n'
                        generated_commands += '        }\n'
                    elif is_destroy:
                        generated_commands += '        auto exists = g_%s_dispatch_map.find(%s);\n' % (
                            second_base_handle_name, cur_cmd.params[-1].name)
                        generated_commands += '        if (exists != g_%s_dispatch_map.end()) {\n' % second_base_handle_name
                        generated_commands += '            std::unique_lock<std::mutex> lock(g_%s_dispatch_mutex);\n' % second_base_handle_name
                        generated_commands += '            g_%s_dispatch_map.erase(%s);\n' % (
                            second_base_handle_name, cur_cmd.params[-1].name)
                        generated_commands += '        }\n'

                # Catch any exceptions that may have occurred.  If any occurred between any of the
                # valid mutex lock/unlock statements, perform the unlock now.
                generated_commands += '    } catch (...) {\n'
                if has_return:
                    generated_commands += '        return XR_ERROR_VALIDATION_FAILURE;\n'
                generated_commands += '    }\n'

                if has_return:
                    generated_commands += '    return result;\n'

                generated_commands += '}\n\n'
                if cur_cmd.protect_value:
                    generated_commands += '#endif // %s\n' % cur_cmd.protect_string

        # Output the xrGetInstanceProcAddr command for the API Dump layer.
        generated_commands += '\n// Layer\'s xrGetInstanceProcAddr\n'
        generated_commands += 'XrResult ApiDumpLayerXrGetInstanceProcAddr(\n'
        generated_commands += '    XrInstance                                  instance,\n'
        generated_commands += '    const char*                                 name,\n'
        generated_commands += '    PFN_xrVoidFunction*                         function) {\n'
        generated_commands += '    try {\n'
        generated_commands += '        std::string func_name = name;\n\n'
        generated_commands += '        // Generate output for this command\n'
        generated_commands += '        std::vector<std::tuple<std::string, std::string, std::string>> contents;\n'
        generated_commands += '        contents.emplace_back("XrResult", "xrGetInstanceProcAddr", "");\n'
        generated_commands += '        contents.emplace_back("XrInstance", "instance", HandleToHexString(instance));\n'
        generated_commands += '        contents.emplace_back("const char*", "name", name);\n'
        generated_commands += '        contents.emplace_back("PFN_xrVoidFunction*", "function", PointerToHexString(reinterpret_cast<const void*>(function)));\n'
        generated_commands += '        ApiDumpLayerRecordContent(contents);\n'

        count = 0
        for x in range(0, 2):
            if x == 0:
                commands = self.core_commands
            else:
                commands = self.ext_commands

            for cur_cmd in commands:
                if cur_cmd.ext_name != cur_extension_name:
                    if self.isCoreExtensionName(cur_cmd.ext_name):
                        generated_commands += '\n        // ---- Core %s commands\n' % cur_cmd.ext_name[11:].replace(
                            "_", ".")
                    else:
                        generated_commands += '\n        // ---- %s extension commands\n' % cur_cmd.ext_name
                    cur_extension_name = cur_cmd.ext_name

                if cur_cmd.name in DONT_GEN_IN_LAYER:
                    continue

                has_return = False
                if cur_cmd.return_type is not None:
                    has_return = True

                # Replace 'xr' in proto name with an API Dump-specific name to avoid collisions.s
                layer_command_name = cur_cmd.name.replace(
                    "xr", "ApiDumpLayerXr")

                if cur_cmd.protect_value:
                    generated_commands += '#if %s\n' % cur_cmd.protect_string

                if count == 0:
                    generated_commands += '        if (func_name == "%s") {\n' % cur_cmd.name
                else:
                    generated_commands += '        } else if (func_name == "%s") {\n' % cur_cmd.name
                count = count + 1

                generated_commands += '            *function = reinterpret_cast<PFN_xrVoidFunction>(%s);\n' % layer_command_name
                if cur_cmd.protect_value:
                    generated_commands += '#endif // %s\n' % cur_cmd.protect_string

        generated_commands += '        }\n'
        generated_commands += '        // If we setup the function, just return\n'
        generated_commands += '        if (*function != nullptr) {\n'
        generated_commands += '            return XR_SUCCESS;\n'
        generated_commands += '        }\n\n'
        generated_commands += '        // We have not found it, so pass it down to the next layer/runtime\n'
        generated_commands += '        std::unique_lock<std::mutex> mlock(g_instance_dispatch_mutex);\n'
        generated_commands += '        auto map_iter = g_instance_dispatch_map.find(instance);\n'
        generated_commands += '        mlock.unlock();\n\n'
        generated_commands += '        if (map_iter == g_instance_dispatch_map.end()) {\n'
        generated_commands += '            return XR_ERROR_HANDLE_INVALID;\n'
        generated_commands += '        }\n\n'
        generated_commands += '        XrGeneratedDispatchTable *gen_dispatch_table = map_iter->second;\n'
        generated_commands += '        if (nullptr == gen_dispatch_table) {\n'
        generated_commands += '            return XR_ERROR_HANDLE_INVALID;\n'
        generated_commands += '        }\n\n'
        generated_commands += '        return gen_dispatch_table->GetInstanceProcAddr(instance, name, function);\n'
        generated_commands += '    } catch (...) {\n'
        generated_commands += '        return XR_ERROR_VALIDATION_FAILURE;\n'
        generated_commands += '    }\n'
        generated_commands += '}\n'
        return generated_commands
