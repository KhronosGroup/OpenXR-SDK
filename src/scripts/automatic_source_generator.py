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
# Purpose:      This file is the base python script for parsing the
#               registry and forming the information in an easy to use
#               way for the rest of the automatic source generation scripts.

import os
import re
import sys
from generator import *
from collections import namedtuple
from inspect import currentframe, getframeinfo

# AutomaticSourceGeneratorOptions - subclass of GeneratorOptions.


class AutomaticSourceGeneratorOptions(GeneratorOptions):
    def __init__(self,
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
        GeneratorOptions.__init__(self, filename, directory, apiname, profile,
                                  versions, emitversions, defaultExtensions,
                                  addExtensions, removeExtensions,
                                  emitExtensions, sortProcedure)
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

# AutomaticSourceOutputGenerator - subclass of OutputGenerator.


class AutomaticSourceOutputGenerator(OutputGenerator):
    """Parse source based on XML element attributes from registry"""

    def __init__(self,
                 errFile=sys.stderr,
                 warnFile=sys.stderr,
                 diagFile=sys.stdout):
        OutputGenerator.__init__(self, errFile, warnFile, diagFile)

        # ** Global types for automatic source generation **
        # Length Member data
        self.LengthMember = namedtuple('LengthMember',
                                       ['array_name',          # The name of the array
                                        'length_name'])        # The name of the length parameter
        # Handle data
        self.HandleData = namedtuple('HandleData',
                                     [  # The name of the handle
                                         'name',
                                         # The name of the handle's direct parent
                                         'parent',
                                         # A list of all the handle's ancestors
                                         'ancestors',
                                         # None or a comma-delimited list indicating #define values to use around this handle
                                         'protect_value',
                                         # Empty string or string to use after #if to protect this handle
                                         'protect_string'])
        # Flag data
        self.FlagBits = namedtuple('FlagBits',
                                   [  # The name of the flag
                                       'name',
                                       # The base type of the flag (for example uint64_t)
                                       'type',
                                       # The list of valid flag values
                                       'valid_flags',
                                       # None or a comma-delimited list indicating #define values to use around this flag
                                       'protect_value',
                                       # Empty string or string to use after #if to protect this flag
                                       'protect_string',
                                       # Name of extension this command is associated with (or None)
                                       'ext_name'])

        # Individual Enum bit value
        self.EnumBitValue = namedtuple('EnumBitValue',
                                       [  # Name of an individual enum bit
                                           'name',
                                           # None or a comma-delimited list indicating #define values to use around this value
                                           'protect_value',
                                           # Empty string or string to use after #if to protect this value
                                           'protect_string',
                                           # Name of extension this command is associated with (or None)
                                           'ext_name'])
        # Enum type data
        self.EnumData = namedtuple('EnumData',
                                   [  # The name of the enum
                                       'name',
                                       # List of possible EnumBitValue for this enum.
                                       'values',
                                       # None or a comma-delimited list indicating #define values to use around this enum
                                       'protect_value',
                                       # Empty string or string to use after #if to protect this enum
                                       'protect_string',
                                       # Name of extension this command is associated with (or None)
                                       'ext_name'])
        # Struct/Union member or Command parameter data
        self.MemberOrParam = namedtuple('MemberOrParam',
                                        [  # The type of this parameter
                                            'type',
                                            # Boolean indicating if this type is a handle
                                            'is_handle',
                                            # Boolean indicating if this has a const identifier
                                            'is_const',
                                            # Boolean indicating if this is a boolean type
                                            'is_bool',
                                            # Boolean indicating if this is optional (pointers may be NULL, etc)
                                            'is_optional',
                                            # Boolean indicating if this is an array
                                            'is_array',
                                            # Number of dimensions for this array
                                            'array_dimen',
                                            # Boolean indicating if this is a statically sized array
                                            'is_static_array',
                                            # List of static array sizes for each dimension
                                            'static_array_sizes',
                                            # Name of array count if this is a value, and not a number
                                            'array_count_var',
                                            # Indicates the array parameter that this is a length for
                                            'array_length_for',
                                            # Depth of pointers for this type (* = 1, ** == 2)
                                            'pointer_count',
                                            # If this is a pointer, and an array, it's the max size of that array
                                            'pointer_count_var',
                                            # Is this parameter null-terminated
                                            'is_null_terminated',
                                            # Boolean indicating if this should not have any automatic validation performed
                                            'no_auto_validity',
                                            # The parameter name
                                            'name',
                                            # Name of valid extension structs for this param (usually for 'next')
                                            'valid_extension_structs',
                                            # The complete C-declaration for this parameter.
                                            'cdecl',
                                            # None or comma-separated list of valid values
                                            'values'])
        # Command data
        self.CommandData = namedtuple('CommandData',
                                      [  # Name of command
                                          'name',
                                          # Boolean indicating this is a create/connect command
                                          'is_create_connect',
                                          # Boolean indicating this is a destroy/disconnect command
                                          'is_destroy_disconnect',
                                          # Name of extension this command is associated with (or None)
                                          'ext_name',
                                          # Type of extension (instance, device)
                                          'ext_type',
                                          # Additional extensions required for this command to be valid
                                          'required_exts',
                                          # Type of handle used as the primary type for this command
                                          'handle_type',
                                          # Base handle for this command
                                          'handle',
                                          # True if an instance is used somewhere in the command
                                          'has_instance',
                                          # Type of return (or None)
                                          'return_type',
                                          # List of MemberOrParam for each parameter in this command
                                          'params',
                                          # None or a comma-delimited list indicating #define values to use around this command
                                          'protect_value',
                                          # Empty string or string to use after #if to protect this command
                                          'protect_string',
                                          # Boolean indicating this command begins some kind of state
                                          'begins_state',
                                          # Boolean indicating this command ends some kind of state
                                          'ends_state',
                                          # Boolean indicating this command checks for some kind of state
                                          'checks_state',
                                          # The complete C-declaration for this command
                                          'cdecl'])
        # Information regarding a structure or union
        self.StructUnionData = namedtuple('StructUnionData',
                                          [  # Name of the structure or union
                                              'name',
                                              # Name of extension this struct/union is associated with (or None)
                                              'ext_name',
                                              # Additional extensions required for this struct/union to be valid
                                              'required_exts',
                                              # Boolean indicating that this struct/union is only for returning information
                                              'returned_only',
                                              # List of MemberOrParam for each member in this struct/union
                                              'members',
                                              # None or a comma-delimited list indicating #define values to use around this structure/union
                                              'protect_value',
                                              # Empty string or string to use after #if to protect this structure/union
                                              'protect_string'
                                              ])
        # Information on a given extension
        self.ExtensionData = namedtuple('ExtensionData',
                                        [  # Name of this extension (ex. XR_EXT_foo)
                                            'name',
                                            # Vendor tag associated with this extension
                                            'vendor_tag',
                                            # Type of extension (instance, device, ...)
                                            'type',
                                            # Define containing a string version of the extension name
                                            'define',
                                            # Number of commands in the extension
                                            'num_commands',
                                            # List of required extensions for using this extension's functionality
                                            'required_exts',
                                            # None or a comma-delimited list indicating #define values to use around this extension
                                            'protect_value',
                                            # Empty string or string to use after #if to protect this extension
                                            'protect_string'])
        # Base type listing (converts from a type into a name which is used as a type somewhere else).
        self.BaseTypeData = namedtuple('BaseTypeData',
                                       [  # The base type
                                           'type',
                                           # The name of the derived type
                                           'name'])
        # Type listing
        self.TypeData = namedtuple('TypeData',
                                   [  # The name of the type
                                       'name',
                                       # None or a comma-delimited list indicating #define values to use around this type
                                       'protect_value',
                                       # Empty string or string to use after #if to protect this type
                                       'protect_string'])
        # Structure relation group data
        self.StructRelationGroup = namedtuple('StructRelationGroup',
                                              [  # The name of the generic structure defining the common elements
                                                  'generic_struct_name',
                                                  # Children of the base structure
                                                  'child_struct_names'])
        # API state listing
        self.ApiState = namedtuple('ApiState',
                                   [  # The name of the state
                                       'state',
                                       # The type that is associated with the state (handle/struct/...)
                                       'type',
                                       # State variable
                                       'variable',
                                       # List of commands that begin the current state
                                       'begin_commands',
                                       # List of commands that end the current state
                                       'end_commands',
                                       # List of commands that check the current state
                                       'check_commands'])
        # Did we encounter an error
        self.encountered_error = False
        # List of strings containing all vendor tags
        self.vendor_tags = []
        # Current vendor tag that should be used by this extension
        self.current_vendor_tag = ''
        # A define used to set the current API version in a component (in the loader, layers, etc).
        self.api_version_define = ''
        # A defined used to grab the current API Header's version
        self.header_version = ''
        # A list of all the API's core commands (CommandData).
        self.core_commands = []
        # A list of all the API's extension commands (CommandData).
        self.ext_commands = []
        # A list of all extensions (ExtensionData) for this API
        self.extensions = []
        # A list of all base data types (BaseTypeData) for this API
        self.api_base_types = []
        # A list of all handles (HandleData) for this API
        self.api_handles = []
        # A list of all structures (StructUnionData) for this API
        self.api_structures = []
        # A list of all unions (StructUnionData) for this API
        self.api_unions = []
        # A list of all enumeration (EnumData) for this API
        self.api_enums = []
        # A list of all flags (FlagBits) for this API
        self.api_flags = []
        # A list of all bitmasks (EnumData) for this API
        self.api_bitmasks = []
        # A list of all object types
        self.api_object_types = []
        # A list of all result types
        self.api_result_types = []
        # A list of all structure types
        self.api_structure_types = []
        # A list of all struct relation groups
        self.struct_relation_groups = []
        # A list of all the API states
        self.api_states = []
        # Max lengths for various items
        self.max_extension_name_length = 32
        self.max_structure_type_length = 32
        self.max_result_length = 32
        self.structs_with_no_type = ['XrBaseInStructure', 'XrBaseOutStructure']

    # This is the basic warning about the source file being generated.  It can be
    # overridden by a derived class.
    #   self            the AutomaticSourceOutputGenerator object
    def outputGeneratedHeaderWarning(self):
        generated_warning = '// *********** THIS FILE IS GENERATED - DO NOT EDIT ***********\n'
        generated_warning += '//     See automatic_source_generator.py for modifications\n'
        generated_warning += '// ************************************************************\n'
        write(generated_warning, file=self.outFile)

    # The author tag can vary per python file, so let it be overridden
    #   self            the AutomaticSourceOutputGenerator object
    def outputGeneratedAuthorNote(self):
        author_note = '//\n'
        author_note += '// Author: Mark Young <marky@lunarg.com>\n'
        author_note += '//\n'
        write(author_note, file=self.outFile)

    # Print an error message that will clearly identify any problems encountered during the
    # code generation process, and return a string that can be inserted to break a compile.
    #   self            the AutomaticSourceOutputGenerator object
    #   message         the message to display
    #   file            the name of the file encountering the problem (or None, default, to autodetect)
    #   line            the line number the failure occurred on (or None, default, to autodetect)
    def printCodeGenErrorMessage(self, message, file=None, line=None):
        if file is None or line is None:
            frame = getframeinfo(currentframe().f_back)
            if file is None:
                file = frame.filename
            if line is None:
                line = frame.lineno + 1
        # The filename we get is the full path file name.  So trim it down to only the
        # last two folders
        trimmed_file = file
        second_last_dir = file.rfind('/', 0, file.rfind('/'))
        if second_last_dir < 0:
            second_last_dir = file.rfind('\\', 0, file.rfind('\\'))
        if second_last_dir > 0:
            trimmed_file = file[second_last_dir + 1:]
        # Set the error flag so we can insert a failure into the build
        self.encountered_error = True
        # Write the message
        print('**CODEGEN_ERROR: %s[%d] %s' % (trimmed_file, line, message))
        return '#{} "{}"\n#error("Bug: {}")\n'.format(line, file, message)

    # Print a warning message the will clearly identify any possible problems encountered
    # during the code generation process
    #   self            the AutomaticSourceOutputGenerator object
    #   file            the name of the file encountering the problem
    #   line            the line number the failure occurred on
    #   message         the message to display
    def printCodeGenWarningMessage(self, file, line, message):
        # The filename we get is the full path file name.  So trim it down to only the
        # last two folders
        trimmed_file = file
        second_last_dir = file.rfind('/', 0, file.rfind('/'))
        if second_last_dir < 0:
            second_last_dir = file.rfind('\\', 0, file.rfind('\\'))
        if second_last_dir > 0:
            trimmed_file = file[second_last_dir + 1:]
        # Write the message
        print('**CODEGEN_WARNING: %s[%d] %s' % (trimmed_file, line, message))

    # Insert an #error line into the source file so the build fails if a failure occurs
    # during the codegen process.  This is so that the CI system can fail when someone introduces
    # some change that breaks code generation so it is obvious and can be fixed prior to being
    # checked into the tree.
    #   self            the AutomaticSourceOutputGenerator object
    def outputErrorIfNeeded(self):
        if self.encountered_error:
            write('#error("Encountered error during generation, inserted so CI catches this failure")\n', file=self.outFile)

    # This is the basic copyright for the source file being generated.  It can be
    # overridden by a derived class.
    #   self            the AutomaticSourceOutputGenerator object
    def outputCopywriteHeader(self):
        copyright = '// Copyright (c) 2017-2019 The Khronos Group Inc.\n'
        copyright += '// Copyright (c) 2017-2019 Valve Corporation\n'
        copyright += '// Copyright (c) 2017-2019 LunarG, Inc.\n'
        copyright += '//\n'
        copyright += '// Licensed under the Apache License, Version 2.0 (the "License");\n'
        copyright += '// you may not use this file except in compliance with the License.\n'
        copyright += '// You may obtain a copy of the License at\n'
        copyright += '//\n'
        copyright += '//     http://www.apache.org/licenses/LICENSE-2.0\n'
        copyright += '//\n'
        copyright += '// Unless required by applicable law or agreed to in writing, software\n'
        copyright += '// distributed under the License is distributed on an "AS IS" BASIS,\n'
        copyright += '// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n'
        copyright += '// See the License for the specific language governing permissions and\n'
        copyright += '// limitations under the License.'
        write(copyright, file=self.outFile)

    # Called once at the beginning of each run.  Starts writing to the output
    # file, including the header warning and copyright.
    #   self            the AutomaticSourceOutputGenerator object
    #   gen_opts        the AutomaticSourceGeneratorOptions object
    def beginFile(self, gen_opts):
        OutputGenerator.beginFile(self, gen_opts)

        # Iterate over all 'tag' Elements and add the names of all the valid vendor
        # tags to the list
        root = self.registry.tree.getroot()
        for tag in root.findall('tags/tag'):
            self.vendor_tags.append(tag.get('name'))

        # User-supplied prefix text, if any (list of strings)
        if (gen_opts.prefixText):
            for s in gen_opts.prefixText:
                write(s, file=self.outFile)
        self.outputGeneratedHeaderWarning()
        self.outputCopywriteHeader()
        self.outputGeneratedAuthorNote()

    # Write out all the information for the appropriate file,
    # and then call down to the base class to wrap everything up.
    #   self            the AutomaticSourceOutputGenerator object
    def endFile(self):
        self.outputErrorIfNeeded()
        # Finish processing in superclass
        OutputGenerator.endFile(self)

    # This is called for the beginning of each API feature.  Each feature includes a set of
    # API types, commands, etc.  Each feature is identified by a name, this includes the API
    # core items (as <API>_VERSION_...) or extension items (by the extension name define
    # EXTENSION_NAME_...).
    #   self            the AutomaticSourceOutputGenerator object
    #   interface       element for the <version> / <extension> to generate
    #   emit            actually write to the header only when True (ignored in this class)
    def beginFeature(self, interface, emit):
        # Start processing in superclass
        OutputGenerator.beginFeature(self, interface, emit)

        # Reset the values for the new feature
        self.currentExtension = ''
        self.name_definition = ''
        self.type = interface.get('type')
        self.num_commands = 0
        self.currentExtension = interface.get('name')
        self.required_exts = []

        # Look through all the enums for this XML feature.  If one of them contains
        # a 'name' field, and that 'name' field contains "EXTENSION_NAME", then we're
        # dealing with the beginning of an extension, and not a core group of functionality.
        enums = interface[0].findall('enum')
        for item in enums:
            name_definition = item.get('name')
            if 'EXTENSION_NAME' in name_definition:
                self.name_definition = name_definition
                break

        # If this is not part of an XR core, it requires the extension provided in the
        # feature's name to be enabled for this functionality to be valid.
        if not self.isCoreExtensionName(self.currentExtension):
            if len(self.currentExtension) + 1 > self.max_extension_name_length:
                self.printCodeGenErrorMessage('Extension name %s length %d in XML is greater'
                                              ' than allowable space of %d when null-terminator is added' % (
                                                  self.currentExtension, len(self.currentExtension), self.max_extension_name_length))
            self.required_exts.append(self.currentExtension)
            # Make sure the extension has the proper vendor tags
            valid_extension_vendor = False
            for cur_vendor_tag in self.vendor_tags:
                extension_prefix_with_tag = "XR_"
                extension_prefix_with_tag += cur_vendor_tag
                extension_prefix_with_tag += "_"
                if self.currentExtension.startswith(extension_prefix_with_tag):
                    valid_extension_vendor = True
                    # Save the extension tag to check every type, etc with
                    self.current_vendor_tag = cur_vendor_tag
            if not valid_extension_vendor:
                self.printCodeGenErrorMessage('Extension %s does not appear to begin with a'
                                              ' valid vendor tag! (for example XR_KHR_)' % self.currentExtension)
                # Set the tag to something invalid so we can explicitly identify each type related to
                # this unknown extension
                self.current_vendor_tag = "UNKNOWN"

        # Add any defined extension dependencies to the base dependency list for this extension
        requires = interface.get('requires')
        if requires:
            self.required_exts.extend(requires.split(','))

    # Called on the completion of an XML feature
    #   self            the AutomaticSourceOutputGenerator object
    def endFeature(self):
        # TODO: Skip Android extensions for now since they don't have a protect clause.
        if 'android' not in self.currentExtension:
            (protect_value, protect_string) = self.genProtectInfo(
                self.featureExtraProtect, None)
            if not self.isCoreExtensionName(self.currentExtension):
                if self.type == 'instance' or self.type == 'system':
                    # Add the completed extension to the list of extensions
                    self.extensions.append(
                        self.ExtensionData(
                            name=self.currentExtension,
                            vendor_tag=self.current_vendor_tag,
                            type=self.type,
                            protect_value=protect_value,
                            protect_string=protect_string,
                            define=self.name_definition,
                            num_commands=self.num_commands,
                            required_exts=self.required_exts))
                else:
                    self.printCodeGenErrorMessage('Extension %s is neither an instance or system extension but '
                                                  ' an unknown type \"%s\"' % (
                                                      self.currentExtension, self.type))
        OutputGenerator.endFeature(self)

    # Process commands, adding to appropriate dispatch tables
    #   self            the AutomaticSourceOutputGenerator object
    #   cmd_info        the XML information for this command
    #   name            the name of the command
    def genCmd(self, cmd_info, name):
        OutputGenerator.genCmd(self, cmd_info, name)

        self.num_commands = self.num_commands + 1
        self.addCommandToDispatchList(
            self.currentExtension, self.type, name, cmd_info)

    # Process a group of items.  We're typically interested in
    # enumeration values or flag bitmasks.
    #   self            the AutomaticSourceOutputGenerator object
    #   cmd_info        the XML information for this group
    #   name            the name of the group
    def genGroup(self, group_info, name):
        OutputGenerator.genGroup(self, group_info, name)
        group_type = group_info.elem.get('type')
        group_supported = group_info.elem.get('supported') != 'disabled'
        is_extension = not self.isCoreExtensionName(self.currentExtension)
        # We really only care to handle enum or bitmask values here
        if group_supported and ('enum' == group_type or 'bitmask' == group_type):
            values = []
            (top_protect_value, top_protect_string) = self.genProtectInfo(
                self.featureExtraProtect, group_info.elem.get('protect'))
            # Both are sorted under "enum" tags.  So grab all the valid
            # field values.
            for elem in group_info.elem.findall('enum'):
                if elem.get('supported') != 'disabled':
                    (enum_protect_value, enum_protect_string) = self.genProtectInfo(
                        self.featureExtraProtect, elem.get('protect'))
                    elem_name = elem.get('name')
                            # TODO this variable is never read
                    if is_extension and not elem_name.endswith(self.current_vendor_tag):
                        self.printCodeGenErrorMessage('Enum value %s in XML (for extension %s) does'
                                                      ' not end with the expected vendor tag \"%s\"' % (
                                                          elem_name, self.currentExtension, self.current_vendor_tag))
                    extension_to_check = self.currentExtension
                    if elem.get('extname') is not None:
                        extension_to_check = elem.get('extname')
                    values.append(
                        self.EnumBitValue(
                            name=elem_name,
                            protect_value=enum_protect_value,
                            protect_string=enum_protect_string,
                            ext_name=extension_to_check))
            if 'enum' == group_type:
                if is_extension and not name.endswith(self.current_vendor_tag):
                    self.printCodeGenErrorMessage('Enum %s in XML (for extension %s) does not end'
                                                  ' with the expected vendor tag \"%s\"' % (
                                                      name, self.currentExtension, self.current_vendor_tag))
                self.api_enums.append(
                    self.EnumData(
                        name=name,
                        values=values,
                        protect_value=top_protect_value,
                        protect_string=top_protect_string,
                        ext_name=self.currentExtension))
            else:
                if is_extension and not name.endswith(self.current_vendor_tag):
                    self.printCodeGenErrorMessage('Bitmask %s in XML (for extension %s) does not'
                                                  ' end with the expected vendor tag \"%s\"' % (
                                                      name, self.currentExtension, self.current_vendor_tag))
                self.api_bitmasks.append(
                    self.EnumData(
                        name=name,
                        values=values,
                        protect_value=top_protect_value,
                        protect_string=top_protect_string,
                        ext_name=self.currentExtension))
        # Check all the XrResult values
        if name == 'XrResult':
            for elem in group_info.elem.findall('enum'):
                if elem.get('supported') != 'disabled':
                    (protect_value, protect_string) = self.genProtectInfo(
                        self.featureExtraProtect, elem.get('protect'))
                    item_name = elem.get('name')
                    if None != item_name:
                        if is_extension and not item_name.endswith(self.current_vendor_tag):
                            self.printCodeGenErrorMessage('XrResult %s in XML (for extension %s) does'
                                                          ' not end with the expected vendor tag \"%s\"' % (
                                                              item_name, self.currentExtension, self.current_vendor_tag))
                        if len(item_name) + 1 > self.max_result_length:
                            self.printCodeGenErrorMessage('XrResult %s length %d in XML is greater'
                                                          ' than allowable space of %d when null-terminator is added' % (
                                                              item_name, len(item_name), self.max_result_length))
                        self.api_result_types.append(
                            self.TypeData(
                                name=item_name,
                                protect_value=protect_value,
                                protect_string=protect_string))
        # Separately save a list of all the API object types we care about
        elif name == 'XrObjectType':
            for elem in group_info.elem.findall('enum'):
                if elem.get('supported') != 'disabled':
                    (protect_value, protect_string) = self.genProtectInfo(
                        self.featureExtraProtect, elem.get('protect'))
                    item_name = elem.get('name')
                    if None != item_name:
                        if is_extension and not item_name.endswith(self.current_vendor_tag):
                            self.printCodeGenErrorMessage('ObjectType %s in XML (for extension %s) does'
                                                          ' not end with the expected vendor tag \"%s\"' % (
                                                              item_name, self.currentExtension, self.current_vendor_tag))
                        self.api_object_types.append(
                            self.TypeData(
                                name=item_name,
                                protect_value=protect_value,
                                protect_string=protect_string))
        # Separately save a list of all the API structure types we care about
        elif name == 'XrStructureType':
            for elem in group_info.elem.findall('enum'):
                if elem.get('supported') != 'disabled':
                    (protect_value, protect_string) = self.genProtectInfo(
                        self.featureExtraProtect, elem.get('protect'))
                    item_name = elem.get('name')
                    if None != item_name:
                        if len(item_name) + 1 > self.max_structure_type_length:
                            self.printCodeGenErrorMessage('StructureType %s length %d in XML (for'
                                                          ' extension %s) is greater than allowable'
                                                          ' space of %d when null-terminator is added' % (
                                                              item_name, len(item_name), self.currentExtension, self.max_structure_type_length))
                        if is_extension and not item_name.endswith(self.current_vendor_tag):
                            self.printCodeGenErrorMessage('StructureType %s in XML (for extension'
                                                          ' %s) does not end with the expected'
                                                          ' vendor tag \"%s\"' % (
                                                              item_name, self.currentExtension, self.current_vendor_tag))
                        self.api_structure_types.append(
                            self.TypeData(
                                name=item_name,
                                protect_value=protect_value,
                                protect_string=protect_string))

    # Get the category of a type
    #   self            the AutomaticSourceOutputGenerator object
    #   type_name       the name of the type to evaluate
    def getTypeCategory(self, type_name):
        types = self.registry.tree.findall("types/type")
        for elem in types:
            if (elem.find("name") is not None and elem.find('name').text == type_name) or elem.attrib.get('name') == type_name:
                return elem.attrib.get('category')

    # Retrieve the type and name for a parameter
    #   self            the AutomaticSourceOutputGenerator object
    #   param           the XML parameter information to access
    def getTypeNameTuple(self, param):
        type = ''
        name = ''
        for elem in param:
            if elem.tag == 'type':
                type = noneStr(elem.text)
            elif elem.tag == 'name':
                name = noneStr(elem.text)
        return (type, name)

    # Retrieve the type, name, and enum for a parameter
    #   self            the AutomaticSourceOutputGenerator object
    #   param           the XML parameter information to access
    def getTypeNameEnumTuple(self, param):
        type = ''
        name = ''
        enum = ''
        for elem in param:
            if elem.tag == 'type':
                type = noneStr(elem.text)
            elif elem.tag == 'name':
                name = noneStr(elem.text)
            elif elem.tag == 'enum':
                enum = noneStr(elem.text)
        return (type, name, enum)

    # Retrieve the value of the len tag
    #   self            the AutomaticSourceOutputGenerator object
    #   param           the XML parameter information to access
    def getLen(self, param):
        result = None
        len_attrib = param.attrib.get('len')
        if len_attrib and len_attrib != 'null-terminated':
            # For string arrays, 'len' can look like 'count,null-terminated',
            # indicating that we have a null terminated array of strings.  We
            # strip the null-terminated from the 'len' field and only return
            # the parameter specifying the string count
            if 'null-terminated' in len_attrib:
                result = len_attrib.split(',')[0]
            else:
                result = len_attrib
            result = str(result).replace('::', '->')
        return result

    # Override base class genType command so we can grab more information about the
    # necessary types.
    #   self            the AutomaticSourceOutputGenerator object
    #   type_info       the XML information for this type
    #   type_name       the name of this type
    def genType(self, type_info, type_name):
        OutputGenerator.genType(self, type_info, type_name)
        type_elem = type_info.elem
        type_category = type_elem.get('category')
        (protect_value, protect_string) = self.genProtectInfo(
            self.featureExtraProtect, type_elem.get('protect'))
        has_proper_ending = True
        if not self.isCoreExtensionName(self.currentExtension) and not type_name.endswith(self.current_vendor_tag):
            has_proper_ending = False
        if type_category == 'struct' or type_category == 'union':
            if not has_proper_ending:
                self.printCodeGenErrorMessage('Struct/union %s in XML (for extension %s) does not end with the expected vendor tag \"%s\"' % (
                    type_name, self.currentExtension, self.current_vendor_tag))
            self.genStructUnion(type_info, type_category, type_name)
        elif type_category == 'handle':
            if not has_proper_ending:
                self.printCodeGenErrorMessage('Handle %s in XML (for extension %s) does not end with the expected vendor tag \"%s\"' % (
                    type_name, self.currentExtension, self.current_vendor_tag))
            # Save this if it's a handle so that we can treat it differently.  Things we
            # need handle for are:
            #  - We need to know when it's created and destroyed,
            #  - We need to setup unordered_maps and mutexes to track dispatch tables for each handle
            self.api_handles.append(
                self.HandleData(
                    name=type_name,
                    parent=self.getHandleParent(type_name),
                    ancestors=self.getHandleAncestors(type_name),
                    protect_value=protect_value,
                    protect_string=protect_string))
        elif type_category == 'basetype':
            # Save the base type information just so we can convert to the base type when
            # outputting to a file.
            basetype_info = self.getTypeNameTuple(type_info.elem)
            base_type = basetype_info[0]
            base_name = basetype_info[1]
            self.api_base_types.append(
                self.BaseTypeData(
                    type=base_type,
                    name=base_name))
        elif type_category == 'define':
            if "XR_CURRENT_API_VERSION" in type_name:
                # The API Version (most importantly, the API Major and Minor version)
                self.api_version_define = type_name
            elif type_name == 'XR_HEADER_VERSION':
                nameElem = type_elem.find('name')
                # The API Header Version (typically used as the patch or build version)
                self.header_version = noneStr(nameElem.tail).strip()
        elif type_category == 'bitmask':
            if not has_proper_ending:
                self.printCodeGenErrorMessage('Bitmask %s in XML (for extension %s) does not end with the expected vendor tag \"%s\"' % (
                    type_name, self.currentExtension, self.current_vendor_tag))
            mask_info = self.getTypeNameTuple(type_info.elem)
            mask_type = mask_info[0]
            mask_name = mask_info[1]
            bitvalues = type_elem.get('bitvalues')
            extension_to_check = self.currentExtension
            if type_elem.get('extname') is not None:
                extension_to_check = type_elem.get('extname')
            # Record a bitmask and all it's valid flag bit values
            self.api_flags.append(
                self.FlagBits(
                    name=mask_name,
                    type=mask_type,
                    valid_flags=bitvalues,
                    protect_value=protect_value,
                    protect_string=protect_string,
                    ext_name=extension_to_check))

    # Enumerant generation
    #   self            the AutomaticSourceOutputGenerator object
    #   enum_info       the XML information for this enum
    #   name            the name of this enum
    def genEnum(self, enum_info, name):
        if name == 'XR_MAX_EXTENSION_NAME_SIZE':
            self.max_extension_name_length = int(enum_info.elem.get('value'))
        if name == 'XR_MAX_STRUCTURE_NAME_SIZE':
            self.max_structure_type_length = int(enum_info.elem.get('value'))
        if name == 'XR_MAX_RESULT_STRING_SIZE':
            self.max_result_length = int(enum_info.elem.get('value'))

    # Generate the protection information based on the feature-level and command/type-level
    # protection statements.
    #   self            the AutomaticSourceOutputGenerator object
    #   feature_protect None or the feature's protection statement
    #   local_protect   None or the local type/command's protection statement
    def genProtectInfo(self, feature_protect, local_protect):
        protect_type = None
        protect_str = ''
        protect_list = []
        # If the feature adding this struct/union contains a protect message, add it.
        if self.featureExtraProtect:
            protect_type = self.featureExtraProtect
        # If the struct/union itself has a protect message, add it.
        if local_protect:
            if protect_type:
                protect_type += ','
                protect_type += local_protect
            else:
                protect_type = local_protect
        # If any protection was found, let's generate a string to use for the #if statements
        if protect_type:
            if ',' in protect_type:
                protect_list.extend(protect_type.split(","))
                count = 0
                for protect_define in protect_list:
                    if count > 0:
                        protect_str += ' && '
                    protect_str += 'defined(%s)' % protect_define
                    count = count + 1
            else:
                protect_str += 'defined(%s)' % protect_type
        return (protect_type, protect_str)
    # Struct/Union member generation.
    # This is a special case of the <type> tag where the contents are interpreted as a set of <member> tags instead of freeform C
    # type declarations. The <member> tags are just like <param> tags - they are a declaration of a struct or union member.
    # Only simple member declarations are supported (no nested structs etc.)
    #   self            the AutomaticSourceOutputGenerator object
    #   type_info       the XML information for this type
    #   type_category   the category of this type.  In this case 'union' or 'struct'.
    #   type_name       the name of this type

    def genStructUnion(self, type_info, type_category, type_name):
        OutputGenerator.genStruct(self, type_info, type_name)
        is_union = type_category == 'union'
        (protect_value, protect_string) = self.genProtectInfo(
            self.featureExtraProtect, type_info.elem.get('protect'))
        members = type_info.elem.findall('.//member')
        required_exts = self.required_exts
        if type_info.elem.get('extname'):
            ext_names = type_info.elem.get('extname')
            required_exts.extend(ext_names.split(','))
        returned_only = False
        if type_info.elem.get('returnedonly') != None and type_info.elem.get('returnedonly') == "true":
            returned_only = True

        # Search through the members to determine all the array lengths
        arraylengths = []
        for member in members:
            membername = member.find('name')
            arraylength = member.attrib.get('len')
            if arraylength is not None:
                for onelength in arraylength.split(','):
                    if 'null-terminated' not in onelength:
                        arraylengths.append(self.LengthMember(array_name=membername.text,
                                                              length_name=onelength))

        # Generate member info
        members_info = []
        for member in members:
            # Get the member's type, enum and name
            member_info = self.getTypeNameEnumTuple(member)
            member_type = member_info[0]
            member_name = member_info[1]
            member_enum = member_info[2]

            # Initialize some flags about this member
            static_array_sizes = []
            no_auto_validity = True if member.attrib.get(
                'noautovalidity') else False
            is_optional = True if (self.paramIsOptional(
                member) or (member_name == 'next')) else False
            is_handle = self.isHandle(member_type)
            is_static_array = self.paramIsStaticArray(member)
            is_array = is_static_array
            array_count_var = ''
            array_name_for_length = ''
            pointer_count_var = ''
            is_null_terminated = False
            member_values = member.attrib.get('values')

            # Determine if this is an array length member
            for arraylength in arraylengths:
                if member_name == arraylength.length_name:
                    array_name_for_length = arraylength.array_name
                    break
            # Determine if this is a null-terminated array
            if member and member.attrib and member.attrib.get('len'):
                is_null_terminated = 'null-terminated' in member.attrib.get(
                    'len')
            cdecl = self.makeCParamDecl(member, 0)
            is_const = True if 'const' in cdecl else False
            pointer_count = self.paramPointerCount(
                cdecl, member_type, member_name)
            array_dimen = self.paramArrayDimension(
                cdecl, member_type, member_name)

            # If this is a static array, grab the sizes
            if is_static_array:
                static_array_sizes = self.paramStaticArraySizes(member)

            # If the enum field is there, then this is an array with an enum
            # static size.
            if '' != member_enum:
                is_array = True
                array_count_var = member_enum

            # If this member has a "len" tag, then it is a pointer to an array
            # with a restricted length.
            if member.attrib.get('len'):
                is_array = True

                # Get the name of the variable to use for the count.  Many times,
                # the length also includes a "null-terminated" descriptor which
                # we want to strip here.
                pointer_count_var = noneStr(member.attrib.get('len'))
                null_term_loc = pointer_count_var.lower().rfind('null-terminated')
                if null_term_loc == 0:
                    null_term_len = len("null-terminated")
                    pointer_count_var = pointer_count_var[null_term_len - 1:]
                elif null_term_loc > 0:
                    pointer_count_var = pointer_count_var[0:null_term_loc]

                # If there's a comma, only grab up to the comma
                comma_loc = pointer_count_var.rfind(',')
                if comma_loc == 0:
                    pointer_count_var = pointer_count_var[1:]
                elif comma_loc > 0:
                    pointer_count_var = pointer_count_var[0:comma_loc]

                # If there's not much useful data, just set it to empty
                if len(pointer_count_var) <= 1:
                    pointer_count_var = ''

            # Append this member to the list of current members
            members_info.append(
                self.MemberOrParam(type=member_type,
                                   name=member_name,
                                   is_const=is_const,
                                   is_handle=is_handle,
                                   is_bool=True if 'XrBool' in member_type else False,
                                   is_optional=is_optional,
                                   no_auto_validity=no_auto_validity,
                                   valid_extension_structs=self.registry.validextensionstructs[
                                       type_name] if member_name == 'next' else None,
                                   is_array=is_array,
                                   is_static_array=is_static_array,
                                   static_array_sizes=static_array_sizes,
                                   array_dimen=array_dimen,
                                   array_count_var=array_count_var,
                                   array_length_for=array_name_for_length,
                                   pointer_count=pointer_count,
                                   pointer_count_var=pointer_count_var,
                                   is_null_terminated=is_null_terminated,
                                   cdecl=cdecl,
                                   values=member_values))
        # If this structure/union expands a generic one, save the information and validate that
        # it contains at least the same elements as the generic one.
        if type_info.elem.get('parentstruct'):
            generic_struct_name = type_info.elem.get('parentstruct')
            if self.isStruct(generic_struct_name):
                found = False
                relation_group = None

                # First, determine if it is or is not already a relation group
                for cur_relation_group in self.struct_relation_groups:
                    if cur_relation_group.generic_struct_name == generic_struct_name:
                        found = True
                        relation_group = cur_relation_group
                        break
                if not found:
                    # Create with an empty child list for now
                    child_list = []
                    relation_group = self.StructRelationGroup(generic_struct_name=generic_struct_name,
                                                              child_struct_names=child_list)
                    self.struct_relation_groups.append(relation_group)

                # Get the structure information for the group's generic structure
                generic_struct = self.getStruct(generic_struct_name)
                base_member_count = len(generic_struct.members)

                # Second, it must have at least the same number of members
                if len(members) >= base_member_count:
                    members_match = True
                    # Third, the first 'n' elements must match in name and type
                    for mem in range(base_member_count):
                        if (members_info[mem].name != generic_struct.members[mem].name or
                                members_info[mem].type != generic_struct.members[mem].type):
                            members_match = False
                            break
                    if members_match:
                        relation_group.child_struct_names.append(type_name)
                    else:
                        frameinfo = getframeinfo(currentframe())
                        self.printCodeGenWarningMessage(frameinfo.filename, frameinfo.lineno + 1,
                                                        'Struct \"%s\" has different children than possible parent struct \"%s\".' % (
                                                            type_name, generic_struct_name))
        if is_union:
            self.api_unions.append(
                self.StructUnionData(name=type_name,
                                     ext_name=self.currentExtension,
                                     required_exts=required_exts,
                                     protect_value=protect_value,
                                     protect_string=protect_string,
                                     returned_only=returned_only,
                                     members=members_info))
        else:
            self.api_structures.append(
                self.StructUnionData(name=type_name,
                                     ext_name=self.currentExtension,
                                     required_exts=required_exts,
                                     protect_value=protect_value,
                                     protect_string=protect_string,
                                     returned_only=returned_only,
                                     members=members_info))

    # Add a command to the appropriate list of commands (core or extension)
    #   self            the AutomaticSourceOutputGenerator object
    #   ext_name        the name of the extension this command is part of
    #   ext_type        the type of the extension (instance/device)
    #   name            the name of the command
    #   cmd_info        the XML information for this command
    def addCommandToDispatchList(self, ext_name, ext_type, name, cmd_info):
        cmd_params = []
        required_exts = []
        is_core = True if self.isCoreExtensionName(ext_name) else False
        cmd_has_instance = False
        is_create_connect = False
        is_destroy_disconnect = False

        # Generate the protection information
        (protect_value, protect_string) = self.genProtectInfo(
            self.featureExtraProtect, cmd_info.elem.find('protect'))

        # Get parameters for this command
        params = cmd_info.elem.findall('param')
        handle_type = self.getTypeNameTuple(params[0])[0]
        handle = self.registry.tree.find(
            "types/type/[name='" + handle_type + "'][@category='handle']")
        return_type = cmd_info.elem.find('proto/type')

        # If the return type is void, we really don't have a return type so set it to None
        if (return_type is not None and return_type.text == 'void'):
            return_type = None

        if not is_core:
            required_exts.append(ext_name)

        # Identify this command as either a create or destroy command for later use
        if 'xrCreate' in name or 'Connect' in name:
            is_create_connect = True
        elif 'xrDestroy' in name or 'Disconnect' in name:
            is_destroy_disconnect = True

        # Search through the parameters to determine all the array lengths
        arraylengths = []
        for param in params:
            paramname = param.find('name')
            arraylength = param.attrib.get('len')
            if arraylength is not None:
                for onelength in arraylength.split(','):
                    if 'null-terminated' not in onelength:
                        arraylengths.append(self.LengthMember(array_name=paramname.text,
                                                              length_name=onelength))

        # See if this command adjusts any state
        begin_valid_state = cmd_info.elem.get('beginvalidstate')
        begins_state = (None != begin_valid_state)
        end_valid_state = cmd_info.elem.get('endvalidstate')
        ends_state = (None != end_valid_state)
        check_valid_state = cmd_info.elem.get('checkvalidstate')
        checks_state = (None != check_valid_state)

        # If a beginvalidstate string exists, add it to the list of tracked states
        if begins_state:
            self.addCommandToBeginStates(begin_valid_state, name)
        # If a endvalidstate string exists, add it to the list of tracked states
        if ends_state:
            self.addCommandToEndStates(end_valid_state, name)
        # If a checkvalidstate string exists, add it to the list of tracked states
        if checks_state:
            self.addCommandToCheckStates(check_valid_state, name)

        # Generate a list of commands
        for param in params:
            is_array = self.paramIsStaticArray(param)
            param_cdecl = self.makeCParamDecl(param, 0)
            param_len = ''
            array_count_var = ''
            pointer_count_var = ''
            array_name_for_length = ''
            is_null_terminated = False
            if param and param.attrib and param.attrib.get('len'):
                is_null_terminated = 'null-terminated' in param.attrib.get(
                    'len')

            # Get the basics of the parameter that we need (type and name) and
            # any info about pointers and arrays we can grab.
            paramInfo = self.getTypeNameTuple(param)
            param_type = paramInfo[0]
            param_name = paramInfo[1]
            pointer_count = self.paramPointerCount(
                param_cdecl, param_type, param_name)
            array_dimen = self.paramArrayDimension(
                param_cdecl, param_type, param_name)

            # Determine if this is an array length parameter
            for arraylength in arraylengths:
                if param_name == arraylength.length_name:
                    array_name_for_length = arraylength.array_name
                    break

            # Determine if this is a null-terminated array
            if param and param.attrib and param.attrib.get('len'):
                is_null_terminated = 'null-terminated' in param.attrib.get(
                    'len')

            # If this is an instance, remember it since we have to treat instance cases
            # special
            if param_type == 'XrInstance':
                cmd_has_instance = True

            # Determine if this is a pointer array with a length variable
            if self.getLen(param):
                param_len = self.getLen(param)
                if '' != param_len:
                    is_array = True
                    pointer_count_var = param_len

            # If this is a handle, and it is a pointer, it really must also be an array unless it is a create command
            if (self.isHandle(param_type) and pointer_count > 0 and not (is_create_connect or is_array or self.paramIsStaticArray(param) or len(array_count_var) > 0 or len(pointer_count_var) > 0)):
                self.printCodeGenErrorMessage('OpenXR command %s has parameter %s which is a non-array pointer to a handle and is not a create command' % (
                    name, param_name))

            # Add the command parameter to the list
            cmd_params.append(
                self.MemberOrParam(
                    type=param_type,
                    name=param_name,
                    is_const=True if 'const' in param_cdecl.strip().lower() else False,
                    is_handle=self.isHandle(param_type),
                    is_bool=True if 'XrBool' in paramInfo[0] else False,
                    is_optional=self.paramIsOptional(param),
                    no_auto_validity=True if param.attrib.get(
                        'noautovalidity') else False,
                    is_array=is_array,
                    is_static_array=self.paramIsStaticArray(param),
                    static_array_sizes=self.paramStaticArraySizes(
                        param) if self.paramIsStaticArray(param) else None,
                    array_dimen=array_dimen,
                    array_count_var=array_count_var,
                    array_length_for=array_name_for_length,
                    pointer_count=pointer_count,
                    pointer_count_var=pointer_count_var,
                    is_null_terminated=is_null_terminated,
                    valid_extension_structs=None,
                    cdecl=param_cdecl,
                    values=None))

        # If this is a create or destroy that returns a handle, it must have a return type.
        if ((is_create_connect or is_destroy_disconnect) and cmd_params[-1].is_handle and
                (return_type is None or return_type.text != 'XrResult')):
            self.printCodeGenErrorMessage(
                'OpenXR create/destroy command %s requires an XrResult return value' % name)

        # The Core OpenXR code will be wrapped in a feature called XR_VERSION_#_#
        # For example: XR_VERSION_1_0 wraps the core 1.0 OpenXR functionality
        if is_core:
            core_command_type = 'instance'
            if handle and handle_type != 'XrInstance':
                core_command_type = 'device'
            self.core_commands.append(
                self.CommandData(name=name,
                                 is_create_connect=is_create_connect,
                                 is_destroy_disconnect=is_destroy_disconnect,
                                 ext_name=ext_name,
                                 ext_type=core_command_type,
                                 required_exts=required_exts,
                                 protect_value=protect_value,
                                 protect_string=protect_string,
                                 return_type=return_type,
                                 handle=handle,
                                 handle_type=handle_type,
                                 has_instance=cmd_has_instance,
                                 params=cmd_params,
                                 begins_state=begins_state,
                                 ends_state=ends_state,
                                 checks_state=checks_state,
                                 cdecl=self.makeCDecls(cmd_info.elem)[0]))
        else:
            self.ext_commands.append(
                self.CommandData(name=name,
                                 is_create_connect=is_create_connect,
                                 is_destroy_disconnect=is_destroy_disconnect,
                                 ext_name=ext_name,
                                 ext_type=ext_type,
                                 required_exts=required_exts,
                                 protect_value=protect_value,
                                 protect_string=protect_string,
                                 return_type=return_type,
                                 handle=handle,
                                 handle_type=handle_type,
                                 has_instance=cmd_has_instance,
                                 params=cmd_params,
                                 begins_state=begins_state,
                                 ends_state=ends_state,
                                 checks_state=checks_state,
                                 cdecl=self.makeCDecls(cmd_info.elem)[0]))

    def findState(self, state):
        found_state = None
        for api_state in self.api_states:
            if api_state.state == state:
                return api_state
        return None

    # Add this command to any begin state checks (used for validation)
    #   self              the AutomaticSourceOutputGenerator object
    #   comma_list_states a comma-delimited string containing the list
    #                     of states that need to be begun for the
    #                     current command
    #   command           the name of the command
    def addCommandToBeginStates(self, comma_list_states, command):
        state_list = comma_list_states.split(",")
        # Loop through each state individually
        for cur_state in state_list:
            found_state = self.findState(cur_state)
            # If not found, create a new one
            if None == found_state:
                # Split the state name into the "type" and the "variable"
                # after the first underscore ('_')
                split_state_name = cur_state.split('_', 1)
                state_type = split_state_name[0]
                state_variable = split_state_name[1]
                # Create a new begin state command list with this command
                # and create empty lists for the rest
                begin_list = []
                end_list = []
                check_list = []
                begin_list.append(command)
                # Create and append the new state
                self.api_states.append(
                    self.ApiState(state=cur_state,
                                  type=state_type,
                                  variable=state_variable,
                                  begin_commands=begin_list,
                                  end_commands=end_list,
                                  check_commands=check_list))
            else:
                # Found, so just add a new begin command
                found_state.begin_commands.append(command)

    # Add this command to any end state checks (used for validation)
    #   self              the AutomaticSourceOutputGenerator object
    #   comma_list_states a comma-delimited string containing the list
    #                     of states that need to end for the
    #                     current command
    #   command           the name of the command
    def addCommandToEndStates(self, comma_list_states, command):
        state_list = comma_list_states.split(",")
        # Loop through each state individually
        for cur_state in state_list:
            found_state = self.findState(cur_state)
            # If not found, create a new one
            if None == found_state:
                # Split the state name into the "type" and the "variable"
                # after the first underscore ('_')
                split_state_name = cur_state.split('_', 1)
                state_type = split_state_name[0]
                state_variable = split_state_name[1]
                # Create a new end state command list with this command
                # and create empty lists for the rest
                begin_list = []
                end_list = []
                check_list = []
                end_list.append(command)
                # Create and append the new state
                self.api_states.append(
                    self.ApiState(state=cur_state,
                                  type=state_type,
                                  variable=state_variable,
                                  begin_commands=begin_list,
                                  end_commands=end_list,
                                  check_commands=check_list))
            else:
                # Found, so just add a new end command
                found_state.end_commands.append(command)

    # Add this command to any state checks (used for validation)
    #   self              the AutomaticSourceOutputGenerator object
    #   comma_list_states a comma-delimited string containing the list
    #                     of states that need to be checked for the
    #                     current command
    #   command           the name of the command
    def addCommandToCheckStates(self, comma_list_states, command):
        state_list = comma_list_states.split(",")
        # Loop through each state individually
        for cur_state in state_list:
            found_state = self.findState(cur_state)
            # If not found, create a new one
            if None == found_state:
                # Split the state name into the "type" and the "variable"
                # after the first underscore ('_')
                split_state_name = cur_state.split('_', 1)
                state_type = split_state_name[0]
                state_variable = split_state_name[1]
                # Create a new check state command list with this command
                # and create empty lists for the rest
                begin_list = []
                end_list = []
                check_list = []
                check_list.append(command)
                # Create and append the new state
                self.api_states.append(
                    self.ApiState(state=cur_state,
                                  type=state_type,
                                  variable=state_variable,
                                  begin_commands=begin_list,
                                  end_commands=end_list,
                                  check_commands=check_list))
            else:
                # Found, so just add a new check command
                found_state.check_commands.append(command)

    # Check if the parameter passed in is a pointer
    #   self            the AutomaticSourceOutputGenerator object
    #   param           the XML information for the param
    def paramIsPointer(self, param):
        ispointer = False
        paramtype = param.find('type')
        if paramtype.tail is not None and '*' in paramtype.tail:
            ispointer = True
        return ispointer

    # Check if the parameter passed in is a pointer to an array
    #   self            the AutomaticSourceOutputGenerator object
    #   param           the XML information for the param
    def paramIsArray(self, param):
        return param.attrib.get('len') is not None

    # Check if the parameter passed in is a static array
    #   self            the AutomaticSourceOutputGenerator object
    #   param           the XML information for the param
    def paramIsStaticArray(self, param):
        is_static_array = 0
        param_name = param.find('name')
        if param_name.tail and ('[' in param_name.tail):
            is_static_array = param_name.tail.count('[')
        return is_static_array

    # Determine the array dimension and the size of each.  Only support
    # up to two dimensions for now
    #   self            the AutomaticSourceOutputGenerator object
    #   param           the XML information for the param
    def paramStaticArraySizes(self, param):
        static_array_sizes = []
        param_name = param.find('name')
        param_enum = param.find('enum')
        if param_name.tail and ('[' in param_name.tail):
            static_array_dimen = param_name.tail.count('[')
            if static_array_dimen > 0:
                if static_array_dimen == 1 and param_enum is not None:
                    static_array_sizes.append(param_enum.text)
                else:
                    tail_str = param_name.tail
                    while (tail_str.count('[') > 0):
                        tail_str = tail_str.replace('[', '', 1)
                        tail_str = tail_str.replace(']', '', 1)
                        if len(tail_str) > 0:
                            cur_size_str = ''
                            if tail_str.count('[') > 0:
                                cur_size_str = tail_str[0:tail_str.find('[')]
                            else:
                                cur_size_str = tail_str
                            static_array_sizes.append(cur_size_str)
        return static_array_sizes

    # Check if the parameter passed in is optional
    # Returns a list of Boolean values for comma separated len attributes (len='false,true')
    #   self            the AutomaticSourceOutputGenerator object
    #   param           the XML information for the param
    def paramIsOptional(self, param):
        # See if the handle is optional
        is_optional = False
        # Simple, if it's optional, return true
        optional_string = param.attrib.get('optional')
        if optional_string:
            if optional_string == 'true':
                is_optional = True
            elif ',' in optional_string:
                opts = []
                for opt in optional_string.split(','):
                    val = opt.strip()
                    if val == 'true':
                        opts.append(True)
                    elif val == 'false':
                        opts.append(False)
                    else:
                        print('Unrecognized len attribute value', val)
                is_optional = opts
        return is_optional

    # Check if the parameter passed in is a pointer
    #   self            the AutomaticSourceOutputGenerator object
    #   param_cdecl     the entire cdecl for the parameter
    #   param_type      the API type of the parameter
    #   param_name      the name of the parameter
    def paramPointerCount(self, param_cdecl, param_type, param_name):
        pointer_count = 0
        type_start = param_cdecl.find(param_type)
        name_start = param_cdecl.find(param_name)
        past_type_string = param_cdecl[type_start + len(param_type):name_start]
        pointer_count = past_type_string.count('*')
        if param_type[:4] == 'PFN_':
            pointer_count = pointer_count + 1
        return pointer_count

    # Check if the parameter passed in is an array
    #   self            the AutomaticSourceOutputGenerator object
    #   param_cdecl     the entire cdecl for the parameter
    #   param_type      the API type of the parameter
    #   param_name      the name of the parameter
    def paramArrayDimension(self, param_cdecl, param_type, param_name):
        array_dimen = 0
        type_start = param_cdecl.find(param_type)
        name_start = param_cdecl.find(param_name)
        past_type_string = param_cdecl[type_start + len(param_type):name_start]
        array_dimen = past_type_string.count('[')
        return array_dimen

    # Get the parent of a handle object
    #   self            the AutomaticSourceOutputGenerator object
    #   handle_name     the name of the handle to return the parent of
    def getHandleParent(self, handle_name):
        types = self.registry.tree.findall("types/type")
        for elem in types:
            if ((elem.find("name") and elem.find('name').text == handle_name) or
                    elem.attrib.get('name') == handle_name):
                return elem.attrib.get('parent')
        return None

    # Get the ancestors of a handle object
    #   self            the AutomaticSourceOutputGenerator object
    #   handle_name     the name of the handle to return the parent of
    def getHandleAncestors(self, handle_name):
        ancestors = []
        current = handle_name
        while True:
            current = self.getHandleParent(current)
            if current is None:
                return ancestors
            ancestors.append(current)

    # Determine if the provided name is really part of the API core
    #   self            the AutomaticSourceOutputGenerator object
    #   ext_name        the name of the "extension" to determine if it really is core
    def isCoreExtensionName(self, ext_name):
        if "XR_VERSION_" in ext_name:
            return True
        return False

    # Determine if all the characters in a string are upper-case
    #   self            the AutomaticSourceOutputGenerator object
    #   check_str       string to check for all uppercase letters
    def isAllUpperCase(self, check_str):
        for c in check_str:
            if c.isalpha() and c.islower():
                return False
        return True

    # Determine if all the characters in a string are numbers
    #   self            the AutomaticSourceOutputGenerator object
    #   check_str       string to check for all numbers
    def isAllNumbers(self, check_str):
        for c in check_str:
            if not c.isdigit():
                return False
        return True

    # Is this an enum type?
    #   self            the AutomaticSourceOutputGenerator object
    #   type_name       the name of the type to check
    def isEnumType(self, type_name):
        for enum_tuple in self.api_enums:
            if type_name == enum_tuple.name:
                return True
        return False

    # Is this type a flag?
    #   self            the AutomaticSourceOutputGenerator object
    #   type_name       the name of the type to check
    def isFlagType(self, type_name):
        for flag_tuple in self.api_flags:
            if type_name == flag_tuple.name:
                return True
        return False

    # This flag is defined to a particular set of bitfields, so can be non-zero.
    #   self            the AutomaticSourceOutputGenerator object
    #   type_name       the name of the type to check
    def flagHasValidValues(self, type_name):
        for flag_tuple in self.api_flags:
            if type_name == flag_tuple.name:
                if flag_tuple.valid_flags:
                    return True
                break
        return False

    # Is this an OpenXR handle?
    #   self            the AutomaticSourceOutputGenerator object
    #   type_name       the name of the type to check
    def isHandle(self, type_name):
        for handle_tuple in self.api_handles:
            if type_name == handle_tuple.name:
                return True
        return False

    # Return the handle if it is one, None otherwise
    #   self            the AutomaticSourceOutputGenerator object
    #   type_name       the name of the type to check
    def getHandle(self, type_name):
        for handle_tuple in self.api_handles:
            if type_name == handle_tuple.name:
                return handle_tuple
        return None

    # Gets the name of a handle parameter (dereferenced if a pointer,
    # the first element thereof if an array),
    # or none if not a handle.
    #   self            the ValidationSourceOutputGenerator object
    #   param           a MemberParam object
    def getFirstHandleName(self, param):
        if not self.isHandle(param.type):
            return None
        name = param.name
        assert(param.pointer_count <= 1)
        if param.pointer_count == 1:
            if param.pointer_count_var is None:
                # Just a pointer
                name = "(*{})".format(name)
            else:
                name += '[0]'
        return name

    # Get the type without any pointer/const modifiers
    #   self            the AutomaticSourceOutputGenerator object
    #   type_cdecl      the type cdecl to extract the basic type from
    def getRawType(self, type_cdecl):
        basic_type = type_cdecl
        # For now, get rid of any constant qualifiers or extra spaces or stars
        # so we can just get the parameter name.
        basic_type.replace("const", "")
        basic_type.replace("*", "")
        basic_type.replace("&", "")
        basic_type.replace(" ", "")

        # See if the type returned is actually another base type and replace it
        # with what the base type is.
        base_type = self.getBaseType(basic_type)
        if base_type:
            basic_type = self.getRawType(base_type.type)
        return basic_type

    # Utility function to determine if a type is a structure
    #   self        the AutomaticSourceOutputGenerator object
    #   type_name   the name of the type to check
    def isStruct(self, type_name):
        is_struct = False
        for cur_struct in self.api_structures:
            if cur_struct.name == type_name:
                is_struct = True
                break
        return is_struct

    # Utility function to determine if a type is a structure with handles
    #   self            the AutomaticSourceOutputGenerator object
    #   type_name       the name of the type to check
    def isStructWithHandles(self, type_name):
        struct = None
        for cur_struct in self.api_structures:
            if cur_struct.name == type_name:
                for member in cur_struct.members:
                    if member.is_handle:
                        struct = cur_struct
                        break
                break
        return struct

    # Utility function to determine if a type is a structure
    #   self            the AutomaticSourceOutputGenerator object
    #   type_name       the name of the type to get the structure for
    def getStruct(self, type_name):
        ret_struct = None
        for cur_struct in self.api_structures:
            if cur_struct.name == type_name:
                ret_struct = cur_struct
                break
        return ret_struct

    # Try to do check if a structure is always considered valid (i.e. there's no rules to its acceptance)
    #   self            the AutomaticSourceOutputGenerator object
    #   struct_name     the name of the structure to evaluate
    def isStructAlwaysValid(self, struct_name):
        struct = self.registry.tree.find(
            "types/type[@name='" + struct_name + "']")
        params = struct.findall('member')
        for param in params:
            paramname = param.find('name')
            paramtype = param.find('type')
            typecategory = self.getTypeCategory(paramtype.text)
            if paramname.text == 'next':
                return False
            if paramname.text == 'type':
                return False
            if paramtype.text == 'void' or paramtype.text == 'char' or self.paramIsArray(param) or self.paramIsPointer(param):
                return False
            elif typecategory == 'handle' or typecategory == 'enum' or typecategory == 'bitmask':
                return False
            elif typecategory == 'struct' or typecategory == 'union':
                if self.isStructAlwaysValid(paramtype.text) is False:
                    return False
        return True

    # Utility function to determine if a type is a union
    #   self            the AutomaticSourceOutputGenerator object
    #   type_name       the name of the type to check
    def isUnion(self, type_name):
        is_union = False
        for cur_union in self.api_unions:
            if cur_union.name == type_name:
                is_union = True
                break
        return is_union

    # Utility function to determine if a type is a union
    #   self            the AutomaticSourceOutputGenerator object
    #   type_name       the name of the type to retrieve the union for
    def getUnion(self, type_name):
        ret_union = None
        for cur_union in self.api_unions:
            if cur_union.name == type_name:
                ret_union = cur_union
                break
        return ret_union

    # Utility function to determine if a type is a basetype
    def getBaseType(self, name):
        ret_base = None
        for base_type in self.api_base_types:
            if base_type.name == name:
                ret_base = base_type
                break
        return ret_base

    # Generate a XrObjectType based on a handle name
    #   self            the AutomaticSourceOutputGenerator object
    #   handle_name     the name of the type to convert to the XrObjectType enum
    def genXrObjectType(self, handle_name):
        # Find any place where a lowercase letter is followed by an uppercase
        # letter, and insert an underscore between them
        value = re.sub('([a-z0-9])([A-Z])', r'\1_\2', handle_name)
        # Change the whole string to uppercase
        value = value.upper()
        # Add "OBJECT_TYPE_" after the XR_ prefix
        object_type_name = re.sub('XR_', 'XR_OBJECT_TYPE_', value, 1)
        invalid_type = True
        for object_type in self.api_object_types:
            if object_type.name == object_type_name:
                invalid_type = False
                break
        if invalid_type:
            self.printCodeGenErrorMessage('Generated XrObjectType %s for handle %s does not exist!' % (
                object_type_name, handle_name))
        return object_type_name

    # Generate a handle name based on a XrObjectType
    #   self            the AutomaticSourceOutputGenerator object
    #   object_type     the XrObjectType enum to convert to a handle name
    def genXrHandleName(self, object_type):
        if 'XR_OBJECT_TYPE_' not in object_type:
            return ''
        cur_string = object_type[15:].lower()
        while '_' in cur_string:
            value_index = cur_string.index('_')
            new_string = ''
            if value_index > 0:
                new_string = cur_string[:value_index]
            new_char = cur_string[value_index + 1].upper()
            new_string += new_char
            new_string += cur_string[value_index + 2:]
            cur_string = new_string
        handle_name = 'Xr'
        handle_name += cur_string
        # Now loop through and find real string.  Case of extension
        # on end may be different, so we'll use the real structure
        # name
        for handle in self.api_handles:
            if handle.name.lower() == handle_name.lower():
                return handle.name
        self.printCodeGenErrorMessage('Generated handle %s for XrObjectType %s does not exist!' % (
            handle_name, object_type))
        return ''

    # Generate a XrStructureType based on a structure typename
    #   self            the AutomaticSourceOutputGenerator object
    #   type_name       the name of the type to convert to the XrStructureType enum
    def genXrStructureType(self, type_name):
        value = type_name.replace('D3D', 'D3d')
        value = value.replace('OpenGL', 'Opengl')
        value = value.replace('OpenglES', 'OpenglEs')
        value = value.replace('iOS', 'Ios')
        value = value.replace('RGB', 'Rgb')
        # Find any place where a lowercase letter is followed by an uppercase
        # letter, and insert an underscore between them
        value = re.sub('([a-z0-9])([A-Z])', r'\1_\2', value)
        # Change the whole string to uppercase
        value = value.upper()
        # Add "TYPE_" after the XR_ prefix
        structure_type_name = re.sub('XR_', 'XR_TYPE_', value, 1)
        # If this structure is part of an extension, and the suffix doesn't have an underscore
        # in front of it at this point, add one.
        for cur_vendor_tag in self.vendor_tags:
            if structure_type_name.endswith(cur_vendor_tag):
                vendor_tag_len = len(cur_vendor_tag)
                if structure_type_name[-(vendor_tag_len + 1)].isalpha():
                    prefix = structure_type_name[:-vendor_tag_len]
                    suffix = structure_type_name[-vendor_tag_len:]
                    structure_type_name = prefix + '_' + suffix
        invalid_type = True
        if not (type_name in self.structs_with_no_type):
            for structure_type in self.api_structure_types:
                if structure_type.name == structure_type_name:
                    invalid_type = False
                    break
            if invalid_type:
                self.printCodeGenErrorMessage('Generated XrStructureType %s for structure %s does not exist!' % (
                    structure_type_name, type_name))
        else:
            structure_type_name = ''
        return structure_type_name

    # Generate a structure typename based on a XrStructureType
    #   self            the AutomaticSourceOutputGenerator object
    #   type_name       the XrStructureType enum to convert to a structure name
    def genXrStructureName(self, type_name):
        if 'XR_TYPE' not in type_name:
            return ''
        if type_name == 'XR_TYPE_UNKNOWN':
            return ''
        cur_string = type_name[7:].lower()
        while '_' in cur_string:
            value_index = cur_string.index('_')
            new_string = ''
            if value_index > 0:
                new_string = cur_string[:value_index]
            new_char = cur_string[value_index + 1].upper()
            new_string += new_char
            new_string += cur_string[value_index + 2:]
            cur_string = new_string
        struct_name = 'Xr'
        struct_name += cur_string
        # Now loop through and find real string.  Case of extension
        # on end may be different, so we'll use the real structure
        # name
        for cur_struct in self.api_structures:
            if cur_struct.name.lower() == struct_name.lower():
                return cur_struct.name
        self.printCodeGenErrorMessage('Generated structure name %s for XrStructureType %s does not exist!' % (
            struct_name, type_name))
        return ''

    # Write the indent for the appropriate number of tab spaces
    #   self            the AutomaticSourceOutputGenerator object
    #   indent_cnt      the number of indents to return a string of
    def writeIndent(self, indent_cnt):
        return '    ' * indent_cnt
