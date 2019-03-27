#!/usr/bin/python3 -i
#
# Copyright (c) 2017 The Khronos Group Inc.
# Copyright (c) 2017 Valve Corporation
# Copyright (c) 2017 LunarG, Inc.
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
# Author: Mark Young <marky@lunarg.com>

import os
import re
import sys
from automatic_source_generator import *
from collections import namedtuple

# The following commands should not be generated for the layer
VALID_USAGE_DONT_GEN = [
    'xrEnumerateApiLayerProperties',
    'xrEnumerateInstanceExtensionProperties',
]

# The following commands have a manually defined component to them.
VALID_USAGE_MANUALLY_DEFINED = [
    'xrCreateInstance',
    'xrDestroyInstance',
    'xrCreateSession',
    # We manually implement some of the XR_EXT_debug_utils entry-points so that we
    # can return validation messages to known debug utils messengers
    'xrSetDebugUtilsObjectNameEXT',
    'xrCreateDebugUtilsMessengerEXT',
    'xrDestroyDebugUtilsMessengerEXT',
    'xrSessionBeginDebugUtilsLabelRegionEXT',
    'xrSessionEndDebugUtilsLabelRegionEXT',
    'xrSessionInsertDebugUtilsLabelEXT',
]

# ValidationSourceGeneratorOptions - subclass of AutomaticSourceGeneratorOptions.


class ValidationSourceGeneratorOptions(AutomaticSourceGeneratorOptions):
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
        AutomaticSourceGeneratorOptions.__init__(self, filename, directory, apiname, profile,
                                                 versions, emitversions, defaultExtensions,
                                                 addExtensions, removeExtensions,
                                                 emitExtensions, sortProcedure)

# ValidationSourceOutputGenerator - subclass of AutomaticSourceOutputGenerator.


class ValidationSourceOutputGenerator(AutomaticSourceOutputGenerator):
    """Generate core validation layer source using XML element attributes from registry"""

    def __init__(self,
                 errFile=sys.stderr,
                 warnFile=sys.stderr,
                 diagFile=sys.stdout):
        AutomaticSourceOutputGenerator.__init__(
            self, errFile, warnFile, diagFile)

    # Override the base class header warning so the comment indicates this file.
    #   self            the ValidationSourceOutputGenerator object
    def OutputGeneratedHeaderWarning(self):
        generated_warning = '// *********** THIS FILE IS GENERATED - DO NOT EDIT ***********\n'
        generated_warning += '//     See validation_layer_generator.py for modifications\n'
        generated_warning += '// ************************************************************\n'
        write(generated_warning, file=self.outFile)

    # Call the base class to properly begin the file, and then add
    # the file-specific header information.
    #   self            the ValidationSourceOutputGenerator object
    #   gen_opts        the ValidationSourceGeneratorOptions object
    def beginFile(self, genOpts):
        AutomaticSourceOutputGenerator.beginFile(self, genOpts)
        preamble = ''
        if self.genOpts.filename == 'xr_generated_core_validation.hpp':
            preamble += '#pragma once\n'
            preamble += '#include <vector>\n'
            preamble += '#include <string>\n'
            preamble += '#include <unordered_map>\n'
            preamble += '#include <thread>\n'
            preamble += '#include <mutex>\n\n'
            preamble += '#include "api_layer_platform_defines.h"\n'
            preamble += '#include <openxr/openxr.h>\n'
            preamble += '#include <openxr/openxr_platform.h>\n\n'
            preamble += '#include "xr_generated_dispatch_table.h"\n'
        elif self.genOpts.filename == 'xr_generated_core_validation.cpp':
            preamble += '#include <sstream>\n'
            preamble += '#include <cstring>\n'
            preamble += '#include <algorithm>\n\n'
            preamble += '#include "xr_generated_core_validation.hpp"\n'
        write(preamble, file=self.outFile)

    # Write out all the information for the appropriate file,
    # and then call down to the base class to wrap everything up.
    #   self            the ValidationSourceOutputGenerator object
    def endFile(self):
        file_data = ''
        if self.genOpts.filename == 'xr_generated_core_validation.hpp':
            file_data += self.outputValidationHeaderInfo()
        elif self.genOpts.filename == 'xr_generated_core_validation.cpp':
            file_data += self.outputCommonTypesForValidation()
            file_data += self.outputValidationSourceFuncs()
        write(file_data, file=self.outFile)

        # Finish processing in superclass
        AutomaticSourceOutputGenerator.endFile(self)

    # Write out common internal types for validation
    #   self            the ValidationSourceOutputGenerator object
    def outputCommonTypesForValidation(self):
        common_validation_types = ''
        common_validation_types += '// Structure used for indicating status of \'flags\' test.\n'
        common_validation_types += 'enum ValidateXrFlagsResult {\n'
        common_validation_types += '    VALIDATE_XR_FLAGS_ZERO,\n'
        common_validation_types += '    VALIDATE_XR_FLAGS_INVALID,\n'
        common_validation_types += '    VALIDATE_XR_FLAGS_SUCCESS,\n'
        common_validation_types += '};\n\n'
        common_validation_types += '// Enum used for indicating handle validation status.\n'
        common_validation_types += 'enum ValidateXrHandleResult {\n'
        common_validation_types += '    VALIDATE_XR_HANDLE_NULL,\n'
        common_validation_types += '    VALIDATE_XR_HANDLE_INVALID,\n'
        common_validation_types += '    VALIDATE_XR_HANDLE_SUCCESS,\n'
        common_validation_types += '};\n\n'
        return common_validation_types

    # Generate C++ structures and maps used for validating the states identified
    # in the specification.
    #   self            the ValidationSourceOutputGenerator object
    def outputValidationStateCheckStructs(self):
        validation_state_checks = '// Structure used for state validation.\n'
        active_structures = dict()
        for cur_state in self.api_states:
            type_name = '%s' % cur_state.type
            cur_list = []
            if None != active_structures.get(type_name):
                cur_list = active_structures.get(type_name)
            cur_list.append(cur_state.variable)
            active_structures[type_name] = cur_list
        for type_name, variable_list in active_structures.items():
            validation_state_checks += 'struct %sValidationStates {\n' % type_name
            for variable in variable_list:
                validation_state_checks += '    bool %s;\n' % variable
            validation_state_checks += '};\n'
            validation_state_checks += 'std::unordered_map<%s, %sValidationStates*> g_%s_valid_states;\n' % (
                type_name, type_name, type_name[2:].lower())
        validation_state_checks += '\n'
        return validation_state_checks

    # Generate C++ structure and utility function prototypes for validating
    # the 'next' chains in structures.
    #   self            the ValidationSourceOutputGenerator object
    def outputValidationSourceNextChainProtos(self):
        next_chain_info = ''
        next_chain_info += '// Result return value for next chain validation\n'
        next_chain_info += 'enum NextChainResult {\n'
        next_chain_info += '    NEXT_CHAIN_RESULT_VALID = 0,\n'
        next_chain_info += '    NEXT_CHAIN_RESULT_ERROR = -1,\n'
        next_chain_info += '    NEXT_CHAIN_RESULT_DUPLICATE_STRUCT = -2,\n'
        next_chain_info += '};\n\n'
        next_chain_info += '// Prototype for validateNextChain command (it uses the validate structure commands so add it after\n'
        next_chain_info += 'NextChainResult ValidateNextChain(GenValidUsageXrInstanceInfo *instance_info,\n'
        next_chain_info += '                                  const std::string &command_name,\n'
        next_chain_info += '                                  std::vector<GenValidUsageXrObjectInfo>& objects_info,\n'
        next_chain_info += '                                  const void* next,\n'
        next_chain_info += '                                  std::vector<XrStructureType>& valid_ext_structs,\n'
        next_chain_info += '                                  std::vector<XrStructureType>& encountered_structs,\n'
        next_chain_info += '                                  std::vector<XrStructureType>& duplicate_structs);\n\n'
        return next_chain_info

    # Generate C++ enum and utility function prototypes for validating
    # the flags in structures.
    #   self            the ValidationSourceOutputGenerator object
    def outputValidationSourceFlagBitValues(self):
        flag_value_validate = ''
        for flag_tuple in self.api_flags:
            if flag_tuple.protect_value:
                flag_value_validate += '#if %s\n' % flag_tuple.protect_string
            flag_value_validate += '// Function to validate %s flags\n' % flag_tuple.name
            flag_value_validate += 'ValidateXrFlagsResult ValidateXr%s(const %s value) {\n' % (
                flag_tuple.name[2:], flag_tuple.type)
            # We need to return a value indicating that the value is zero because in some
            # circumstances, 0 is ok.  However, in other cases, 0 is disallowed.  So, leave
            # it up to the calling function to decide what is correct.
            flag_value_validate += '    if (0 == value) {\n'
            flag_value_validate += '        return VALIDATE_XR_FLAGS_ZERO;\n'
            flag_value_validate += '    }\n'
            # If the flag has no values defined for this flag, then anything other than
            # zero generates an error.
            if flag_tuple.valid_flags == None:
                flag_value_validate += '    return VALIDATE_XR_FLAGS_INVALID;\n'
            else:
                # This flag has values set.  So, check (and remove) each valid value.  Once that's done
                # anything left over would be invalid.
                flag_value_validate += '    %s int_value = value;\n' % flag_tuple.type
                for mask_tuple in self.api_bitmasks:
                    if mask_tuple.name == flag_tuple.valid_flags:
                        for cur_value in mask_tuple.values:
                            if cur_value.protect_value and flag_tuple.protect_value != cur_value.protect_value:
                                flag_value_validate += '#if %s\n' % cur_value.protect_string
                            flag_value_validate += '    if ((int_value & %s) != 0) {\n' % cur_value.name
                            flag_value_validate += '        // Clear the value %s since it is valid\n' % cur_value.name
                            flag_value_validate += '        int_value &= ~%s;\n' % cur_value.name
                            flag_value_validate += '    }\n'
                            if cur_value.protect_value and flag_tuple.protect_value != cur_value.protect_value:
                                flag_value_validate += '#endif // %s\n' % cur_value.protect_string
                        break
                flag_value_validate += '    if (int_value != 0) {\n'
                flag_value_validate += '        // Something is left, it must be invalid\n'
                flag_value_validate += '        return VALIDATE_XR_FLAGS_INVALID;\n'
                flag_value_validate += '    }\n'
                flag_value_validate += '    return VALIDATE_XR_FLAGS_SUCCESS;\n'
            flag_value_validate += '}\n\n'
            if flag_tuple.protect_value:
                flag_value_validate += '#endif // %s\n' % flag_tuple.protect_string
        return flag_value_validate

    # Generate C++ functions for validating enums.
    #   self            the ValidationSourceOutputGenerator object
    def outputValidationSourceEnumValues(self):
        enum_value_validate = ''
        for enum_tuple in self.api_enums:
            if enum_tuple.protect_value:
                enum_value_validate += '#if %s\n' % enum_tuple.protect_string
            enum_value_validate += '// Function to validate %s enum\n' % enum_tuple.name
            enum_value_validate += 'bool ValidateXrEnum(GenValidUsageXrInstanceInfo *instance_info,\n'
            enum_value_validate += '                    const std::string &command_name,\n'
            enum_value_validate += '                    const std::string &validation_name,\n'
            enum_value_validate += '                    const std::string &item_name,\n'
            enum_value_validate += '                    std::vector<GenValidUsageXrObjectInfo>& objects_info,\n'
            enum_value_validate += '                    const %s value) {\n' % enum_tuple.name
            indent = 1
            checked_extension = ''
            if enum_tuple.ext_name and not self.isCoreExtensionName(enum_tuple.ext_name):
                checked_extension = enum_tuple.ext_name
                enum_value_validate += self.writeIndent(indent)
                enum_value_validate += '// Enum requires extension %s, so check that it is enabled\n' % enum_tuple.ext_name
                enum_value_validate += self.writeIndent(indent)
                enum_value_validate += 'if (nullptr != instance_info && !ExtensionEnabled(instance_info->enabled_extensions, "%s")) {\n' % enum_tuple.ext_name
                indent += 1
                enum_value_validate += self.writeIndent(indent)
                enum_value_validate += 'std::string vuid = "VUID-";\n'
                enum_value_validate += self.writeIndent(indent)
                enum_value_validate += 'vuid += validation_name;\n'
                enum_value_validate += self.writeIndent(indent)
                enum_value_validate += 'vuid += "-";\n'
                enum_value_validate += self.writeIndent(indent)
                enum_value_validate += 'vuid += item_name;\n'
                enum_value_validate += self.writeIndent(indent)
                enum_value_validate += 'vuid += "-parameter";\n'
                enum_value_validate += self.writeIndent(indent)
                enum_value_validate += 'std::string error_str = "%s requires extension ";\n' % enum_tuple.name
                enum_value_validate += self.writeIndent(indent)
                enum_value_validate += 'error_str += " \\"%s\\" to be enabled, but it is not enabled";\n' % enum_tuple.ext_name
                enum_value_validate += self.writeIndent(indent)
                enum_value_validate += 'CoreValidLogMessage(instance_info, vuid,\n'
                enum_value_validate += self.writeIndent(indent)
                enum_value_validate += '                    VALID_USAGE_DEBUG_SEVERITY_ERROR, command_name,\n'
                enum_value_validate += self.writeIndent(indent)
                enum_value_validate += '                    objects_info, error_str);\n'
                enum_value_validate += self.writeIndent(indent)
                enum_value_validate += 'return false;\n'
                indent -= 1
                enum_value_validate += self.writeIndent(indent)
                enum_value_validate += '}\n'
            enum_value_validate += self.writeIndent(indent)
            enum_value_validate += 'switch (value) {\n'
            indent += 1
            for cur_value in enum_tuple.values:
                if cur_value.protect_value and enum_tuple.protect_value != cur_value.protect_value:
                    enum_value_validate += '#if %s\n' % cur_value.protect_string
                enum_value_validate += self.writeIndent(indent)
                enum_value_validate += 'case %s:\n' % cur_value.name
                if cur_value.protect_value and enum_tuple.protect_value != cur_value.protect_value:
                    enum_value_validate += '#endif // %s\n' % cur_value.protect_string
                if cur_value.ext_name and cur_value.ext_name != checked_extension and not self.isCoreExtensionName(cur_value.ext_name):
                    indent += 1
                    enum_value_validate += self.writeIndent(indent)
                    enum_value_validate += '// Enum value %s requires extension %s, so check that it is enabled\n' % (
                        cur_value.name, cur_value.ext_name)
                    enum_value_validate += self.writeIndent(indent)
                    enum_value_validate += 'if (nullptr != instance_info && !ExtensionEnabled(instance_info->enabled_extensions, "%s")) {\n' % cur_value.ext_name
                    indent += 1
                    enum_value_validate += self.writeIndent(indent)
                    enum_value_validate += 'std::string vuid = "VUID-";\n'
                    enum_value_validate += self.writeIndent(indent)
                    enum_value_validate += 'vuid += validation_name;\n'
                    enum_value_validate += self.writeIndent(indent)
                    enum_value_validate += 'vuid += "-";\n'
                    enum_value_validate += self.writeIndent(indent)
                    enum_value_validate += 'vuid += item_name;\n'
                    enum_value_validate += self.writeIndent(indent)
                    enum_value_validate += 'vuid += "-parameter";\n'
                    enum_value_validate += self.writeIndent(indent)
                    enum_value_validate += 'std::string error_str = "%s value \\"%s\\"";\n' % (
                        enum_tuple.name, cur_value.name)
                    enum_value_validate += self.writeIndent(indent)
                    enum_value_validate += 'error_str += " being used, which requires extension ";\n'
                    enum_value_validate += self.writeIndent(indent)
                    enum_value_validate += 'error_str += " \\"%s\\" to be enabled, but it is not enabled";\n' % cur_value.ext_name
                    enum_value_validate += self.writeIndent(indent)
                    enum_value_validate += 'CoreValidLogMessage(instance_info, vuid,\n'
                    enum_value_validate += self.writeIndent(indent)
                    enum_value_validate += '                    VALID_USAGE_DEBUG_SEVERITY_ERROR, command_name,\n'
                    enum_value_validate += self.writeIndent(indent)
                    enum_value_validate += '                    objects_info, error_str);\n'
                    enum_value_validate += self.writeIndent(indent)
                    enum_value_validate += 'return false;\n'
                    indent -= 1
                    enum_value_validate += self.writeIndent(indent)
                    enum_value_validate += '}\n'
                    enum_value_validate += self.writeIndent(indent)
                    enum_value_validate += 'return true;\n'
                    indent -= 1
                elif cur_value.name == 'XR_TYPE_UNKNOWN':
                    enum_value_validate += self.writeIndent(indent + 1)
                    enum_value_validate += 'return false; // Invalid XrStructureType \n'
                else:
                    enum_value_validate += self.writeIndent(indent + 1)
                    enum_value_validate += 'return true;\n'
            indent -= 1
            enum_value_validate += self.writeIndent(indent)
            enum_value_validate += 'default:\n'
            enum_value_validate += self.writeIndent(indent + 1)
            enum_value_validate += 'return false;\n'
            indent -= 1
            enum_value_validate += '}\n'
            enum_value_validate += '}\n\n'
            if enum_tuple.protect_value:
                enum_value_validate += '#endif // %s\n' % enum_tuple.protect_string
        return enum_value_validate

    # Generate prototypes for functions used internal to the source file so other functions can use them
    #   self            the ValidationSourceOutputGenerator object
    def outputValidationInternalProtos(self):
        validation_internal_protos = ''
        for handle in self.api_handles:
            if handle.protect_value:
                validation_internal_protos += '#if %s\n' % handle.protect_string
            validation_internal_protos += 'ValidateXrHandleResult Verify%sHandle(const %s* handle_to_check);\n' % (
                handle.name, handle.name)
            if handle.protect_value:
                validation_internal_protos += '#endif // %s\n' % handle.protect_string
        validation_internal_protos += '\n// Write out prototypes for handle parent verification functions\n'
        validation_internal_protos += 'bool VerifyXrParent(XrObjectType handle1_type, const XR_VALIDATION_GENERIC_HANDLE_TYPE handle1,\n'
        validation_internal_protos += '                               XrObjectType handle2_type, const XR_VALIDATION_GENERIC_HANDLE_TYPE handle2,\n'
        validation_internal_protos += '                               bool check_this);\n'
        validation_internal_protos += '\n// Function to check if an extension has been enabled\n'
        validation_internal_protos += 'bool ExtensionEnabled(std::vector<std::string> &extensions, const char* const check_extension_name);\n'
        validation_internal_protos += '\n// Functions to validate structures\n'
        for xr_struct in self.api_structures:
            if xr_struct.protect_value:
                validation_internal_protos += '#if %s\n' % xr_struct.protect_string
            validation_internal_protos += 'XrResult ValidateXrStruct(GenValidUsageXrInstanceInfo *instance_info, const std::string &command_name,\n'
            validation_internal_protos += '                          std::vector<GenValidUsageXrObjectInfo>& objects_info, bool check_members,\n'
            validation_internal_protos += '                          const %s* value);\n' % xr_struct.name
            if xr_struct.protect_value:
                validation_internal_protos += '#endif // %s\n' % xr_struct.protect_string
        return validation_internal_protos

    # Generate C++ functions for validating 'next' chains in a structure.
    #   self            the ValidationSourceOutputGenerator object
    def outputValidationSourceNextChainFunc(self):
        next_chain_info = ''
        next_chain_info += 'NextChainResult ValidateNextChain(GenValidUsageXrInstanceInfo *instance_info,\n'
        next_chain_info += '                                  const std::string &command_name,\n'
        next_chain_info += '                                  std::vector<GenValidUsageXrObjectInfo>& objects_info,\n'
        next_chain_info += '                                  const void* next,\n'
        next_chain_info += '                                  std::vector<XrStructureType>& valid_ext_structs,\n'
        next_chain_info += '                                  std::vector<XrStructureType>& encountered_structs,\n'
        next_chain_info += '                                  std::vector<XrStructureType>& duplicate_structs) {\n'
        next_chain_info += self.writeIndent(1)
        next_chain_info += 'NextChainResult return_result = NEXT_CHAIN_RESULT_VALID;\n'
        next_chain_info += self.writeIndent(1)
        next_chain_info += '// NULL is valid\n'
        next_chain_info += self.writeIndent(1)
        next_chain_info += 'if (nullptr == next) {\n'
        next_chain_info += self.writeIndent(2)
        next_chain_info += 'return return_result;\n'
        next_chain_info += self.writeIndent(1)
        next_chain_info += '}\n'
        next_chain_info += self.writeIndent(1)
        next_chain_info += '// Non-NULL is not valid if there is no valid extension structs\n'
        next_chain_info += self.writeIndent(1)
        next_chain_info += 'if (nullptr != next && 0 == valid_ext_structs.size()) {\n'
        next_chain_info += self.writeIndent(2)
        next_chain_info += 'return NEXT_CHAIN_RESULT_ERROR;\n'
        next_chain_info += self.writeIndent(1)
        next_chain_info += '}\n'
        next_chain_info += self.writeIndent(1)
        next_chain_info += 'const XrBaseInStructure* next_header = reinterpret_cast<const XrBaseInStructure*>(next);\n'
        next_chain_info += self.writeIndent(1)
        next_chain_info += 'auto valid_ext = std::find(valid_ext_structs.begin(), valid_ext_structs.end(), next_header->type);\n'
        next_chain_info += self.writeIndent(1)
        next_chain_info += 'if (valid_ext == valid_ext_structs.end()) {\n'
        next_chain_info += self.writeIndent(2)
        next_chain_info += '// Not a valid extension structure type for this next chain.\n'
        next_chain_info += self.writeIndent(2)
        next_chain_info += 'return NEXT_CHAIN_RESULT_ERROR;\n'
        next_chain_info += self.writeIndent(1)
        next_chain_info += '} else {\n'
        next_chain_info += self.writeIndent(2)
        next_chain_info += '// Check to see if we\'ve already encountered this structure.\n'
        next_chain_info += self.writeIndent(2)
        next_chain_info += 'auto already_encountered_ext = std::find(encountered_structs.begin(), encountered_structs.end(), next_header->type);\n'
        next_chain_info += self.writeIndent(2)
        next_chain_info += 'if (already_encountered_ext != encountered_structs.end()) {\n'
        next_chain_info += self.writeIndent(3)
        next_chain_info += '// Make sure we only put in unique types into our duplicate list.\n'
        next_chain_info += self.writeIndent(3)
        next_chain_info += 'auto already_duplicate = std::find(duplicate_structs.begin(), duplicate_structs.end(), next_header->type);\n'
        next_chain_info += self.writeIndent(3)
        next_chain_info += 'if (already_duplicate == duplicate_structs.end()) {\n'
        next_chain_info += self.writeIndent(4)
        next_chain_info += 'duplicate_structs.push_back(next_header->type);\n'
        next_chain_info += self.writeIndent(3)
        next_chain_info += '}\n'
        next_chain_info += self.writeIndent(3)
        next_chain_info += 'return_result = NEXT_CHAIN_RESULT_DUPLICATE_STRUCT;\n'
        next_chain_info += self.writeIndent(2)
        next_chain_info += '}\n'
        next_chain_info += self.writeIndent(1)
        next_chain_info += '}\n'
        # Validate the rest of this struct
        next_chain_info += self.writeIndent(1)
        next_chain_info += 'switch (next_header->type) {\n'
        for enum_tuple in self.api_enums:
            if enum_tuple.name == 'XrStructureType':
                if enum_tuple.protect_value:
                    next_chain_info += '#if %s\n' % enum_tuple.protect_string
                for cur_value in enum_tuple.values:
                    struct_define_name = self.genXrStructureName(
                        cur_value.name)
                    if len(struct_define_name) > 0:
                        struct_tuple = self.getStruct(struct_define_name)
                        if struct_tuple.protect_value:
                            next_chain_info += '#if %s\n' % struct_tuple.protect_string
                        next_chain_info += self.writeIndent(2)
                        next_chain_info += 'case %s:\n' % cur_value.name
                        next_chain_info += self.writeIndent(3)
                        next_chain_info += 'if (XR_SUCCESS != ValidateXrStruct(instance_info, command_name, objects_info, false,\n'
                        next_chain_info += self.writeIndent(3)
                        next_chain_info += '                                   reinterpret_cast<const %s*>(next))) {\n' % struct_define_name
                        next_chain_info += self.writeIndent(4)
                        next_chain_info += 'return NEXT_CHAIN_RESULT_ERROR;\n'
                        next_chain_info += self.writeIndent(3)
                        next_chain_info += '}\n'
                        next_chain_info += self.writeIndent(3)
                        next_chain_info += 'break;\n'
                        if struct_tuple.protect_value:
                            next_chain_info += '#endif // %s\n' % struct_tuple.protect_string
                if enum_tuple.protect_value:
                    next_chain_info += '#endif //%s\n' % enum_tuple.protect_string
                break
        next_chain_info += self.writeIndent(2)
        next_chain_info += 'default:\n'
        next_chain_info += self.writeIndent(3)
        next_chain_info += 'return NEXT_CHAIN_RESULT_ERROR;\n'
        next_chain_info += self.writeIndent(1)
        next_chain_info += '}\n'
        # Validate any chained structs
        next_chain_info += self.writeIndent(1)
        next_chain_info += 'NextChainResult next_result = ValidateNextChain(instance_info, command_name,\n'
        next_chain_info += self.writeIndent(1)
        next_chain_info += '                                                objects_info, next_header->next,\n'
        next_chain_info += self.writeIndent(1)
        next_chain_info += '                                                valid_ext_structs,\n'
        next_chain_info += self.writeIndent(1)
        next_chain_info += '                                                encountered_structs,\n'
        next_chain_info += self.writeIndent(1)
        next_chain_info += '                                                duplicate_structs);\n'
        next_chain_info += self.writeIndent(1)
        next_chain_info += 'if (NEXT_CHAIN_RESULT_VALID == next_result && NEXT_CHAIN_RESULT_VALID != return_result) {\n'
        next_chain_info += self.writeIndent(2)
        next_chain_info += 'return return_result;\n'
        next_chain_info += self.writeIndent(1)
        next_chain_info += '} else {\n'
        next_chain_info += self.writeIndent(2)
        next_chain_info += 'return next_result;\n'
        next_chain_info += self.writeIndent(1)
        next_chain_info += '}\n'
        next_chain_info += '}\n\n'
        return next_chain_info

    # Generate C++ header information containing functionality used in both
    # the generated and manual code.
    #  - Structures used to store validation information on a per-handle basis.
    #  - Unordered_map and mutexes used for storing the structure information on a per handle basis.
    #   self            the ValidationSourceOutputGenerator object
    def outputValidationHeaderInfo(self):
        commands = []
        validation_header_info = ''
        cur_extension_name = ''

        # Handle information
        validation_header_info += '// Handle information for easy conversion\n'
        validation_header_info += '#if (defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__) ) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__))\n'
        validation_header_info += '    #define XR_VALIDATION_GENERIC_HANDLE_TYPE  uint64_t\n'
        validation_header_info += '    #define CONVERT_HANDLE_TO_GENERIC(handle)  reinterpret_cast<uint64_t>(handle)\n'
        validation_header_info += '    #define CONVERT_GENERIC_TO_HANDLE(type, handle)  reinterpret_cast<type>(handle)\n'
        validation_header_info += '    #define CHECK_FOR_NULL_HANDLE(handle)  (XR_NULL_HANDLE == reinterpret_cast<void*>(handle))\n'
        validation_header_info += '#else\n'
        validation_header_info += '    #define XR_VALIDATION_GENERIC_HANDLE_TYPE  uint64_t\n'
        validation_header_info += '    #define CONVERT_HANDLE_TO_GENERIC(handle)  (handle)\n'
        validation_header_info += '    #define CONVERT_GENERIC_TO_HANDLE(type, handle)  (handle)\n'
        validation_header_info += '    #define CHECK_FOR_NULL_HANDLE(handle)  (XR_NULL_HANDLE == handle)\n'
        validation_header_info += '#endif\n'

        # First, define the instance struct used for passing information around.
        # This information includes things like the dispatch table as well as the
        # enabled extensions.
        validation_header_info += '// Structure used for storing the instance information we need for validating\n'
        validation_header_info += '// various aspects of the OpenXR API.\n\n'
        validation_header_info += '// Debug Utils items\n'
        validation_header_info += 'struct CoreValidationMessengerInfo {\n'
        validation_header_info += '    XrDebugUtilsMessengerEXT messenger;\n'
        validation_header_info += '    XrDebugUtilsMessengerCreateInfoEXT* create_info;\n'
        validation_header_info += '};\n\n'
        validation_header_info += 'struct GenValidUsageXrInstanceInfo {\n'
        validation_header_info += '    XrInstance instance;\n'
        validation_header_info += '    XrGeneratedDispatchTable* dispatch_table;\n'
        validation_header_info += '    std::vector<std::string> enabled_extensions;\n'
        validation_header_info += '    std::vector<CoreValidationMessengerInfo*> debug_messengers;\n'
        validation_header_info += '    std::vector<XrDebugUtilsObjectNameInfoEXT*> object_names;\n'
        validation_header_info += '};\n\n'
        validation_header_info += '// Structure used for storing information for other handles\n'
        validation_header_info += 'struct GenValidUsageXrHandleInfo {\n'
        validation_header_info += '    GenValidUsageXrInstanceInfo *instance_info;\n'
        validation_header_info += '    XrObjectType direct_parent_type;\n'
        validation_header_info += '    XR_VALIDATION_GENERIC_HANDLE_TYPE direct_parent_handle;\n'
        validation_header_info += '};\n\n'
        validation_header_info += '// Structure used for storing session label information\n'
        validation_header_info += 'struct GenValidUsageXrInternalSessionLabel {\n'
        validation_header_info += '    XrDebugUtilsLabelEXT debug_utils_label;\n'
        validation_header_info += '    std::string label_name;\n'
        validation_header_info += '    bool is_individual_label;\n'
        validation_header_info += '};\n\n'
        validation_header_info += '// Unordered Map associating pointer to a vector of session label information to a session\'s handle\n'
        validation_header_info += 'extern std::unordered_map<XrSession, std::vector<GenValidUsageXrInternalSessionLabel*>*> g_xr_session_labels;\n\n'
        validation_header_info += '// This function is used to delete session labels when a session is destroyed\n'
        validation_header_info += 'extern void CoreValidationDeleteSessionLabels(XrSession session);\n\n'

        for x in range(0, 2):
            if x == 0:
                commands = self.core_commands
            else:
                commands = self.ext_commands

            for cur_cmd in commands:
                if cur_cmd.ext_name != cur_extension_name:
                    if 'XR_VERSION_' in cur_cmd.ext_name:
                        validation_header_info += '\n// ---- Core %s commands\n' % cur_cmd.ext_name[11:].replace(
                            "_", ".")
                    else:
                        validation_header_info += '\n// ---- %s extension commands\n' % cur_cmd.ext_name
                    cur_extension_name = cur_cmd.ext_name

                prototype = cur_cmd.cdecl.replace("API_ATTR ", "")
                prototype = prototype.replace("XRAPI_CALL ", "")

                # We need to always export xrGetInstanceProcAddr, even though we automatically generate it.
                # Also, we really only need the core function, not the others.
                if 'xrGetInstanceProcAddr' in cur_cmd.name:
                    validation_header_info += '%s\n' % prototype.replace(
                        " xr", " GenValidUsageXr")
                    continue
                elif cur_cmd.name in VALID_USAGE_DONT_GEN or not cur_cmd.name in VALID_USAGE_MANUALLY_DEFINED:
                    continue

                if cur_cmd.protect_value:
                    validation_header_info += '#if %s\n' % cur_cmd.protect_string

                # Core call, for us to make from here into the manually implemented code
                validation_header_info += '%s\n' % prototype.replace(
                    " xr", " CoreValidationXr")
                # Validate Inputs and Next calls for the validation to make
                validation_header_info += 'XrResult %s(' % cur_cmd.name.replace(
                    "xr", "GenValidUsageInputsXr")
                count = 0
                for param in cur_cmd.params:
                    if count > 0:
                        validation_header_info += ', '
                    count = count + 1
                    validation_header_info += param.cdecl.strip()
                validation_header_info += ');\n'
                validation_header_info += '%s\n' % prototype.replace(
                    " xr", " GenValidUsageNextXr")

                if cur_cmd.protect_value:
                    validation_header_info += '#endif // %s\n' % cur_cmd.protect_string

        validation_header_info += '\n// Current API version of the Core Validation API Layer\n#define XR_CORE_VALIDATION_API_VERSION '
        validation_header_info += self.api_version_define
        validation_header_info += '\n'

        validation_header_info += '\n// Externs for Core Validation\n'
        for handle in self.api_handles:
            base_handle_name = handle.name[2:].lower()
            if handle.protect_value:
                validation_header_info += '#if %s\n' % handle.protect_string
            if handle.name == 'XrInstance':
                validation_header_info += 'extern std::unordered_map<%s, GenValidUsageXrInstanceInfo*> g_%s_info_map;\n' % (
                    handle.name, base_handle_name)
            else:
                validation_header_info += 'extern std::unordered_map<%s, GenValidUsageXrHandleInfo*> g_%s_info_map;\n' % (
                    handle.name, base_handle_name)
            validation_header_info += 'extern std::mutex g_%s_dispatch_mutex;\n' % base_handle_name
            if handle.protect_value:
                validation_header_info += '#endif // %s\n' % handle.protect_string
        validation_header_info += 'void GenValidUsageCleanUpMaps(GenValidUsageXrInstanceInfo *instance_info);\n\n'

        validation_header_info += '// Object information used for logging.\n'
        validation_header_info += 'struct GenValidUsageXrObjectInfo {\n'
        validation_header_info += '    XR_VALIDATION_GENERIC_HANDLE_TYPE handle;\n'
        validation_header_info += '    XrObjectType type;\n'
        validation_header_info += '};\n'
        validation_header_info += '// Debug message severity levels for logging.\n'
        validation_header_info += 'enum GenValidUsageDebugSeverity {\n'
        validation_header_info += '    VALID_USAGE_DEBUG_SEVERITY_DEBUG = 0,\n'
        validation_header_info += '    VALID_USAGE_DEBUG_SEVERITY_INFO = 7,\n'
        validation_header_info += '    VALID_USAGE_DEBUG_SEVERITY_WARNING = 14,\n'
        validation_header_info += '    VALID_USAGE_DEBUG_SEVERITY_ERROR = 21,\n'
        validation_header_info += '};\n\n'
        validation_header_info += '\n// Function to convert XrObjectType to string\n'
        validation_header_info += 'std::string GenValidUsageXrObjectTypeToString(const XrObjectType& type);\n\n'
        validation_header_info += '// Function to record all the core validation information\n'
        validation_header_info += 'extern void CoreValidLogMessage(GenValidUsageXrInstanceInfo *instance_info, const std::string &message_id,\n'
        validation_header_info += '                                GenValidUsageDebugSeverity message_severity, const std::string &command_name,\n'
        validation_header_info += '                                std::vector<GenValidUsageXrObjectInfo> objects_info, const std::string &message);\n'
        return validation_header_info

    # Generate C++ utility functions to verify that all the required extensions have been enabled.
    #   self            the ValidationSourceOutputGenerator object
    def writeVerifyExtensions(self):
        verify_extensions = 'bool ExtensionEnabled(std::vector<std::string> &extensions, const char* const check_extension_name) {\n'
        verify_extensions += self.writeIndent(1)
        verify_extensions += 'for (auto enabled_extension: extensions) {\n'
        verify_extensions += self.writeIndent(2)
        verify_extensions += 'if (enabled_extension == check_extension_name) {\n'
        verify_extensions += self.writeIndent(3)
        verify_extensions += 'return true;\n'
        verify_extensions += self.writeIndent(2)
        verify_extensions += '}\n'
        verify_extensions += self.writeIndent(1)
        verify_extensions += '}\n'
        verify_extensions += self.writeIndent(1)
        verify_extensions += 'return false;\n'
        verify_extensions += '}\n\n'
        number_of_instance_extensions = 0
        number_of_system_extensions = 0
        for extension in self.extensions:
            if extension.type == 'instance':
                number_of_instance_extensions += 1
            elif extension.type == 'system':
                number_of_system_extensions += 1
        verify_extensions += 'bool ValidateInstanceExtensionDependencies(GenValidUsageXrInstanceInfo *gen_instance_info,\n'
        verify_extensions += '                                           const std::string &command,\n'
        verify_extensions += '                                           const std::string &struct_name,\n'
        verify_extensions += '                                           std::vector<GenValidUsageXrObjectInfo>& objects_info,\n'
        verify_extensions += '                                           std::vector<std::string> &extensions) {\n'
        indent = 1
        if number_of_instance_extensions > 0:
            verify_extensions += self.writeIndent(indent)
            verify_extensions += 'for (uint32_t cur_index = 0; cur_index < extensions.size(); ++cur_index) {\n'
            indent += 1
            for extension in self.extensions:
                number_of_required = len(extension.required_exts) - 1
                if extension.type == 'instance' and number_of_required > 0:
                    verify_extensions += self.writeIndent(indent)
                    verify_extensions += 'if (extensions[cur_index] == "%s") {\n' % extension.name
                    current_count = 0
                    indent += 1
                    verify_extensions += self.writeIndent(indent)
                    verify_extensions += 'for (uint32_t check_index = 0; check_index < extensions.size(); ++check_index) {\n'
                    indent += 1
                    verify_extensions += self.writeIndent(indent)
                    verify_extensions += 'if (cur_index == check_index) {\n'
                    verify_extensions += self.writeIndent(indent + 1)
                    verify_extensions += 'continue;\n'
                    verify_extensions += self.writeIndent(indent)
                    verify_extensions += '}\n'
                    current_count = 0
                    for required_ext in extension.required_exts:
                        if current_count > 0:
                            found = False
                            for extension_look in self.extensions:
                                if extension_look.name == required_ext:
                                    found = True
                                    if extension_look.type != 'instance':
                                        verify_extensions += self.printCodeGenErrorMessage('Instance extension "%s" requires non-instance extension "%s" which is not allowed' % (
                                            self.currentExtension, required_ext))
                            if not found:
                                verify_extensions += self.printCodeGenErrorMessage('Instance extension "%s" lists extension "%s" as a requirement, but'
                                                                                   ' it is not defined in the registry.' % (
                                                                                       self.currentExtension, required_ext))
                            verify_extensions += self.writeIndent(indent)
                            verify_extensions += 'if (!ExtensionEnabled(extensions, "%s")) {\n' % required_ext
                            indent += 1
                            verify_extensions += self.writeIndent(indent)
                            verify_extensions += 'if (nullptr != gen_instance_info) {\n'
                            indent += 1
                            verify_extensions += self.writeIndent(indent)
                            verify_extensions += 'std::string vuid = "VUID-";\n'
                            verify_extensions += self.writeIndent(indent)
                            verify_extensions += 'vuid += command;\n'
                            verify_extensions += self.writeIndent(indent)
                            verify_extensions += 'vuid += "-";\n'
                            verify_extensions += self.writeIndent(indent)
                            verify_extensions += 'vuid += struct_name;\n'
                            verify_extensions += self.writeIndent(indent)
                            verify_extensions += 'vuid += "-parameter";\n'
                            verify_extensions += self.writeIndent(indent)
                            verify_extensions += 'CoreValidLogMessage(gen_instance_info, vuid, VALID_USAGE_DEBUG_SEVERITY_ERROR,\n'
                            verify_extensions += self.writeIndent(indent)
                            verify_extensions += '                    command, objects_info,\n'
                            verify_extensions += self.writeIndent(indent)
                            verify_extensions += '                    "Missing extension dependency \\"%s\\" (required by extension" \\\n' % required_ext
                            verify_extensions += self.writeIndent(indent)
                            verify_extensions += '                    "\\"%s\\") from enabled extension list");\n' % extension.name
                            indent -= 1
                            verify_extensions += self.writeIndent(indent)
                            verify_extensions += '}\n'
                            verify_extensions += self.writeIndent(indent)
                            verify_extensions += 'return false;\n'
                            indent -= 1
                            verify_extensions += self.writeIndent(indent)
                            verify_extensions += '}\n'
                        current_count += 1
                    indent -= 1
                    verify_extensions += self.writeIndent(indent)
                    verify_extensions += '}\n'
                    indent -= 1
                    verify_extensions += self.writeIndent(indent)
                    verify_extensions += '}\n'
            indent -= 1
            verify_extensions += self.writeIndent(indent)
            verify_extensions += '}\n'
        else:
            verify_extensions += self.writeIndent(indent)
            verify_extensions += '// No instance extensions to check dependencies for\n'
        verify_extensions += self.writeIndent(indent)
        verify_extensions += 'return true;\n'
        verify_extensions += '}\n\n'
        verify_extensions += 'bool ValidateSystemExtensionDependencies(GenValidUsageXrInstanceInfo *gen_instance_info,\n'
        verify_extensions += '                                         const std::string &command,\n'
        verify_extensions += '                                         const std::string &struct_name,\n'
        verify_extensions += '                                         std::vector<GenValidUsageXrObjectInfo>& objects_info,\n'
        verify_extensions += '                                         std::vector<std::string> &extensions) {\n'
        indent = 1
        if number_of_system_extensions > 0:
            verify_extensions += self.writeIndent(indent)
            verify_extensions += 'for (uint32_t cur_index = 0; cur_index < extensions.size(); ++cur_index) {\n'
            indent += 1
            for extension in self.extensions:
                number_of_required = len(self.required_exts) - 1
                if extension.type == 'system' and number_of_required > 0:
                    verify_extensions += self.writeIndent(indent)
                    verify_extensions += 'if (extensions[cur_index] == "%s") {\n' % extension.name
                    current_count = 0
                    indent += 1
                    verify_extensions += self.writeIndent(indent)
                    verify_extensions += 'for (uint32_t check_index = 0; check_index < extensions.size(); ++check_index) {\n'
                    indent += 1
                    verify_extensions += self.writeIndent(indent)
                    verify_extensions += 'if (cur_index == check_index) {\n'
                    verify_extensions += self.writeIndent(indent + 1)
                    verify_extensions += 'continue;\n'
                    verify_extensions += self.writeIndent(indent)
                    verify_extensions += '}\n'
                    current_count = 0
                    for required_ext in extension.required_exts:
                        if current_count > 0:
                            found = False
                            is_instance = False
                            for extension_look in self.extensions:
                                if extension_look.name == required_ext:
                                    found = True
                                    if extension_look.type == 'instance':
                                        is_instance = True
                                    if not is_instance and extension_look.type != 'system':
                                        verify_extensions += self.printCodeGenErrorMessage('System extension "%s" has an extension dependency on extension "%s" '
                                                                                           'which is of an invalid type.' % (
                                                                                               self.currentExtension, required_ext))
                            if not found:
                                verify_extensions += self.printCodeGenErrorMessage('System extension "%s" lists extension "%s" as a requirement, but'
                                                                                   ' it is not defined in the registry.' % (
                                                                                       self.currentExtension, required_ext))
                            if is_instance:
                                verify_extensions += self.writeIndent(indent)
                                verify_extensions += '// This is an instance extension dependency, so make sure it is enabled in the instance\n'
                                verify_extensions += self.writeIndent(indent)
                                verify_extensions += 'if (!ExtensionEnabled(gen_instance_info->enabled_extensions, "%s") {\n' % required_ext
                            else:
                                verify_extensions += self.writeIndent(indent)
                                verify_extensions += 'if (!ExtensionEnabled(extensions, "%s")) {\n' % required_ext
                            indent += 1
                            verify_extensions += self.writeIndent(indent)
                            verify_extensions += 'std::string vuid = "VUID-";\n'
                            verify_extensions += self.writeIndent(indent)
                            verify_extensions += 'vuid += command;\n'
                            verify_extensions += self.writeIndent(indent)
                            verify_extensions += 'vuid += "-";\n'
                            verify_extensions += self.writeIndent(indent)
                            verify_extensions += 'vuid += struct_name;\n'
                            verify_extensions += self.writeIndent(indent)
                            verify_extensions += 'vuid += "-parameter";\n'
                            verify_extensions += self.writeIndent(indent)
                            verify_extensions += 'CoreValidLogMessage(gen_instance_info, vuid, VALID_USAGE_DEBUG_SEVERITY_ERROR,\n'
                            verify_extensions += self.writeIndent(indent)
                            verify_extensions += '                    command, objects_info,\n'
                            verify_extensions += self.writeIndent(indent)
                            verify_extensions += '                    "Missing extension dependency \\"%s\\" (required by extension" \\' % required_ext
                            verify_extensions += self.writeIndent(indent)
                            verify_extensions += '                    "\\"%s\\") from enabled extension list");\n' % extension.name
                            verify_extensions += self.writeIndent(indent)
                            verify_extensions += 'return false;\n'
                            indent -= 1
                            verify_extensions += self.writeIndent(indent)
                            verify_extensions += '}\n'
                        current_count += 1
                    indent -= 1
                    verify_extensions += self.writeIndent(indent)
                    verify_extensions += '}\n'
                    indent -= 1
                    verify_extensions += self.writeIndent(indent)
                    verify_extensions += '}\n'
            indent -= 1
            verify_extensions += self.writeIndent(indent)
            verify_extensions += '}\n'
        else:
            verify_extensions += self.writeIndent(indent)
            verify_extensions += '// No system extensions to check dependencies for\n'
        verify_extensions += self.writeIndent(indent)
        verify_extensions += 'return true;\n'
        verify_extensions += '}\n\n'
        return verify_extensions

    # Generate C++ enum and utility functions for verify that handles are valid.
    #   self            the ValidationSourceOutputGenerator object
    def writeValidateHandleChecks(self):
        verify_handle = ''
        for handle in self.api_handles:
            if handle.protect_value:
                verify_handle += '#if %s\n' % handle.protect_string
            indent = 1
            lower_handle_name = handle.name[2:].lower()
            verify_handle += 'ValidateXrHandleResult Verify%sHandle(const %s* handle_to_check) {\n' % (
                handle.name, handle.name)
            verify_handle += self.writeIndent(indent)
            verify_handle += 'try {\n'
            indent = indent + 1
            verify_handle += self.writeIndent(indent)
            verify_handle += 'if (nullptr == handle_to_check) {\n'
            verify_handle += self.writeIndent(indent + 1)
            verify_handle += 'return VALIDATE_XR_HANDLE_INVALID;\n'
            verify_handle += self.writeIndent(indent)
            verify_handle += '}\n'
            verify_handle += self.writeIndent(indent)
            verify_handle += '// XR_NULL_HANDLE is valid in some cases, so we want to return that we found that value\n'
            verify_handle += self.writeIndent(indent)
            verify_handle += '// and let the calling function decide what to do with it.\n'
            verify_handle += self.writeIndent(indent)
            verify_handle += 'if (CHECK_FOR_NULL_HANDLE(*handle_to_check)) {\n'
            verify_handle += self.writeIndent(indent + 1)
            verify_handle += 'return VALIDATE_XR_HANDLE_NULL;\n'
            verify_handle += self.writeIndent(indent)
            verify_handle += '}\n\n'
            verify_handle += self.writeIndent(indent)
            verify_handle += '// Try to find the handle in the appropriate map\n'
            verify_handle += self.writeIndent(indent)
            verify_handle += 'std::unique_lock<std::mutex> lock(g_%s_dispatch_mutex);\n' % lower_handle_name
            verify_handle += self.writeIndent(indent)
            verify_handle += 'auto entry_returned = g_%s_info_map.find(*handle_to_check);\n' % lower_handle_name
            verify_handle += self.writeIndent(indent)
            verify_handle += '// If it is not a valid handle, it should return the end of the map.\n'
            verify_handle += self.writeIndent(indent)
            verify_handle += 'if (g_%s_info_map.end() == entry_returned) {\n' % lower_handle_name
            verify_handle += self.writeIndent(indent + 1)
            verify_handle += 'return VALIDATE_XR_HANDLE_INVALID;\n'
            verify_handle += self.writeIndent(indent)
            verify_handle += '}\n'
            verify_handle += self.writeIndent(indent)
            verify_handle += 'return VALIDATE_XR_HANDLE_SUCCESS;\n'
            indent = indent - 1
            verify_handle += self.writeIndent(indent)
            verify_handle += '} catch (...) {\n'
            verify_handle += self.writeIndent(indent + 1)
            verify_handle += 'return VALIDATE_XR_HANDLE_INVALID;\n'
            verify_handle += self.writeIndent(indent)
            verify_handle += '}\n'
            verify_handle += '}\n\n'
            if handle.protect_value:
                verify_handle += '#endif // %s\n' % handle.protect_string
        return verify_handle

    # Generate C++ utility functions for verify that handles share a parent.
    #   self            the ValidationSourceOutputGenerator object
    def writeValidateHandleParent(self):
        verify_parent = '// Implementation function to get parent handle information\n'
        verify_parent += 'bool GetXrParent(const XrObjectType inhandle_type, const XR_VALIDATION_GENERIC_HANDLE_TYPE inhandle,\n'
        verify_parent += '                            XrObjectType& outhandle_type, XR_VALIDATION_GENERIC_HANDLE_TYPE& outhandle) {\n'
        indent = 1
        for handle in self.api_handles:
            if handle.name == 'XrInstance':
                verify_parent += self.writeIndent(indent)
                verify_parent += 'if (inhandle_type == XR_OBJECT_TYPE_INSTANCE) {\n'
                verify_parent += self.writeIndent(indent + 1)
                verify_parent += 'return false;\n'
                verify_parent += self.writeIndent(indent)
                verify_parent += '}\n'
            else:
                handle_lower = handle.name[2:].lower()
                handle_mutex = 'g_'
                handle_mutex += handle_lower
                handle_mutex += '_dispatch_mutex'
                handle_lock = '%s_lock' % handle_lower
                handle_map = 'g_'
                handle_map += handle_lower
                handle_map += '_info_map[CONVERT_GENERIC_TO_HANDLE(%s, inhandle)]' % handle.name
                verify_parent += self.writeIndent(indent)
                verify_parent += 'if (inhandle_type == %s) {\n' % self.genXrObjectType(
                    handle.name)
                indent += 1
                verify_parent += self.writeIndent(indent)
                verify_parent += '// Get the object and parent of the handle\n'
                verify_parent += self.writeIndent(indent)
                verify_parent += 'std::unique_lock<std::mutex> %s(%s);\n' % (handle_lock, handle_mutex)
                verify_parent += self.writeIndent(indent)
                verify_parent += 'GenValidUsageXrHandleInfo *handle_info = %s;\n' % handle_map
                verify_parent += self.writeIndent(indent)
                verify_parent += 'outhandle_type = handle_info->direct_parent_type;\n'
                verify_parent += self.writeIndent(indent)
                verify_parent += 'outhandle = handle_info->direct_parent_handle;\n'
                verify_parent += self.writeIndent(indent)
                verify_parent += 'return true;\n'
                indent -= 1
                verify_parent += self.writeIndent(indent)
                verify_parent += '}\n'
        verify_parent += '    return false;\n'
        verify_parent += '}\n\n'
        verify_parent += '// Implementation of VerifyXrParent function\n'
        verify_parent += 'bool VerifyXrParent(XrObjectType handle1_type, const XR_VALIDATION_GENERIC_HANDLE_TYPE handle1,\n'
        verify_parent += '                               XrObjectType handle2_type, const XR_VALIDATION_GENERIC_HANDLE_TYPE handle2,\n'
        verify_parent += '                               bool check_this) {\n'
        indent = 1
        verify_parent += self.writeIndent(indent)
        verify_parent += 'if (CHECK_FOR_NULL_HANDLE(handle1) || CHECK_FOR_NULL_HANDLE(handle2)) {\n'
        verify_parent += self.writeIndent(indent + 1)
        verify_parent += 'return false;\n'
        verify_parent += self.writeIndent(indent)
        verify_parent += '} else if (check_this && handle1_type == handle2_type) {\n'
        verify_parent += self.writeIndent(indent + 1)
        verify_parent += 'return (handle1 == handle2);\n'
        verify_parent += self.writeIndent(indent)
        verify_parent += '}\n'
        verify_parent += self.writeIndent(indent)
        verify_parent += 'if (handle1_type == XR_OBJECT_TYPE_INSTANCE && handle2_type != XR_OBJECT_TYPE_INSTANCE) {\n'
        indent += 1
        verify_parent += self.writeIndent(indent)
        verify_parent += 'XrObjectType parent_type;\n'
        verify_parent += self.writeIndent(indent)
        verify_parent += 'XR_VALIDATION_GENERIC_HANDLE_TYPE parent_handle;\n'
        verify_parent += self.writeIndent(indent)
        verify_parent += 'if (!GetXrParent(handle2_type, handle2, parent_type, parent_handle)) {\n'
        verify_parent += self.writeIndent(indent + 1)
        verify_parent += 'return false;\n'
        verify_parent += self.writeIndent(indent)
        verify_parent += '}\n'
        verify_parent += self.writeIndent(indent)
        verify_parent += 'return VerifyXrParent(handle1_type, handle1, parent_type, parent_handle, true);\n'
        indent -= 1
        verify_parent += self.writeIndent(indent)
        verify_parent += '} else if (handle2_type == XR_OBJECT_TYPE_INSTANCE && handle1_type != XR_OBJECT_TYPE_INSTANCE) {\n'
        indent += 1
        verify_parent += self.writeIndent(indent)
        verify_parent += 'XrObjectType parent_type;\n'
        verify_parent += self.writeIndent(indent)
        verify_parent += 'XR_VALIDATION_GENERIC_HANDLE_TYPE parent_handle;\n'
        verify_parent += self.writeIndent(indent)
        verify_parent += 'if (!GetXrParent(handle1_type, handle1, parent_type, parent_handle)) {\n'
        verify_parent += self.writeIndent(indent + 1)
        verify_parent += 'return false;\n'
        verify_parent += self.writeIndent(indent)
        verify_parent += '}\n'
        verify_parent += self.writeIndent(indent)
        verify_parent += 'return VerifyXrParent(parent_type, parent_handle, handle2_type, handle2, true);\n'
        indent -= 1
        verify_parent += self.writeIndent(indent)
        verify_parent += '} else {\n'
        indent += 1
        verify_parent += self.writeIndent(indent)
        verify_parent += 'XrObjectType parent1_type;\n'
        verify_parent += self.writeIndent(indent)
        verify_parent += 'XR_VALIDATION_GENERIC_HANDLE_TYPE parent1_handle;\n'
        verify_parent += self.writeIndent(indent)
        verify_parent += self.writeIndent(indent)
        verify_parent += 'XrObjectType parent2_type;\n'
        verify_parent += self.writeIndent(indent)
        verify_parent += 'XR_VALIDATION_GENERIC_HANDLE_TYPE parent2_handle;\n'
        verify_parent += self.writeIndent(indent)
        verify_parent += 'if (!GetXrParent(handle1_type, handle1, parent1_type, parent1_handle)) {\n'
        verify_parent += self.writeIndent(indent + 1)
        verify_parent += 'return false;\n'
        verify_parent += self.writeIndent(indent)
        verify_parent += '}\n'
        verify_parent += self.writeIndent(indent)
        verify_parent += 'if (!GetXrParent(handle2_type, handle2, parent2_type, parent2_handle)) {\n'
        verify_parent += self.writeIndent(indent + 1)
        verify_parent += 'return false;\n'
        verify_parent += self.writeIndent(indent)
        verify_parent += '}\n'
        verify_parent += self.writeIndent(indent)
        verify_parent += 'if (parent1_type == handle2_type) {\n'
        verify_parent += self.writeIndent(indent + 1)
        verify_parent += 'return (parent1_handle == handle2);\n'
        verify_parent += self.writeIndent(indent)
        verify_parent += '} else if (handle1_type == parent2_type) {\n'
        verify_parent += self.writeIndent(indent + 1)
        verify_parent += 'return (handle1 == parent2_handle);\n'
        verify_parent += self.writeIndent(indent)
        verify_parent += '} else {\n'
        verify_parent += self.writeIndent(indent + 1)
        verify_parent += 'return VerifyXrParent(parent1_type, parent1_handle, parent2_type, parent2_handle, true);\n'
        verify_parent += self.writeIndent(indent)
        verify_parent += '}\n'
        indent -= 1
        verify_parent += self.writeIndent(indent)
        verify_parent += '}\n'
        verify_parent += self.writeIndent(indent)
        verify_parent += 'return false;\n'
        indent -= 1
        verify_parent += '}\n\n'
        return verify_parent

    # Generate inline C++ code to check if a 'next' chain is valid for the current structure.
    #   self            the ValidationSourceOutputGenerator object
    #   struct_type     the name of the type of structure performing the validation check
    #   member          the member generated in automatic_source_generator.py to validate
    #   indent          the number of "tabs" to space in for the resulting C+ code.
    def writeValidateStructNextCheck(self, struct_type, struct_name, member, indent):
        validate_struct_next = self.writeIndent(indent)
        validate_struct_next += 'std::vector<XrStructureType> valid_ext_structs;\n'
        validate_struct_next += self.writeIndent(indent)
        validate_struct_next += 'std::vector<XrStructureType> duplicate_ext_structs;\n'
        validate_struct_next += self.writeIndent(indent)
        validate_struct_next += 'std::vector<XrStructureType> encountered_structs;\n'
        if member.valid_extension_structs and len(member.valid_extension_structs) > 0:
            for valid_struct in member.valid_extension_structs:
                validate_struct_next += self.writeIndent(indent)
                validate_struct_next += 'valid_ext_structs.push_back(%s);\n' % self.genXrStructureType(
                    valid_struct)
        validate_struct_next += self.writeIndent(indent)
        validate_struct_next += 'NextChainResult next_result = ValidateNextChain(instance_info, command_name, objects_info,\n'
        validate_struct_next += self.writeIndent(indent)
        validate_struct_next += '                                                 %s->%s, valid_ext_structs,\n' % (
            struct_name, member.name)
        validate_struct_next += self.writeIndent(indent)
        validate_struct_next += '                                                 encountered_structs,\n'
        validate_struct_next += self.writeIndent(indent)
        validate_struct_next += '                                                 duplicate_ext_structs);\n'
        validate_struct_next += self.writeIndent(indent)
        validate_struct_next += '// No valid extension structs for this \'next\'.  Therefore, must be NULL\n'
        validate_struct_next += self.writeIndent(indent)
        validate_struct_next += '// or only contain a list of valid extension structures.\n'
        validate_struct_next += self.writeIndent(indent)
        validate_struct_next += 'if (NEXT_CHAIN_RESULT_ERROR == next_result) {\n'
        validate_struct_next += self.writeIndent(indent + 1)
        validate_struct_next += 'CoreValidLogMessage(instance_info, "VUID-%s-%s-next",\n' % (struct_type,
                                                                                             member.name)
        validate_struct_next += self.writeIndent(indent + 1)
        validate_struct_next += '                    VALID_USAGE_DEBUG_SEVERITY_ERROR, command_name,\n'
        validate_struct_next += self.writeIndent(indent + 1)
        validate_struct_next += '                    objects_info, "Invalid structure(s) in \\"next\\" chain for %s struct \\"%s\\"");\n' % (struct_type,
                                                                                                                                             member.name)
        validate_struct_next += self.writeIndent(indent + 1)
        validate_struct_next += 'xr_result = XR_ERROR_VALIDATION_FAILURE;\n'
        validate_struct_next += self.writeIndent(indent)
        validate_struct_next += '} else if (NEXT_CHAIN_RESULT_DUPLICATE_STRUCT == next_result) {\n'
        validate_struct_next += self.writeIndent(indent + 1)
        validate_struct_next += 'char struct_type_buffer[XR_MAX_STRUCTURE_NAME_SIZE];\n'
        validate_struct_next += self.writeIndent(indent + 1)
        validate_struct_next += 'std::string error_message = "Multiple structures of the same type(s) in \\"next\\" chain for ";\n'
        validate_struct_next += self.writeIndent(indent + 1)
        validate_struct_next += 'error_message += "%s : ";\n' % struct_type
        validate_struct_next += self.writeIndent(indent + 1)
        validate_struct_next += 'if (nullptr != instance_info) {\n'
        validate_struct_next += self.writeIndent(indent + 2)
        validate_struct_next += 'bool wrote_struct = false;\n'
        validate_struct_next += self.writeIndent(indent + 2)
        validate_struct_next += 'for (uint32_t dup = 0; dup < duplicate_ext_structs.size(); ++dup) {\n'
        validate_struct_next += self.writeIndent(indent + 3)
        validate_struct_next += 'if (XR_SUCCESS == instance_info->dispatch_table->StructureTypeToString(instance_info->instance,\n'
        validate_struct_next += self.writeIndent(indent + 3)
        validate_struct_next += '                                                                       duplicate_ext_structs[dup],\n'
        validate_struct_next += self.writeIndent(indent + 3)
        validate_struct_next += '                                                                       struct_type_buffer)) {\n'
        validate_struct_next += self.writeIndent(indent + 4)
        validate_struct_next += 'if (wrote_struct) {\n'
        validate_struct_next += self.writeIndent(indent + 5)
        validate_struct_next += 'error_message += ", ";\n'
        validate_struct_next += self.writeIndent(indent + 4)
        validate_struct_next += '} else {\n'
        validate_struct_next += self.writeIndent(indent + 5)
        validate_struct_next += 'wrote_struct = true;\n'
        validate_struct_next += self.writeIndent(indent + 4)
        validate_struct_next += '}\n'
        validate_struct_next += self.writeIndent(indent + 4)
        validate_struct_next += 'error_message += struct_type_buffer;\n'
        validate_struct_next += self.writeIndent(indent + 3)
        validate_struct_next += '}\n'
        validate_struct_next += self.writeIndent(indent + 2)
        validate_struct_next += '}\n'
        validate_struct_next += self.writeIndent(indent + 1)
        validate_struct_next += '}\n'
        validate_struct_next += self.writeIndent(indent + 1)
        validate_struct_next += 'CoreValidLogMessage(instance_info, "VUID-%s-next-unique",\n' % struct_type
        validate_struct_next += self.writeIndent(indent + 1)
        validate_struct_next += '                    VALID_USAGE_DEBUG_SEVERITY_ERROR, command_name,\n'
        validate_struct_next += self.writeIndent(indent + 1)
        validate_struct_next += '                    objects_info,\n'
        validate_struct_next += self.writeIndent(indent + 1)
        validate_struct_next += '"Multiple structures of the same type(s) in \\"next\\" chain for %s struct");\n' %  struct_type
        validate_struct_next += self.writeIndent(indent + 1)
        validate_struct_next += 'xr_result = XR_ERROR_VALIDATION_FAILURE;\n'
        validate_struct_next += self.writeIndent(indent)
        validate_struct_next += '}\n'
        return validate_struct_next


    # Generate inline C++ code to check if a pointer to a variable or array is valid.
    #   self                the ValidationSourceOutputGenerator object
    #   cmd_struct_name     the name of the structure or command generating this validation check.
    #   member_param_name   the name of the member or parameter getting validated
    #   member_param_type   the type of the member or parameter getting validated
    #   pointer_to_check    the full name of the pointer to check (usually cmd_struct_name +
    #                       member_param_name in some fashion)
    #   full_count_var      the full name of the array count variable (if this is an array), or None
    #   short_count_var     the short name of the array count variable (if this is an array), or None
    #   is_in_cmd           Boolean indicating that this is being called directly from inside a command
    #   indent              the number of "tabs" to space in for the resulting C+ code.
    def writeValidatePointerArrayNonNull(self, cmd_struct_name, member_param_name, member_param_type,
                                         pointer_to_check, full_count_var, short_count_var, is_in_cmd,
                                         indent):
        array_check = self.writeIndent(indent)
        instance_info_string = 'instance_info'
        command_string = 'command_name'
        error_prefix = ''
        if is_in_cmd:
            if cmd_struct_name == 'xrCreateInstance':
                instance_info_string = 'nullptr'
            else:
                instance_info_string = 'gen_instance_info'
            command_string = '"%s"' % cmd_struct_name
            error_prefix = 'Invalid NULL for'
        else:
            error_prefix = '%s contains invalid NULL for' % cmd_struct_name
        if full_count_var is None or len(full_count_var) == 0:
            array_check += '// Non-optional pointer/array variable that needs to not be NULL\n'
            array_check += self.writeIndent(indent)
            array_check += 'if (nullptr == %s) {\n' % pointer_to_check
            indent = indent + 1
            array_check += self.writeIndent(indent)
            array_check += 'CoreValidLogMessage(%s, "VUID-%s-%s-parameter",\n' % (instance_info_string,
                                                                                  cmd_struct_name,
                                                                                  member_param_name)
            array_check += self.writeIndent(indent)
            array_check += '                    VALID_USAGE_DEBUG_SEVERITY_ERROR, %s, objects_info,\n' % command_string
            array_check += self.writeIndent(indent)
            array_check += '                    "%s %s \\"%s\\" which is not "\n' % (error_prefix,
                                                                                     member_param_type,
                                                                                     member_param_name)
            array_check += self.writeIndent(indent)
            array_check += '                    "optional and must be non-NULL");\n'
        else:
            array_check += '// Pointer/array variable with a length variable.  Make sure that\n'
            array_check += self.writeIndent(indent)
            array_check += '// if length variable is non-zero that the pointer is not NULL\n'
            array_check += self.writeIndent(indent)
            array_check += 'if (nullptr == %s && 0 != %s) {\n' % (
                pointer_to_check, full_count_var)
            indent = indent + 1
            array_check += self.writeIndent(indent)
            array_check += 'CoreValidLogMessage(%s, "VUID-%s-%s-parameter",\n' % (instance_info_string,
                                                                                  cmd_struct_name,
                                                                                  member_param_name)
            array_check += self.writeIndent(indent)
            array_check += '                    VALID_USAGE_DEBUG_SEVERITY_ERROR, %s, objects_info,\n' % command_string
            array_check += self.writeIndent(indent)
            array_check += '                    "%s %s \\"%s\\" is which not "\n' % (error_prefix,
                                                                                     member_param_type,
                                                                                     member_param_name)
            array_check += self.writeIndent(indent)
            array_check += '                    "optional since \\"%s\\" is set and must be non-NULL");\n' % short_count_var
        array_check += self.writeIndent(indent)
        array_check += 'return XR_ERROR_VALIDATION_FAILURE;\n'
        indent = indent - 1
        array_check += self.writeIndent(indent)
        array_check += '}\n'
        return array_check

    # Write an inline check to make sure an Enum is valid
    #   self                the ValidationSourceOutputGenerator object
    #   cmd_struct_name     the name of the structure or command generating this validation check.
    #   cmd_name_param      the name of the parameter containing the command name
    #   param_type          the type of enum to validate getting validated
    #   param_name          the name of the parameter to validate
    #   full_param_name     the full name of the parameter to check (usually cmd_struct_name +
    #                       member_param_name in some fashion)
    #   param_is_pointer    Boolean indicate that the parameter is a pointer
    #   is_in_cmd           Boolean indicating that this is being called directly from inside a command
    #   indent              the number of "tabs" to space in for the resulting C+ code.
    def writeValidateInlineEnum(self, cmd_struct_name, cmd_name_param, param_type, param_name, full_param_name,
                                param_is_pointer, is_in_cmd, indent):
        int_indent = indent
        inline_enum_str = self.writeIndent(int_indent)
        inline_enum_str += '// Make sure the enum type %s value is valid\n' % param_type
        inline_enum_str += self.writeIndent(int_indent)
        pointer_string = ''
        if param_is_pointer:
            pointer_string = '*'
        instance_info_string = 'instance_info'
        error_prefix = ''
        if is_in_cmd:
            if cmd_struct_name == 'xrCreateInstance':
                instance_info_string = 'nullptr'
            else:
                instance_info_string = 'gen_instance_info'
            error_prefix = 'Invalid'
        else:
            error_prefix = '%s contains invalid' % cmd_struct_name
        inline_enum_str += 'if (!ValidateXrEnum(%s, %s, "%s", "%s", objects_info, %s%s)) {\n' % (
            instance_info_string, cmd_name_param, cmd_struct_name, param_name, pointer_string, full_param_name)
        int_indent = int_indent + 1
        inline_enum_str += self.writeIndent(int_indent)
        inline_enum_str += 'std::ostringstream oss_enum;\n'
        inline_enum_str += self.writeIndent(int_indent)
        inline_enum_str += 'oss_enum << std::hex << static_cast<int32_t>(%s%s);\n' % (pointer_string,
                                                                                      full_param_name)
        inline_enum_str += self.writeIndent(int_indent)
        inline_enum_str += 'std::string error_str = "%s %s \\"%s\\" enum value 0x";\n' % (error_prefix,
                                                                                          param_type,
                                                                                          param_name)
        inline_enum_str += self.writeIndent(int_indent)
        inline_enum_str += 'error_str += oss_enum.str();\n'
        inline_enum_str += self.writeIndent(int_indent)
        inline_enum_str += 'CoreValidLogMessage(%s, "VUID-%s-%s-parameter",\n' % (instance_info_string,
                                                                                  cmd_struct_name,
                                                                                  param_name)
        inline_enum_str += self.writeIndent(int_indent)
        inline_enum_str += '                    VALID_USAGE_DEBUG_SEVERITY_ERROR, %s,\n' % cmd_name_param
        inline_enum_str += self.writeIndent(int_indent)
        inline_enum_str += '                    objects_info, error_str);\n'
        inline_enum_str += self.writeIndent(int_indent)
        inline_enum_str += 'return XR_ERROR_VALIDATION_FAILURE;\n'
        int_indent = int_indent - 1
        inline_enum_str += self.writeIndent(int_indent)
        inline_enum_str += '}\n'
        return inline_enum_str

    # Write an inline check to make sure a flag is valid
    #   self                the ValidationSourceOutputGenerator object
    #   cmd_struct_name     the name of the structure or command generating this validation check.
    #   cmd_name_param      the name of the parameter containing the command name
    #   param_type          the type of flag to validate getting validated
    #   param_name          the name of the parameter to validate
    #   full_param_name     the full name of the parameter to check (usually cmd_struct_name +
    #                       member_param_name in some fashion)
    #   param_is_pointer    Boolean indicating that the parameter is a pointer
    #   is_optional         Boolean indicating that the parameter is optional
    #   is_in_cmd           Boolean indicating that this is being called directly from inside a command
    #   indent              the number of "tabs" to space in for the resulting C+ code.
    def writeValidateInlineFlag(self, cmd_struct_name, cmd_name_param, param_type, param_name, full_param_name,
                                param_is_pointer, is_optional, is_in_cmd, indent):
        int_indent = indent
        inline_flag_str = self.writeIndent(int_indent)
        # Add underscore between lowercase then uppercase
        result_name = re.sub('([a-z0-9])([A-Z])', r'\1_\2', param_type)
        # Change to uppercase
        result_name = result_name[3:].lower()
        result_name += '_result'
        pointer_string = ''
        if param_is_pointer:
            pointer_string = '*'
        instance_info_string = 'instance_info'
        error_prefix = ''
        if is_in_cmd:
            if cmd_struct_name == 'xrCreateInstance':
                instance_info_string = 'nullptr'
            else:
                instance_info_string = 'gen_instance_info'
            error_prefix = 'Invalid'
        else:
            error_prefix = '%s invalid member' % cmd_struct_name
        inline_flag_str += 'ValidateXrFlagsResult %s = ValidateXr%s(%s%s);\n' % (result_name,
                                                                                 param_type[2:],
                                                                                 pointer_string,
                                                                                 full_param_name)
        if self.flagHasValidValues(param_type):
            if not is_optional:
                # Must be non-zero
                inline_flag_str += self.writeIndent(int_indent)
                inline_flag_str += '// Flags must be non-zero in this case.\n'
                inline_flag_str += self.writeIndent(int_indent)
                inline_flag_str += 'if (VALIDATE_XR_FLAGS_ZERO == %s) {\n' % result_name
                int_indent = int_indent + 1
                inline_flag_str += self.writeIndent(int_indent)
                inline_flag_str += 'CoreValidLogMessage(%s, "VUID-%s-%s-requiredbitmask",\n' % (instance_info_string,
                                                                                                cmd_struct_name,
                                                                                                param_name)
                inline_flag_str += self.writeIndent(int_indent)
                inline_flag_str += '                    VALID_USAGE_DEBUG_SEVERITY_ERROR, %s,\n' % cmd_name_param
                inline_flag_str += self.writeIndent(int_indent)
                inline_flag_str += '                    objects_info, "%s \\"%s\\" flag must be non-zero");\n' % (param_type,
                                                                                                                  param_name)
                inline_flag_str += self.writeIndent(int_indent)
                inline_flag_str += 'return XR_ERROR_VALIDATION_FAILURE;\n'
                int_indent = int_indent - 1
                inline_flag_str += self.writeIndent(int_indent)
                inline_flag_str += '} else if (VALIDATE_XR_FLAGS_SUCCESS != %s) {\n' % result_name
            else:
                inline_flag_str += self.writeIndent(int_indent)
                inline_flag_str += '// Valid flags available, so it must be invalid to fail.\n'
                inline_flag_str += self.writeIndent(int_indent)
                inline_flag_str += 'if (VALIDATE_XR_FLAGS_INVALID == %s) {\n' % result_name
            int_indent = int_indent + 1
            inline_flag_str += self.writeIndent(int_indent)
            inline_flag_str += '// Otherwise, flags must be valid.\n'
            inline_flag_str += self.writeIndent(int_indent)
            inline_flag_str += 'std::ostringstream oss_enum;\n'
            inline_flag_str += self.writeIndent(int_indent)
            inline_flag_str += 'oss_enum << std::hex << static_cast<int32_t>(%s%s);\n' % (pointer_string,
                                                                                          full_param_name)
            inline_flag_str += self.writeIndent(int_indent)
            inline_flag_str += 'std::string error_str = "%s %s \\"%s\\" flag value 0x";\n' % (error_prefix,
                                                                                              param_type,
                                                                                              param_name)
            inline_flag_str += self.writeIndent(int_indent)
            inline_flag_str += 'error_str += oss_enum.str();\n'
            inline_flag_str += self.writeIndent(int_indent)
            inline_flag_str += 'error_str += " contains illegal bit";\n'
            inline_flag_str += self.writeIndent(int_indent)
            inline_flag_str += 'CoreValidLogMessage(%s, "VUID-%s-%s-parameter",\n' % (instance_info_string,
                                                                                      cmd_struct_name,
                                                                                      param_name)
            inline_flag_str += self.writeIndent(int_indent)
            inline_flag_str += '                    VALID_USAGE_DEBUG_SEVERITY_ERROR, %s,\n' % cmd_name_param
            inline_flag_str += self.writeIndent(int_indent)
            inline_flag_str += '                    objects_info, error_str);\n'
            inline_flag_str += self.writeIndent(int_indent)
            inline_flag_str += 'return XR_ERROR_VALIDATION_FAILURE;\n'
            int_indent = int_indent - 1
            inline_flag_str += self.writeIndent(int_indent)
            inline_flag_str += '}\n'
        else:
            # Must be zero
            inline_flag_str += self.writeIndent(int_indent)
            inline_flag_str += '// Flags must be zero in this case.\n'
            inline_flag_str += self.writeIndent(int_indent)
            inline_flag_str += 'if (VALIDATE_XR_FLAGS_ZERO != %s) {\n' % result_name
            int_indent = int_indent + 1
            inline_flag_str += self.writeIndent(int_indent)
            inline_flag_str += 'CoreValidLogMessage(%s, "VUID-%s-%s-zerobitmask",\n' % (instance_info_string,
                                                                                        cmd_struct_name,
                                                                                        param_name)
            inline_flag_str += self.writeIndent(int_indent)
            inline_flag_str += '                    VALID_USAGE_DEBUG_SEVERITY_ERROR, %s,\n' % cmd_name_param
            inline_flag_str += self.writeIndent(int_indent)
            inline_flag_str += '                    objects_info, "%s \\"%s\\" flag must be zero");\n' % (param_type,
                                                                                                          param_name)
            inline_flag_str += self.writeIndent(int_indent)
            inline_flag_str += 'return XR_ERROR_VALIDATION_FAILURE;\n'
            int_indent = int_indent - 1
            inline_flag_str += self.writeIndent(int_indent)
            inline_flag_str += '}\n'
        return inline_flag_str

    # Write an inline check to make sure a handle is valid
    #   self                the ValidationSourceOutputGenerator object
    #   cmd_name            the name of the command generating this validation check.
    #   vuid_name           the name of the structure or command to put in the VUID
    #   member_param        the member or parameter generated in automatic_source_generator.py to validate
    #   mem_par_desc_name   Descriptive name of parameter
    #   output_result_type  Boolean indicating we need to output the handle result type (since it hasn't
    #                       been defined in the C++ code yet).
    #   return_on_null      Boolean indicating we need to return immediately if we encounter a NULL
    #   instance_info_name  Name of the parameter storing the instance information
    #   element_in_array    This is a single element in an array
    #   indent              the number of "tabs" to space in for the resulting C+ code.
    def writeValidateInlineHandleValidation(self, cmd_name, vuid_name, member_param, mem_par_desc_name,
                                            output_result_type, return_on_null, instance_info_name,
                                            element_in_array, indent):
        inline_validate_handle = ''
        adjust_to_pointer = ''
        if (not element_in_array and member_param.pointer_count == 0) or (element_in_array and member_param.pointer_count == 1):
            adjust_to_pointer = '&'
        inline_validate_handle += self.writeIndent(indent)
        if output_result_type:
            inline_validate_handle += 'ValidateXrHandleResult '
        inline_validate_handle += 'handle_result = Verify%sHandle(%s%s);\n' % (member_param.type, adjust_to_pointer,
                                                                               mem_par_desc_name)
        wrote_first_if = False
        if member_param.is_optional:
            # If we have to return on a Handle that has a value of XR_NULL_HANDLE, do so.
            if return_on_null:
                wrote_first_if = True
                inline_validate_handle += self.writeIndent(indent)
                inline_validate_handle += 'if (handle_result == VALIDATE_XR_HANDLE_NULL) {\n'
                inline_validate_handle += self.writeIndent(indent + 1)
                inline_validate_handle += '// Handle is optional so NULL is valid.  But we can\'t do anything else, either.\n'
                inline_validate_handle += self.writeIndent(indent + 1)
                inline_validate_handle += 'return XR_SUCCESS;\n'
                inline_validate_handle += self.writeIndent(indent)
                inline_validate_handle += '}'

        # Otherwise, catch the non-success case.  If we catch the NULL handle above, we add an "else" to
        # the if below.
        if not member_param.no_auto_validity:
            if wrote_first_if:
                inline_validate_handle += ' else '
            else:
                inline_validate_handle += self.writeIndent(indent)
            indent = indent + 1
            if member_param.is_optional:
                inline_validate_handle += 'if (handle_result == VALIDATE_XR_HANDLE_INVALID) {\n'
                inline_validate_handle += self.writeIndent(indent)
                inline_validate_handle += '// Not a valid handle\n'
            else:
                inline_validate_handle += 'if (handle_result != VALIDATE_XR_HANDLE_SUCCESS) {\n'
                inline_validate_handle += self.writeIndent(indent)
                inline_validate_handle += '// Not a valid handle or NULL (which is not valid in this case)\n'
            inline_validate_handle += self.writeIndent(indent)
            inline_validate_handle += 'std::ostringstream oss_handle;\n'
            inline_validate_handle += self.writeIndent(indent)
            inline_validate_handle += 'oss_handle << std::hex << reinterpret_cast<const void*>(%s);\n' % mem_par_desc_name
            inline_validate_handle += self.writeIndent(indent)
            inline_validate_handle += 'std::string error_str = "Invalid %s handle \\"%s\\" 0x";\n' % (member_param.type,
                                                                                                      member_param.name)
            inline_validate_handle += self.writeIndent(indent)
            inline_validate_handle += 'error_str += oss_handle.str();\n'
            inline_validate_handle += self.writeIndent(indent)
            inline_validate_handle += 'CoreValidLogMessage(%s, "VUID-%s-%s-parameter",\n' % (instance_info_name,
                                                                                             vuid_name,
                                                                                             member_param.name)
            inline_validate_handle += self.writeIndent(indent)
            inline_validate_handle += '                    VALID_USAGE_DEBUG_SEVERITY_ERROR, %s,\n' % cmd_name
            inline_validate_handle += self.writeIndent(indent)
            inline_validate_handle += '                    objects_info, error_str);\n'
            inline_validate_handle += self.writeIndent(indent)
            inline_validate_handle += 'return XR_ERROR_HANDLE_INVALID;\n'
            indent = indent - 1
            inline_validate_handle += self.writeIndent(indent)
            inline_validate_handle += '}\n'
        else:
            inline_validate_handle += '\n'
        return inline_validate_handle

    def outputParamMemberContents(self, is_command, struct_command_name, param_member, param_member_prefix, instance_info_variable,
                                  command_name_variable, is_first_param, primary_handle, primary_handle_desc_name, primary_handle_tuple,
                                  wrote_handle_proto, indent):
        param_member_contents = ''
        is_loop = False
        is_pointer = False
        is_array = param_member.is_array
        check_pointer_array_null = False
        loop_string = ''
        wrote_loop = False
        prefixed_param_member_name = param_member_prefix
        prefixed_param_member_name += param_member.name
        pre_loop_prefixed_param_member_name = prefixed_param_member_name
        loop_param_name = 'value_'
        loop_param_name += param_member.name.lower()
        loop_param_name += '_inc'
        if len(param_member.array_count_var) != 0:
            is_array = True
            if param_member.pointer_count > 0:
                is_pointer = True
        elif len(param_member.pointer_count_var) != 0:
            is_array = True
            if param_member.pointer_count > 1:
                is_pointer = True
        elif param_member.pointer_count > 0:
            is_pointer = True
        if is_array or is_pointer:
            check_pointer_array_null = not param_member.is_optional and not param_member.is_static_array
        short_count_var = None
        full_count_var = None
        if is_array:
            long_count_name = param_member_prefix
            if param_member.is_static_array:
                short_count_var = param_member.static_array_sizes[0]
                long_count_name = param_member.static_array_sizes[0]
            elif len(param_member.array_count_var) != 0:
                short_count_var = param_member.array_count_var
                if self.isAllUpperCase(param_member.array_count_var):
                    long_count_name = param_member.array_count_var
                else:
                    long_count_name += param_member.array_count_var
            else:
                short_count_var = param_member.pointer_count_var
                if self.isAllUpperCase(param_member.pointer_count_var):
                    long_count_name = param_member.pointer_count_var
                else:
                    long_count_name += param_member.pointer_count_var
            if check_pointer_array_null:
                full_count_var = long_count_name
                param_member_contents += self.writeValidatePointerArrayNonNull(struct_command_name,
                                                                               param_member.name,
                                                                               param_member.type,
                                                                               prefixed_param_member_name,
                                                                               full_count_var,
                                                                               short_count_var,
                                                                               is_command,
                                                                               indent)
            if (param_member.is_handle or self.isEnumType(param_member.type) or
                    (self.isStruct(param_member.type) and not self.isStructAlwaysValid(param_member.type))):
                loop_string += self.writeIndent(indent)
                loop_string += 'for (uint32_t %s = 0; %s < %s; ++%s) {\n' % (loop_param_name,
                                                                             loop_param_name,
                                                                             long_count_name,
                                                                             loop_param_name)
                indent = indent + 1
                prefixed_param_member_name = '%s[%s]' % (
                    prefixed_param_member_name, loop_param_name)
                is_loop = True
        elif check_pointer_array_null:
            param_member_contents += self.writeValidatePointerArrayNonNull(struct_command_name,
                                                                           param_member.name,
                                                                           param_member.type,
                                                                           prefixed_param_member_name,
                                                                           None,
                                                                           None,
                                                                           is_command,
                                                                           indent)

        if not param_member.is_static_array and len(param_member.array_length_for) > 0:
            if param_member.is_optional:
                param_member_contents += self.writeIndent(indent)
                param_member_contents += '// Optional array must be non-NULL when %s is non-zero\n' % prefixed_param_member_name
                param_member_contents += self.writeIndent(indent)
                param_member_contents += 'if (0 != %s && nullptr == %s%s) {\n' % (
                    prefixed_param_member_name, param_member_prefix, param_member.array_length_for)
                param_member_contents += self.writeIndent(indent + 1)
                param_member_contents += 'CoreValidLogMessage(%s, "VUID-%s-%s-parameter",\n' % (instance_info_variable,
                                                                                                struct_command_name,
                                                                                                param_member.array_length_for)
                param_member_contents += self.writeIndent(indent + 1)
                param_member_contents += '                    VALID_USAGE_DEBUG_SEVERITY_ERROR, %s,\n' % command_name_variable
                param_member_contents += self.writeIndent(indent + 1)
                param_member_contents += '                    objects_info,\n'
                param_member_contents += self.writeIndent(indent + 1)
                param_member_contents += '                    '
                if is_command:
                    param_member_contents += '"Command %s param %s' % (
                        struct_command_name, param_member.array_length_for)
                else:
                    param_member_contents += '"Structure %s member %s' % (
                        struct_command_name, param_member.name)
                param_member_contents += ' is NULL, but %s is greater than 0");\n' % prefixed_param_member_name
                param_member_contents += self.writeIndent(indent + 1)
                param_member_contents += 'xr_result = XR_ERROR_VALIDATION_FAILURE;\n'
                param_member_contents += self.writeIndent(indent)
                param_member_contents += '}\n'
            else:
                param_member_contents += self.writeIndent(indent)
                param_member_contents += '// Non-optional array length must be non-zero\n'
                param_member_contents += self.writeIndent(indent)
                param_member_contents += 'if (0 >= %s && nullptr != %s%s) {\n' % (
                    prefixed_param_member_name, param_member_prefix, param_member.array_length_for)
                param_member_contents += self.writeIndent(indent + 1)
                param_member_contents += 'CoreValidLogMessage(%s, "VUID-%s-%s-arraylength",\n' % (instance_info_variable,
                                                                                                  struct_command_name,
                                                                                                  param_member.name)
                param_member_contents += self.writeIndent(indent + 1)
                param_member_contents += '                    VALID_USAGE_DEBUG_SEVERITY_ERROR, %s,\n' % command_name_variable
                param_member_contents += self.writeIndent(indent + 1)
                param_member_contents += '                    objects_info,\n'
                param_member_contents += self.writeIndent(indent + 1)
                param_member_contents += '                    '
                if is_command:
                    param_member_contents += '"Command %s param %s' % (
                        struct_command_name, param_member.name)
                else:
                    param_member_contents += '"Structure %s member %s' % (
                        struct_command_name, param_member.name)
                param_member_contents += ' is non-optional and must be greater than 0");\n'
                param_member_contents += self.writeIndent(indent + 1)
                param_member_contents += 'xr_result = XR_ERROR_VALIDATION_FAILURE;\n'
                param_member_contents += self.writeIndent(indent)
                param_member_contents += '}\n'
        first_time_handle_check = not wrote_handle_proto
        if param_member.is_handle:
            if param_member.pointer_count == 0:
                param_member_contents += self.writeValidateInlineHandleValidation(command_name_variable,
                                                                                  struct_command_name,
                                                                                  param_member,
                                                                                  prefixed_param_member_name,
                                                                                  first_time_handle_check,
                                                                                  is_command,
                                                                                  instance_info_variable,
                                                                                  False,
                                                                                  indent)

                # If the first item is a handle, and this is a different handle, we need to verify that
                # one is either the parent of the other, or that they share a common ancestor.
                if primary_handle_tuple is not None and not first_time_handle_check:
                    current_handle_tuple = self.getHandle(param_member.type)
                    param_member_contents += self.writeInlineParentCheckCall(instance_info_variable,
                                                                             primary_handle_tuple,
                                                                             primary_handle,
                                                                             primary_handle_desc_name,
                                                                             current_handle_tuple,
                                                                             param_member,
                                                                             prefixed_param_member_name,
                                                                             struct_command_name,
                                                                             command_name_variable,
                                                                             indent)

                elif not is_command:
                    primary_handle_tuple = self.getHandle(param_member.type)
                    primary_handle = param_member
                    primary_handle_desc_name = prefixed_param_member_name
            elif is_array:
                param_member_contents += loop_string
                wrote_loop = True
                param_member_contents += self.writeValidateInlineHandleValidation(command_name_variable,
                                                                                  struct_command_name,
                                                                                  param_member,
                                                                                  prefixed_param_member_name,
                                                                                  first_time_handle_check,
                                                                                  is_command,
                                                                                  instance_info_variable,
                                                                                  True,
                                                                                  indent)
        elif self.isStruct(param_member.type) and not self.isStructAlwaysValid(param_member.type):
            param_member_contents += loop_string
            wrote_loop = True
            is_relation_group = False
            relation_group = None
            # Check to see if this struct is the base of a relation group
            for cur_rel_group in self.struct_relation_groups:
                if cur_rel_group.generic_struct_name == param_member.type:
                    relation_group = cur_rel_group
                    is_relation_group = True
                    break

            # If this struct is the base of a relation group, check to see if this call really should go to any one of
            # it's children instead of itself.
            if is_relation_group:
                for child in relation_group.child_struct_names:
                    child_struct = self.getStruct(child)
                    if child_struct.protect_value:
                        param_member_contents += '#if %s\n' % child_struct.protect_string

                    param_member_contents += self.writeIndent(indent)
                    param_member_contents += '// Validate if %s is a child structure of type %s and it is valid\n' % (
                        param_member.type, child)
                    param_member_contents += self.writeIndent(indent)

                    base_child_struct_name = child[2:].lower()

                    if is_pointer or is_array:
                        new_type_info = param_member.cdecl.replace(
                            param_member.type, child)
                        new_type_info = new_type_info.replace(
                            param_member.name, "")
                        new_type_info = new_type_info.strip().rstrip()
                        param_member_contents += '%s new_%s_value = reinterpret_cast<%s>(%s);\n' % (
                            new_type_info, base_child_struct_name, new_type_info, pre_loop_prefixed_param_member_name)
                        param_member_contents += self.writeIndent(indent)
                        deref_string = '->' if is_pointer else '.'
                        if is_array:
                            param_member_contents += 'if (new_%s_value[%s]%stype == %s) {\n' % (
                                base_child_struct_name, loop_param_name, deref_string, self.genXrStructureType(child))
                        else:
                            param_member_contents += 'if (new_%s_value%stype == %s) {\n' % (
                                base_child_struct_name, deref_string, self.genXrStructureType(child))
                    else:
                        param_member_contents += 'const %s* new_%s_value = reinterpret_cast<const %s*>(&%s);\n' % (
                            child, base_child_struct_name, child, pre_loop_prefixed_param_member_name)
                        param_member_contents += self.writeIndent(indent)
                        param_member_contents += 'if (new_%s_value->type == %s) {\n' % (
                            base_child_struct_name, self.genXrStructureType(child))
                    indent = indent + 1

                    if param_member.is_optional:
                        param_member_contents += self.writeIndent(indent)
                        param_member_contents += 'if (nullptr != new_%s_value) {\n' % base_child_struct_name
                        indent = indent + 1
                        param_member_contents += self.writeIndent(indent)
                        param_member_contents += 'xr_result = ValidateXrStruct(%s, %s,\n' % (
                            instance_info_variable, command_name_variable)
                        param_member_contents += self.writeIndent(indent)
                        param_member_contents += '                                                objects_info,'
                        if is_command:
                            param_member_contents += ' false,'
                        else:
                            param_member_contents += ' check_members,'
                        if is_array:
                            if is_pointer:
                                param_member_contents += ' new_%s_value[%s]);\n' % (
                                    base_child_struct_name, loop_param_name)
                            else:
                                param_member_contents += ' &new_%s_value[%s]);\n' % (
                                    base_child_struct_name, loop_param_name)
                        else:
                            param_member_contents += ' new_%s_value);\n' % base_child_struct_name
                    else:
                        param_member_contents += self.writeIndent(indent)
                        param_member_contents += 'xr_result = ValidateXrStruct(%s, %s,\n' % (
                            instance_info_variable, command_name_variable)
                        param_member_contents += self.writeIndent(indent)
                        param_member_contents += '                                                objects_info,'
                        if is_command:
                            param_member_contents += 'false,'
                        else:
                            param_member_contents += ' check_members,'
                        if is_array:
                            param_member_contents += ' new_%s_value[%s]);\n' % (
                                base_child_struct_name, loop_param_name)
                        else:
                            param_member_contents += ' new_%s_value);\n' % base_child_struct_name

                    param_member_contents += self.writeIndent(indent)
                    param_member_contents += 'if (XR_SUCCESS != xr_result) {\n'
                    indent = indent + 1
                    param_member_contents += self.writeIndent(indent)
                    param_member_contents += 'std::string error_message = "'
                    if is_command:
                        param_member_contents += 'Command %s param %s";\n' % (
                            struct_command_name, param_member.name)
                    else:
                        param_member_contents += 'Structure %s member %s";\n' % (
                            struct_command_name, param_member.name)
                    if is_array:
                        param_member_contents += self.writeIndent(indent)
                        param_member_contents += 'error_message += "[";\n'
                        param_member_contents += self.writeIndent(indent)
                        param_member_contents += 'error_message += std::to_string(%s);\n' % loop_param_name
                        param_member_contents += self.writeIndent(indent)
                        param_member_contents += 'error_message += "]";\n'
                    param_member_contents += self.writeIndent(indent)
                    param_member_contents += 'error_message += " is invalid";\n'

                    param_member_contents += self.writeIndent(indent)
                    param_member_contents += 'CoreValidLogMessage(%s, "VUID-%s-%s-parameter",\n' % (
                        instance_info_variable, struct_command_name, param_member.name)
                    param_member_contents += self.writeIndent(indent)
                    param_member_contents += '                    VALID_USAGE_DEBUG_SEVERITY_ERROR, %s,\n' % command_name_variable
                    param_member_contents += self.writeIndent(indent)
                    param_member_contents += '                    objects_info,\n'
                    param_member_contents += self.writeIndent(indent)
                    param_member_contents += '                    error_message);\n'
                    param_member_contents += self.writeIndent(indent)
                    param_member_contents += 'return XR_ERROR_VALIDATION_FAILURE;\n'
                    if is_array:
                        param_member_contents += self.writeIndent(indent)
                        param_member_contents += 'break;\n'
                        param_member_contents += self.writeIndent(indent - 1)
                        param_member_contents += '} else {\n'
                        param_member_contents += self.writeIndent(indent)
                        param_member_contents += 'continue;\n'

                    if param_member.is_optional:
                        param_member_contents += self.writeIndent(indent)
                        param_member_contents += '}\n'
                        indent = indent - 1

                    indent = indent - 1
                    param_member_contents += self.writeIndent(indent)
                    param_member_contents += '}\n'

                    indent = indent - 1
                    param_member_contents += self.writeIndent(indent)
                    param_member_contents += '}\n'

                    if child_struct.protect_value:
                        param_member_contents += '#endif // %s\n' % child_struct.protect_string

            param_member_contents += self.writeIndent(indent)
            if is_relation_group:
                param_member_contents += '// Validate that the base-structure %s is valid\n' % (
                                         param_member.type)
            else:
                param_member_contents += '// Validate that the structure %s is valid\n' % (
                                         param_member.type)
            param_member_contents += self.writeIndent(indent)
            if is_pointer:
                if param_member.is_optional:
                    param_member_contents += 'if (nullptr != %s) {\n' % prefixed_param_member_name
                    indent = indent + 1
                    param_member_contents += self.writeIndent(indent)
                    param_member_contents += 'xr_result = ValidateXrStruct(%s, %s,\n' % (
                        instance_info_variable, command_name_variable)
                    param_member_contents += self.writeIndent(indent)
                    param_member_contents += '                                                objects_info,'
                    if is_command:
                        param_member_contents += ' false,'
                    else:
                        param_member_contents += ' check_members,'
                    param_member_contents += ' %s);\n' % prefixed_param_member_name
                else:
                    param_member_contents += 'xr_result = ValidateXrStruct(%s, %s, objects_info,\n' % (
                        instance_info_variable, command_name_variable)
                    param_member_contents += self.writeIndent(indent)
                    param_member_contents += '                                                '
                    if is_command:
                        if param_member.is_const:
                            param_member_contents += 'true,'
                        else:
                            param_member_contents += 'false,'
                    else:
                        param_member_contents += 'check_members,'
                    param_member_contents += ' %s);\n' % prefixed_param_member_name
            else:
                param_member_contents += 'xr_result = ValidateXrStruct(%s, %s, objects_info,\n' % (
                    instance_info_variable, command_name_variable)
                param_member_contents += self.writeIndent(indent)
                param_member_contents += '                                                '
                if is_command:
                    param_member_contents += 'true,'
                else:
                    param_member_contents += 'check_members,'
                param_member_contents += ' &%s);\n' % prefixed_param_member_name

            param_member_contents += self.writeIndent(indent)
            param_member_contents += 'if (XR_SUCCESS != xr_result) {\n'
            indent = indent + 1
            param_member_contents += self.writeIndent(indent)
            param_member_contents += 'CoreValidLogMessage(%s, "VUID-%s-%s-parameter",\n' % (
                instance_info_variable, struct_command_name, param_member.name)
            param_member_contents += self.writeIndent(indent)
            param_member_contents += '                    VALID_USAGE_DEBUG_SEVERITY_ERROR, %s,\n' % command_name_variable
            param_member_contents += self.writeIndent(indent)
            param_member_contents += '                    objects_info,\n'
            param_member_contents += self.writeIndent(indent)
            param_member_contents += '                    '
            if is_command:
                param_member_contents += '"Command %s param %s' % (
                    struct_command_name, param_member.name)
            else:
                param_member_contents += '"Structure %s member %s' % (
                    struct_command_name, param_member.name)
            param_member_contents += ' is invalid");\n'
            param_member_contents += self.writeIndent(indent)
            param_member_contents += 'return xr_result;\n'
            indent = indent - 1
            if is_pointer and param_member.is_optional:
                param_member_contents += self.writeIndent(indent)
                param_member_contents += '}\n'
                indent = indent - 1
            param_member_contents += self.writeIndent(indent)
            param_member_contents += '}\n'
        elif self.isEnumType(param_member.type):
            if is_array:
                param_member_contents += loop_string
                wrote_loop = True
            param_member_contents += self.writeValidateInlineEnum(struct_command_name,
                                                                  command_name_variable,
                                                                  param_member.type,
                                                                  param_member.name,
                                                                  prefixed_param_member_name,
                                                                  is_pointer,
                                                                  is_command,
                                                                  indent)
        elif self.isFlagType(param_member.type):
            param_member_contents += self.writeValidateInlineFlag(struct_command_name,
                                                                  command_name_variable,
                                                                  param_member.type,
                                                                  param_member.name,
                                                                  prefixed_param_member_name,
                                                                  is_pointer,
                                                                  param_member.is_optional,
                                                                  is_command,
                                                                  indent)
        elif "void" not in param_member.type:
            if param_member.is_null_terminated:
                param_member_contents += self.writeIndent(indent)
                param_member_contents += '// NOTE: Can\'t validate "VUID-%s-%s-parameter" null-termination\n' % (struct_command_name,
                                                                                                                 param_member.name)
            elif param_member.pointer_count > 0:
                param_member_contents += self.writeIndent(indent)
                param_member_contents += '// NOTE: Can\'t validate "VUID-%s-%s-parameter" type\n' % (struct_command_name,
                                                                                                     param_member.name)
            elif param_member.is_static_array and "char" in param_member.type:
                param_member_contents += self.writeIndent(indent)
                param_member_contents += 'if (%s < std::strlen(%s)) {\n' % (
                    param_member.static_array_sizes[0], prefixed_param_member_name)
                indent = indent + 1
                param_member_contents += self.writeIndent(indent)
                param_member_contents += 'CoreValidLogMessage(%s, "VUID-%s-%s-parameter",\n' % (
                    instance_info_variable, struct_command_name, param_member.name)
                param_member_contents += self.writeIndent(indent)
                param_member_contents += '                    VALID_USAGE_DEBUG_SEVERITY_ERROR, %s,\n' % command_name_variable
                param_member_contents += self.writeIndent(indent)
                param_member_contents += '                    objects_info,\n'
                param_member_contents += self.writeIndent(indent)
                param_member_contents += '                    '
                if is_command:
                    param_member_contents += '"Command %s param %s' % (
                        struct_command_name, param_member.name)
                else:
                    param_member_contents += '"Structure %s member %s' % (
                        struct_command_name, param_member.name)
                param_member_contents += ' length is too long.");\n'
                param_member_contents += self.writeIndent(indent)
                param_member_contents += 'return XR_ERROR_VALIDATION_FAILURE;\n'
                indent = indent - 1
                param_member_contents += self.writeIndent(indent)
                param_member_contents += '}\n'
        if is_loop:
            indent = indent - 1
            if wrote_loop:
                param_member_contents += self.writeIndent(indent)
                param_member_contents += '}\n'

        return param_member_contents

    # Write the validation function for every struct we know about.
    #   self            the ValidationSourceOutputGenerator object
    def writeValidateStructFuncs(self):
        struct_check = ''
        # Now write out the actual functions
        for xr_struct in self.api_structures:
            if xr_struct.name in self.structs_with_no_type:
                continue

            indent = 1
            is_relation_group = False
            relation_group = None

            if xr_struct.protect_value:
                struct_check += '#if %s\n' % xr_struct.protect_string
            struct_check += 'XrResult ValidateXrStruct(GenValidUsageXrInstanceInfo *instance_info, const std::string &command_name,\n'
            struct_check += '                          std::vector<GenValidUsageXrObjectInfo>& objects_info, bool check_members,\n'
            struct_check += '                          const %s* value) {\n' % xr_struct.name
            setup_bail = False
            struct_check += '    XrResult xr_result = XR_SUCCESS;\n'

            # Check to see if this struct is the base of a relation group
            for cur_rel_group in self.struct_relation_groups:
                if cur_rel_group.generic_struct_name == xr_struct.name:
                    relation_group = cur_rel_group
                    is_relation_group = True
                    break

            # If this struct is the base of a relation group, check to see if this call really should go to any one of
            # it's children instead of itself.
            if is_relation_group:
                for member in xr_struct.members:
                    if member.name == 'next':
                        struct_check += self.writeIndent(indent)
                        struct_check += '// NOTE: Can\'t validate "VUID-%s-next-next" because it is a base structure\n' % xr_struct.name
                    else:
                        struct_check += self.writeIndent(indent)
                        struct_check += '// NOTE: Can\'t validate "VUID-%s-%s-parameter" because it is a base structure\n' % (
                            xr_struct.name, member.name)
                for child in relation_group.child_struct_names:
                    child_struct = self.getStruct(child)
                    if child_struct.protect_value:
                        struct_check += '#if %s\n' % child_struct.protect_string
                    struct_check += self.writeIndent(indent)
                    struct_check += 'if (value->type == %s) {\n' % self.genXrStructureType(
                        child)
                    indent += 1
                    struct_check += self.writeIndent(indent)
                    struct_check += 'const %s* new_value = reinterpret_cast<const %s*>(value);\n' % (
                        child, child)
                    if child_struct.ext_name and not self.isCoreExtensionName(child_struct.ext_name):
                        struct_check += self.writeIndent(indent)
                        struct_check += 'if (nullptr != instance_info && !ExtensionEnabled(instance_info->enabled_extensions, "%s")) {\n' % child_struct.ext_name
                        indent += 1
                        struct_check += self.writeIndent(indent)
                        struct_check += 'std::string error_str = "%s being used with child struct type ";\n' % xr_struct.name
                        struct_check += self.writeIndent(indent)
                        struct_check += 'error_str += "\\"%s\\"";\n' % self.genXrStructureType(
                            child)
                        struct_check += self.writeIndent(indent)
                        struct_check += 'error_str += " which requires extension \\"%s\\" to be enabled, but it is not enabled";\n' % child_struct.ext_name
                        struct_check += self.writeIndent(indent)
                        struct_check += 'CoreValidLogMessage(instance_info, "VUID-%s-type-type",\n' % (
                            xr_struct.name)
                        struct_check += self.writeIndent(indent)
                        struct_check += '                    VALID_USAGE_DEBUG_SEVERITY_ERROR, command_name,\n'
                        struct_check += self.writeIndent(indent)
                        struct_check += '                    objects_info, error_str);\n'
                        struct_check += self.writeIndent(indent)
                        struct_check += 'return XR_ERROR_VALIDATION_FAILURE;\n'
                        indent -= 1
                        struct_check += self.writeIndent(indent)
                        struct_check += '}\n'
                    struct_check += self.writeIndent(indent)
                    struct_check += 'return ValidateXrStruct(instance_info, command_name, objects_info, check_members, new_value);\n'
                    indent -= 1
                    struct_check += self.writeIndent(indent)
                    struct_check += '}\n'
                    if child_struct.protect_value:
                        struct_check += '#endif // %s\n' % child_struct.protect_string
                struct_check += self.writeIndent(indent)
                struct_check += 'std::ostringstream oss_type;\n'
                struct_check += self.writeIndent(indent)
                struct_check += 'oss_type << std::hex << value->type;\n'
                struct_check += self.writeIndent(indent)
                struct_check += 'std::string error_str = "%s has an invalid XrStructureType 0x";\n' % xr_struct.name
                struct_check += self.writeIndent(indent)
                struct_check += 'error_str += oss_type.str();\n'
                struct_check += self.writeIndent(indent)
                struct_check += 'CoreValidLogMessage(instance_info, "VUID-%s-type-type",\n' % (
                    xr_struct.name)
                struct_check += self.writeIndent(indent)
                struct_check += '                    VALID_USAGE_DEBUG_SEVERITY_ERROR, command_name,\n'
                struct_check += self.writeIndent(indent)
                struct_check += '                    objects_info, error_str);\n'
                struct_check += self.writeIndent(indent)
                struct_check += 'return XR_ERROR_VALIDATION_FAILURE;\n'
                struct_check += '}\n\n'
                continue

            first_member_handle_tuple = None
            first_member_handle = None
            count = 0
            wrote_handle_check_proto = False
            has_enable_extension_count = False
            has_enable_extension_names = False
            for member in xr_struct.members:
                # If we're not supposed to check this, then skip it
                if member.no_auto_validity:
                    continue
                if member.name == 'type':
                    struct_check += self.writeIndent(indent)
                    struct_check += '// Make sure the structure type is correct\n'
                    struct_check += self.writeIndent(indent)
                    struct_check += 'if (value->type != %s) {\n' % self.genXrStructureType(
                        xr_struct.name)
                    indent = indent + 1
                    struct_check += self.writeIndent(indent)
                    struct_check += 'std::ostringstream oss_type;\n'
                    struct_check += self.writeIndent(indent)
                    struct_check += 'oss_type << std::hex << value->type;\n'
                    struct_check += self.writeIndent(indent)
                    struct_check += 'std::string error_str = "%s has an invalid XrStructureType 0x";\n' % xr_struct.name
                    struct_check += self.writeIndent(indent)
                    struct_check += 'error_str += oss_type.str();\n'
                    struct_check += self.writeIndent(indent)
                    struct_check += 'error_str += ", expected %s";\n' % self.genXrStructureType(
                        xr_struct.name)
                    struct_check += self.writeIndent(indent)
                    struct_check += 'CoreValidLogMessage(instance_info, "VUID-%s-%s-type",\n' % (
                        xr_struct.name, member.name)
                    struct_check += self.writeIndent(indent)
                    struct_check += '                    VALID_USAGE_DEBUG_SEVERITY_ERROR, command_name,\n'
                    struct_check += self.writeIndent(indent)
                    struct_check += '                    objects_info, error_str);\n'
                    struct_check += self.writeIndent(indent)
                    struct_check += 'xr_result = XR_ERROR_VALIDATION_FAILURE;\n'
                    indent = indent - 1
                    struct_check += self.writeIndent(indent)
                    struct_check += '}\n'
                    continue
                elif member.name == 'next':
                    struct_check += self.writeValidateStructNextCheck(
                        xr_struct.name, 'value', member, indent)
                elif member.name == 'enabledExtensionCount':
                    has_enable_extension_count = True
                elif member.name == 'enabledExtensionNames':
                    has_enable_extension_names = True
                elif not setup_bail:
                    struct_check += self.writeIndent(indent)
                    struct_check += '// If we are not to check the rest of the members, just return here.\n'
                    struct_check += self.writeIndent(indent)
                    struct_check += 'if (!check_members || XR_SUCCESS != xr_result) {\n'
                    struct_check += self.writeIndent(indent + 1)
                    struct_check += 'return xr_result;\n'
                    struct_check += self.writeIndent(indent)
                    struct_check += '}\n'
                    setup_bail = True
                struct_check += self.outputParamMemberContents(False, xr_struct.name, member, 'value->',
                                                               "instance_info", "command_name",
                                                               count == 0,
                                                               first_member_handle,
                                                               first_member_handle,
                                                               first_member_handle_tuple,
                                                               wrote_handle_check_proto,
                                                               indent)
                if member.is_handle:
                    wrote_handle_check_proto = True
                count = count + 1

            # We only have extensions to check if both the count and enable fields are there
            if has_enable_extension_count and has_enable_extension_names:
                # This is create instance, so check all instance extensions
                struct_check += self.writeIndent(indent)
                struct_check += 'std::vector<std::string> enabled_extension_vec;\n'
                struct_check += self.writeIndent(indent)
                struct_check += 'for (uint32_t extension = 0; extension < value->enabledExtensionCount; ++extension) {\n'
                struct_check += self.writeIndent(indent + 1)
                struct_check += 'enabled_extension_vec.push_back(value->enabledExtensionNames[extension]);\n'
                struct_check += self.writeIndent(indent)
                struct_check += '}\n'
                if xr_struct.name == 'XrInstanceCreateInfo':
                    struct_check += self.writeIndent(indent)
                    struct_check += 'if (!ValidateInstanceExtensionDependencies(nullptr, command_name, "%s",\n' % xr_struct.name
                    struct_check += self.writeIndent(indent)
                    struct_check += '                                           objects_info, enabled_extension_vec)) {\n'
                    struct_check += self.writeIndent(indent + 1)
                    struct_check += 'return XR_ERROR_VALIDATION_FAILURE;\n'
                    struct_check += self.writeIndent(indent)
                    struct_check += '}\n'
                else:
                    struct_check += self.writeIndent(indent)
                    struct_check += 'if (!ValidateSystemExtensionDependencies(instance_info, command_name, "%s",\n' % xr_struct.name
                    struct_check += self.writeIndent(indent)
                    struct_check += '                                         objects_info, enabled_extension_vec)) {\n'
                    struct_check += self.writeIndent(indent + 1)
                    struct_check += 'return XR_ERROR_VALIDATION_FAILURE;\n'
                    struct_check += self.writeIndent(indent)
                    struct_check += '}\n'
            struct_check += self.writeIndent(indent)
            struct_check += '// Everything checked out properly\n'
            struct_check += self.writeIndent(indent)
            struct_check += 'return xr_result;\n'
            struct_check += '}\n\n'
            if xr_struct.protect_value:
                struct_check += '#endif // %s\n' % xr_struct.protect_string
        struct_check += '\n'
        return struct_check

    # Write an inline validation check for handle parents
    #   self                    the ValidationSourceOutputGenerator object
    #   instance_info_string    string used to identify the variable associated with the instance information struct.
    #   first_handle_tuple      the handle tuple associated with the type of the first handle
    #   first_handle_mem_param  the member/param of the first handle
    #   first_handle_desc_name  the descriptive name of the first handle
    #   cur_handle_tuple        the handle tuple associated with the type of the current handle
    #   cur_handle_mem_param    the member/param of the current handle
    #   cur_handle_desc_name    the descriptive name of the current handle
    #   vuid_name               the VUID identifier to associate this check and member/param name with
    #   cmd_name_param          the parameter containing the associated command name
    #   indent                  the number of tab-stops to indent the current inline strings
    def writeInlineParentCheckCall(self, instance_info_string, first_handle_tuple, first_handle_mem_param, first_handle_desc_name,
                                   cur_handle_tuple, cur_handle_mem_param, cur_handle_desc_name, vuid_name,
                                   cmd_name_param, indent):
        parent_check_string = ''
        parent_id = 'commonparent'
        if (first_handle_tuple.name == cur_handle_tuple.parent or
                cur_handle_tuple.name == first_handle_tuple.parent):
            parent_id = '%s-parent' % cur_handle_mem_param.name
        parent_check_string += self.writeIndent(indent)
        pointer_deref = ''
        if cur_handle_mem_param.pointer_count > 0:
            pointer_deref = '*'
        compare_flag = 'true'
        if first_handle_mem_param.type == cur_handle_mem_param.type:
            compare_flag = 'false'
        if cur_handle_mem_param.is_optional:
            parent_check_string += '// If the second handle is optional, only check for a common parent if\n'
            parent_check_string += self.writeIndent(indent)
            parent_check_string += '// it is not XR_NULL_HANDLE\n'
            parent_check_string += self.writeIndent(indent)
            parent_check_string += 'if (!CHECK_FOR_NULL_HANDLE(%s) && !VerifyXrParent(%s, CONVERT_HANDLE_TO_GENERIC(%s),\n' % (
                cur_handle_desc_name,
                self.genXrObjectType(first_handle_mem_param.type),
                first_handle_desc_name)
            parent_check_string += '                                                                                  %s,  CONVERT_HANDLE_TO_GENERIC(%s%s), %s)) {\n' % (
                self.genXrObjectType(cur_handle_mem_param.type),
                pointer_deref,
                cur_handle_desc_name,
                compare_flag)
        else:
            parent_check_string += '// Verify that the handles share a common ancestry\n'
            parent_check_string += self.writeIndent(indent)
            parent_check_string += 'if (!VerifyXrParent(%s,  CONVERT_HANDLE_TO_GENERIC(%s),\n' % (
                self.genXrObjectType(first_handle_mem_param.type), first_handle_desc_name)
            parent_check_string += '                             %s,  CONVERT_HANDLE_TO_GENERIC(%s%s), %s)) {\n' % (
                self.genXrObjectType(cur_handle_mem_param.type), pointer_deref, cur_handle_desc_name, compare_flag)
        indent = indent + 1
        parent_check_string += self.writeIndent(indent)
        parent_check_string += 'std::ostringstream oss_handle_1;\n'
        parent_check_string += self.writeIndent(indent)
        parent_check_string += 'oss_handle_1 << std::hex << reinterpret_cast<const void*>(%s);\n' % first_handle_desc_name
        parent_check_string += self.writeIndent(indent)
        parent_check_string += 'std::ostringstream oss_handle_2;\n'
        parent_check_string += self.writeIndent(indent)
        parent_check_string += 'oss_handle_2 << std::hex << reinterpret_cast<const void*>(%s%s);\n' % (
            pointer_deref, cur_handle_desc_name)
        parent_check_string += self.writeIndent(indent)
        parent_check_string += 'std::string error_str = "%s ";\n' % first_handle_mem_param.type
        parent_check_string += self.writeIndent(indent)
        parent_check_string += 'error_str += oss_handle_1.str();\n'
        if first_handle_tuple.name == cur_handle_tuple.parent:
            parent_check_string += self.writeIndent(indent)
            parent_check_string += 'error_str += " must be a parent to %s ";\n' % cur_handle_mem_param.type
            parent_check_string += self.writeIndent(indent)
            parent_check_string += 'error_str += oss_handle_2.str();\n'
        elif cur_handle_tuple.name == first_handle_tuple.parent:
            parent_check_string += self.writeIndent(indent)
            parent_check_string += 'error_str += " must be a child of %s ";\n' % cur_handle_mem_param.type
            parent_check_string += self.writeIndent(indent)
            parent_check_string += 'error_str += oss_handle_2.str();\n'
        else:
            parent_check_string += self.writeIndent(indent)
            parent_check_string += 'error_str += " and %s ";\n' % cur_handle_mem_param.type
            parent_check_string += self.writeIndent(indent)
            parent_check_string += 'error_str += oss_handle_2.str();\n'
            parent_check_string += self.writeIndent(indent)
            parent_check_string += 'error_str += " must share a parent";\n'
        parent_check_string += self.writeIndent(indent)
        parent_check_string += 'CoreValidLogMessage(%s, "VUID-%s-%s",\n' % (instance_info_string,
                                                                            vuid_name,
                                                                            parent_id)
        parent_check_string += self.writeIndent(indent)
        parent_check_string += '                    VALID_USAGE_DEBUG_SEVERITY_ERROR, %s,\n' % cmd_name_param
        parent_check_string += self.writeIndent(indent)
        parent_check_string += '                    objects_info, error_str);\n'
        parent_check_string += self.writeIndent(indent)
        parent_check_string += 'return XR_ERROR_VALIDATION_FAILURE;\n'
        indent = indent - 1
        parent_check_string += self.writeIndent(indent)
        parent_check_string += '}\n'
        return parent_check_string

    # Generate C++ code to validate the inputs of the current command.
    #   self            the ValidationSourceOutputGenerator object
    #   cur_command     the command generated in automatic_source_generator.py to validate
    def genValidateInputsFunc(self, cur_command):
        pre_validate_func = ''

        pre_validate_func += 'XrResult %s(' % cur_command.name.replace("xr",
                                                                       "GenValidUsageInputsXr")
        pre_validate_func += '\n'
        count = 0
        for param in cur_command.params:
            if count > 0:
                pre_validate_func += ',\n'
            pre_validate_func += '    '
            pre_validate_func += param.cdecl.strip()
            count = count + 1

        pre_validate_func += ') {\n'
        wrote_handle_check_proto = False
        is_first_param_handle = cur_command.params[0].is_handle
        first_param_handle_tuple = self.getHandle(cur_command.params[0].type)

        # If the first parameter is a handle and we either have to validate that handle, or check
        # for extension information, then we will need the instance information.
        indent = 1
        pre_validate_func += self.writeIndent(indent)
        pre_validate_func += 'try {\n'
        indent = indent + 1

        pre_validate_func += self.writeIndent(indent)
        pre_validate_func += 'XrResult xr_result = XR_SUCCESS;\n'
        pre_validate_func += self.writeIndent(indent)
        pre_validate_func += 'std::vector<GenValidUsageXrObjectInfo> objects_info;\n'
        if first_param_handle_tuple != None:
            pre_validate_func += self.writeIndent(indent)
            pre_validate_func += 'GenValidUsageXrObjectInfo handle_info = {};\n'
            pre_validate_func += self.writeIndent(indent)
            handle_param = cur_command.params[0]
            first_handle_name = self.getFirstHandleName(handle_param)
            pre_validate_func += 'handle_info.handle = CONVERT_HANDLE_TO_GENERIC(%s);\n' % first_handle_name
            pre_validate_func += self.writeIndent(indent)
            pre_validate_func += 'handle_info.type = %s;\n' % self.genXrObjectType(
                handle_param.type)
            pre_validate_func += self.writeIndent(indent)
            pre_validate_func += 'objects_info.push_back(handle_info);\n\n'
            lower_handle_name = first_param_handle_tuple.name[2:].lower()
            if first_param_handle_tuple.name == 'XrInstance':
                pre_validate_func += self.writeIndent(indent)
                pre_validate_func += 'std::unique_lock<std::mutex> mlock(g_%s_dispatch_mutex);\n' % handle_param.type[2:].lower()
                pre_validate_func += self.writeIndent(indent)
                pre_validate_func += 'GenValidUsageXrInstanceInfo *gen_instance_info = g_instance_info_map[%s];\n' % first_handle_name
                pre_validate_func += self.writeIndent(indent)
                pre_validate_func += 'mlock.unlock();\n'
            else:
                pre_validate_func += self.writeIndent(indent)
                pre_validate_func += 'GenValidUsageXrHandleInfo *gen_%s_info = ' % lower_handle_name
                pre_validate_func += 'g_%s_info_map[%s];\n' % (
                    lower_handle_name, first_handle_name)
                pre_validate_func += self.writeIndent(indent)
                pre_validate_func += 'GenValidUsageXrInstanceInfo *gen_instance_info = gen_%s_info->instance_info;\n' % lower_handle_name

        # If any of the associated handles has validation state tracking, get the
        # appropriate struct setup for validation later in the function
        valid_type_list = []
        if cur_command.checks_state:
            for cur_state in self.api_states:
                if cur_command.name in cur_state.check_commands:
                    command_param_of_type = ''
                    for param in cur_command.params:
                        if param.type == cur_state.type:
                            command_param_of_type = param.name
                            break
                    if (len(command_param_of_type) > 0) and cur_state.type not in valid_type_list:
                        valid_type_list.append(cur_state.type)
                        pre_validate_func += self.writeIndent(2)
                        pre_validate_func += 'auto %s_valid = g_%s_valid_states[%s];\n' % (
                            cur_state.type[2:].lower(), cur_state.type[2:].lower(), command_param_of_type)

            for additional_ext in cur_command.required_exts:
                pre_validate_func += self.writeIndent(indent)
                pre_validate_func += '// Check to make sure that the extension this command is in has been enabled\n'
                pre_validate_func += self.writeIndent(indent)
                pre_validate_func += 'if (!ExtensionEnabled(gen_instance_info->enabled_extensions, "%s")) {\n' % additional_ext
                pre_validate_func += self.writeIndent(indent + 1)
                pre_validate_func += 'return XR_ERROR_VALIDATION_FAILURE;\n'
                pre_validate_func += self.writeIndent(indent)
                pre_validate_func += '}\n'

        instance_info_variable = 'gen_instance_info' if first_param_handle_tuple else 'nullptr'

        # Check for non-optional null pointers
        count = 0
        for param in cur_command.params:
            # TODO use_pointer_deref never gets used?
            use_pointer_deref = False
            if len(param.array_count_var) != 0 or len(param.pointer_count_var) != 0:
                if ((len(param.array_count_var) != 0 and param.pointer_count > 0) or
                        (len(param.pointer_count_var) != 0 and param.pointer_count > 1)):
                    use_pointer_deref = True
            elif param.pointer_count > 0:
                use_pointer_deref = True
            if count > 0 and param.is_handle and not param.pointer_count > 0:
                pre_validate_func += self.writeIndent(indent)
                pre_validate_func += 'handle_info.handle = CONVERT_HANDLE_TO_GENERIC(%s);\n' % param.name
                pre_validate_func += self.writeIndent(indent)
                pre_validate_func += 'handle_info.type = %s;\n' % self.genXrObjectType(
                    param.type)
                pre_validate_func += self.writeIndent(indent)
                pre_validate_func += 'objects_info.push_back(handle_info);\n'
            if not param.no_auto_validity:
                command_name_string = '"%s"' % cur_command.name
                pre_validate_func += self.outputParamMemberContents(True, cur_command.name, param, '',
                                                                    instance_info_variable,
                                                                    command_name_string,
                                                                    count == 0,
                                                                    cur_command.params[0],
                                                                    cur_command.params[0].name,
                                                                    first_param_handle_tuple,
                                                                    wrote_handle_check_proto,
                                                                    indent)
                wrote_handle_check_proto = True
            count = count + 1

        base_handle_name = cur_command.params[0].type[2:].lower()

        # If this command needs to be checked to ensure that it is executing between
        # a "begin" and an "end" command, do so.
        if cur_command.checks_state:
            for cur_state in self.api_states:
                if cur_command.name in cur_state.check_commands:
                    for param in cur_command.params:
                        if param.type == cur_state.type:
                            break
                    pre_validate_func += self.writeIndent(2)
                    pre_validate_func += '// Validate that this command is called at the proper time between the\n'
                    pre_validate_func += self.writeIndent(2)
                    pre_validate_func += '// appropriate commands\n'
                    pre_validate_func += self.writeIndent(2)
                    pre_validate_func += 'if (!%s_valid->%s) {\n' % (
                        cur_state.type[2:].lower(), cur_state.variable)
                    pre_validate_func += self.writeIndent(3)
                    pre_validate_func += 'std::string error_msg = "%s is required to be called between successful calls to ";\n' % cur_command.name
                    pre_validate_func += self.writeIndent(3)
                    pre_validate_func += 'error_msg += "'
                    cur_count = 0
                    for begin_command in cur_state.begin_commands:
                        if cur_count > 0:
                            pre_validate_func += '/'
                        cur_count += 1
                        pre_validate_func += '%s' % begin_command
                    pre_validate_func += ' and '
                    cur_count = 0
                    for end_command in cur_state.end_commands:
                        if cur_count > 0:
                            pre_validate_func += '/'
                        cur_count += 1
                        pre_validate_func += '%s' % end_command
                    pre_validate_func += ' commands";\n'
                    pre_validate_func += self.writeIndent(3)
                    pre_validate_func += 'CoreValidLogMessage(%s, "VUID-%s-%s-checkstate",\n' % (
                        instance_info_variable, cur_command.name, cur_state.state)
                    pre_validate_func += self.writeIndent(3)
                    pre_validate_func += '                    VALID_USAGE_DEBUG_SEVERITY_ERROR, "%s", objects_info,\n' % cur_command.name
                    pre_validate_func += self.writeIndent(3)
                    pre_validate_func += '                    error_msg);\n'
                    pre_validate_func += self.writeIndent(3)
                    pre_validate_func += 'return XR_ERROR_VALIDATION_FAILURE;\n'
                    pre_validate_func += self.writeIndent(2)
                    pre_validate_func += '}\n'

        # If this command needs to indicate that a validation state has begun, do so.
        if cur_command.begins_state:
            for cur_state in self.api_states:
                if cur_command.name in cur_state.check_commands:
                    for param in cur_command.params:
                        if param.type == cur_state.type:
                            break
                    # First, make sure we're not calling two (or more) "begins" in a row
                    pre_validate_func += self.writeIndent(2)
                    pre_validate_func += '// Validate that this command is called first or only after the corresponding\n'
                    pre_validate_func += self.writeIndent(2)
                    pre_validate_func += '// "completion" commands\n'
                    pre_validate_func += self.writeIndent(2)
                    pre_validate_func += 'if (%s_valid->%s) {\n' % (
                        cur_state.type[2:].lower(), cur_state.variable)
                    pre_validate_func += self.writeIndent(3)
                    pre_validate_func += 'std::string error_msg = "%s is called again without first successfully calling ";\n' % cur_command.name
                    pre_validate_func += self.writeIndent(3)
                    pre_validate_func += 'error_msg += "'
                    cur_count = 0
                    for end_command in cur_state.end_commands:
                        if cur_count > 0:
                            pre_validate_func += '/'
                        cur_count += 1
                        pre_validate_func += '%s' % end_command
                    pre_validate_func += '";\n'
                    pre_validate_func += self.writeIndent(3)
                    pre_validate_func += 'CoreValidLogMessage(%s, "VUID-%s-%s-beginstate",\n' % (
                        instance_info_variable, cur_command.name, cur_state.state)
                    pre_validate_func += self.writeIndent(3)
                    pre_validate_func += '                    VALID_USAGE_DEBUG_SEVERITY_ERROR, "%s", objects_info,\n' % cur_command.name
                    pre_validate_func += self.writeIndent(3)
                    pre_validate_func += '                    error_msg);\n'
                    pre_validate_func += self.writeIndent(3)
                    pre_validate_func += 'return XR_ERROR_VALIDATION_FAILURE;\n'
                    pre_validate_func += self.writeIndent(2)
                    pre_validate_func += '}\n'
                    # Begin the appropriate state
                    pre_validate_func += self.writeIndent(2)
                    pre_validate_func += '// Begin the %s state\n' % cur_state.state
                    pre_validate_func += self.writeIndent(2)
                    pre_validate_func += '%s_valid->%s = true;\n' % (
                        cur_state.type[2:].lower(), cur_state.variable)

        # If this command needs to indicate an end of a validation state, do so.
        if cur_command.ends_state:
            for cur_state in self.api_states:
                if cur_command.name in cur_state.check_commands:
                    for param in cur_command.params:
                        if param.type == cur_state.type:
                            break
                    # First, make sure we're not calling two (or more) "ends" in a row (or before a "begin")
                    pre_validate_func += self.writeIndent(2)
                    pre_validate_func += '// Validate that this command is called after the corresponding\n'
                    pre_validate_func += self.writeIndent(2)
                    pre_validate_func += '// "begin" commands\n'
                    pre_validate_func += self.writeIndent(2)
                    pre_validate_func += 'if (!%s_valid->%s) {\n' % (
                        cur_state.type[2:].lower(), cur_state.variable)
                    pre_validate_func += self.writeIndent(3)
                    pre_validate_func += 'std::string error_msg = "%s is called again without first successfully calling ";\n' % cur_command.name
                    pre_validate_func += self.writeIndent(3)
                    pre_validate_func += 'error_msg += "'
                    cur_count = 0
                    for begin_command in cur_state.begin_commands:
                        if cur_count > 0:
                            pre_validate_func += '/'
                        cur_count += 1
                        pre_validate_func += '%s' % begin_command
                    pre_validate_func += '";\n'
                    pre_validate_func += self.writeIndent(3)
                    pre_validate_func += 'CoreValidLogMessage(%s, "VUID-%s-%s-endstate",\n' % (
                        instance_info_variable, cur_command.name, cur_state.state)
                    pre_validate_func += self.writeIndent(3)
                    pre_validate_func += '                    VALID_USAGE_DEBUG_SEVERITY_ERROR, "%s", objects_info,\n' % cur_command.name
                    pre_validate_func += self.writeIndent(3)
                    pre_validate_func += '                    error_msg);\n'
                    pre_validate_func += self.writeIndent(3)
                    pre_validate_func += 'return XR_ERROR_VALIDATION_FAILURE;\n'
                    pre_validate_func += self.writeIndent(2)
                    pre_validate_func += '}\n'
                    # End the appropriate state
                    pre_validate_func += self.writeIndent(2)
                    pre_validate_func += '// End the %s state\n' % cur_state.state
                    pre_validate_func += self.writeIndent(2)
                    pre_validate_func += '%s_valid->%s = false;\n' % (
                        cur_state.type[2:].lower(), cur_state.variable)

        pre_validate_func += self.writeIndent(indent)
        pre_validate_func += 'return XR_SUCCESS;\n'
        indent = indent - 1
        pre_validate_func += self.writeIndent(indent)
        pre_validate_func += '} catch (...) {\n'
        pre_validate_func += self.writeIndent(indent + 1)
        pre_validate_func += 'return XR_ERROR_VALIDATION_FAILURE;\n'
        pre_validate_func += self.writeIndent(indent)
        pre_validate_func += '}\n'
        pre_validate_func += '}\n\n'
        return pre_validate_func

    # Generate C++ code to call down to the next layer/loader terminator/runtime
    #   self            the ValidationSourceOutputGenerator object
    #   cur_command     the command generated in automatic_source_generator.py to validate
    #   has_return      Boolean indicating that the command must return a value (usually XrResult)
    #   is_create       Boolean indicating that the command is a create command
    #   is_destroy      Boolean indicating that the command is a destroy command
    #   is_sempath_query Boolean indicating that this is a semantic path query (treat similar to a create)
    def genNextValidateFunc(self, cur_command, has_return, is_create, is_destroy, is_sempath_query):
        next_validate_func = ''
        # Note: We don't make a "next" call for xrCreateInstance in a layer because we
        # actually have to call xrCreateApiLayerInstance.  Also, we have to setup the first
        # entry into the dispatch table so it's a special case all around.
        if 'xrCreateInstance' in cur_command.name:
            return ''
        prototype = cur_command.cdecl
        prototype = prototype.replace(" xr", " GenValidUsageNextXr")
        prototype = prototype.replace("API_ATTR ", "")
        prototype = prototype.replace("XRAPI_CALL ", "")
        prototype = prototype.replace(";", " {")
        next_validate_func += '%s\n' % (prototype)
        if has_return:
            return_prefix = '    '
            return_prefix += cur_command.return_type.text
            return_prefix += ' result'
            if cur_command.return_type.text == 'XrResult':
                return_prefix += ' = XR_SUCCESS;\n'
            else:
                return_prefix += ';\n'
            next_validate_func += return_prefix

        next_validate_func += '    try {\n'

        # Next, we have to call down to the next implementation of this command in the call chain.
        # Before we can do that, we have to figure out what the dispatch table is
        base_handle_name = cur_command.params[0].type[2:].lower()
        if cur_command.params[0].is_handle:
            handle_tuple = self.getHandle(cur_command.params[0].type)
            first_handle_name = self.getFirstHandleName(cur_command.params[0])
            next_validate_func += '        std::unique_lock<std::mutex> dispatch_lock(g_%s_dispatch_mutex);\n' % base_handle_name
            if handle_tuple.name == 'XrInstance':
                next_validate_func += '        GenValidUsageXrInstanceInfo *gen_instance_info = g_instance_info_map[%s];\n' % first_handle_name
            else:
                next_validate_func += '        GenValidUsageXrHandleInfo *gen_%s_info = ' % base_handle_name
                next_validate_func += 'g_%s_info_map[%s];\n' % (
                    base_handle_name, first_handle_name)
                next_validate_func += '        GenValidUsageXrInstanceInfo *gen_instance_info = gen_%s_info->instance_info;\n' % base_handle_name
            next_validate_func += '        dispatch_lock.unlock();\n'
        else:
            next_validate_func += '#error("Bug")\n'
        # Call down, looking for the returned result if required.
        next_validate_func += '        '
        if has_return:
            next_validate_func += 'result = '
        next_validate_func += 'gen_instance_info->dispatch_table->%s(' % cur_command.name[2:]

        count = 0
        for param in cur_command.params:
            if count > 0:
                next_validate_func += ', '
            next_validate_func += param.name
            count = count + 1
        next_validate_func += ');\n'

        # If this is a create command, we have to create an entry in the appropriate
        # unordered_map pointing to the correct dispatch table for the newly created
        # object.  Likewise, if it's a delete command, we have to remove the entry
        # for the dispatch table from the unordered_map
        last_name = ''
        last_lower_type = ''
        if cur_command.params[-1].is_handle:
            last_handle_tuple = self.getHandle(cur_command.params[-1].type)
            last_lower_type = last_handle_tuple.name[2:].lower()
            last_name = cur_command.params[-1].name
            if is_create:
                next_validate_func += '        if (XR_SUCCESS == result && nullptr != %s) {\n' % last_name
                next_validate_func += '            std::unique_lock<std::mutex> info_lock(g_%s_dispatch_mutex);\n' % last_lower_type
                next_validate_func += '            auto exists = g_%s_info_map.find(*%s);\n' % (
                    last_lower_type, last_name)
                next_validate_func += '            if (exists == g_%s_info_map.end()) {\n' % last_lower_type
                if last_handle_tuple.name == 'XrInstance':
                    next_validate_func += '                g_%s_info_map[*%s] = gen_instance_info;\n' % (
                        last_lower_type, last_name)
                else:
                    next_validate_func += '                GenValidUsageXrHandleInfo *handle_info = new GenValidUsageXrHandleInfo();\n'
                    next_validate_func += '                handle_info->instance_info = gen_instance_info;\n'
                    next_validate_func += '                handle_info->direct_parent_type = %s;\n' % self.genXrObjectType(
                        cur_command.params[0].type)
                    next_validate_func += '                handle_info->direct_parent_handle = CONVERT_HANDLE_TO_GENERIC(%s);\n' % cur_command.params[
                        0].name
                    next_validate_func += '                g_%s_info_map[*%s] = handle_info;\n' % (last_lower_type, last_name)

                # If this object contains a state that needs tracking, allocate it
                valid_type_list = []
                for cur_state in self.api_states:
                    if last_handle_tuple.name == cur_state.type and cur_state.type not in valid_type_list:
                        valid_type_list.append(cur_state.type)
                        next_validate_func += self.writeIndent(4)
                        next_validate_func += '// Check to see if this object that has been created has a validation\n'
                        next_validate_func += self.writeIndent(4)
                        next_validate_func += '// state structure that needs to be created as well.\n'
                        next_validate_func += self.writeIndent(4)
                        next_validate_func += '%sValidationStates *%s_valid_state = new %sValidationStates;\n' % (
                            cur_state.type, cur_state.type[2:].lower(), cur_state.type)
                        next_validate_func += self.writeIndent(4)
                        next_validate_func += '(*%s_valid_state) = {};\n' % cur_state.type[2:].lower(
                        )
                        next_validate_func += self.writeIndent(4)
                        next_validate_func += 'g_%s_valid_states[(*%s)] = %s_valid_state;\n' % (
                            cur_state.type[2:].lower(), last_name, cur_state.type[2:].lower())

                next_validate_func += '            }\n'
                next_validate_func += '        }\n'
            elif is_destroy:
                if cur_command.params[-1].type == 'XrSession':
                    next_validate_func += '\n        // Clean up any labels associated with this session\n'
                    next_validate_func += '        CoreValidationDeleteSessionLabels(session);\n\n'
                next_validate_func += '        std::unique_lock<std::mutex> info_lock(g_%s_dispatch_mutex);\n' % last_lower_type
                next_validate_func += '        auto exists = g_%s_info_map.find(%s);\n' % (
                    last_lower_type, last_name)
                # Only remove the handle from our map if the runtime returned success
                next_validate_func += '        if (XR_SUCCEEDED(result) && (exists != g_%s_info_map.end())) {\n' % last_lower_type

                # If this object contains a state that needs tracking, free it
                valid_type_list = []
                for cur_state in self.api_states:
                    if last_handle_tuple.name == cur_state.type and cur_state.type not in valid_type_list:
                        valid_type_list.append(cur_state.type)
                        next_validate_func += self.writeIndent(3)
                        next_validate_func += '// Check to see if this object that is about to be destroyed has a\n'
                        next_validate_func += self.writeIndent(3)
                        next_validate_func += '// validation state structure that needs to be cleaned up.\n'
                        next_validate_func += self.writeIndent(3)
                        next_validate_func += '%sValidationStates *%s_valid_state = g_%s_valid_states[%s];\n' % (
                            cur_state.type, cur_state.type[2:].lower(), cur_state.type[2:].lower(), last_name)
                        next_validate_func += self.writeIndent(3)
                        next_validate_func += 'if (nullptr != %s_valid_state) {\n' % cur_state.type[2:].lower(
                        )
                        next_validate_func += self.writeIndent(4)
                        next_validate_func += 'delete %s_valid_state;\n' % cur_state.type[2:].lower(
                        )
                        next_validate_func += self.writeIndent(4)
                        next_validate_func += 'g_%s_valid_states.erase(%s);\n' % (
                            cur_state.type[2:].lower(), last_name)
                        next_validate_func += self.writeIndent(3)
                        next_validate_func += '}\n'

                if last_handle_tuple.name != 'XrInstance':
                    next_validate_func += '            GenValidUsageXrHandleInfo *handle_info = g_%s_info_map[%s];\n' % (
                        last_lower_type, last_name)
                next_validate_func += '            g_%s_info_map.erase(%s);\n' % (last_lower_type, last_name)
                if last_handle_tuple.name != 'XrInstance':
                    next_validate_func += '            delete handle_info;\n'
                next_validate_func += '        }\n'
                if 'xrDestroyInstance' in cur_command.name:
                    next_validate_func += '        GenValidUsageCleanUpMaps(gen_instance_info);\n'
            elif is_sempath_query:
                # xrEnumeratePhysicalDevices is a special case.  It's kind of like a create in that we need to track
                # these items, but it's also an array of items.
                next_validate_func += '        if (XR_SUCCESS == result && nullptr != %s && nullptr != %s) {\n' % (cur_command.params[-2].name,
                                                                                                                   last_name)
                next_validate_func += '            std::unique_lock<std::mutex> info_lock(g_%s_dispatch_mutex);\n' % last_lower_type
                next_validate_func += '            for (uint32_t sempath = 0; sempath < *%s; ++sempath) {\n' % cur_command.params[-2].name
                next_validate_func += '                auto exists = g_%s_info_map.find(%s[sempath]);\n' % (
                    last_lower_type, cur_command.params[-1].name)
                next_validate_func += '                if (exists != g_%s_info_map.end()) {\n' % last_lower_type
                next_validate_func += '                    GenValidUsageXrHandleInfo *handle_info = new GenValidUsageXrHandleInfo();\n'
                next_validate_func += '                    handle_info->instance_info = gen_instance_info;\n'
                next_validate_func += '                handle_info->direct_parent_type = %s;\n' % self.genXrObjectType(
                    cur_command.params[0].type)
                next_validate_func += '                handle_info->direct_parent_handle = CONVERT_HANDLE_TO_GENERIC(%s);\n' % cur_command.params[0].name
                next_validate_func += '                    g_%s_info_map[%s[sempath]] = handle_info;\n' % (
                    last_lower_type, last_name)
                next_validate_func += '                }\n'
                next_validate_func += '            }\n'
                next_validate_func += '        }\n'

        # Catch any exceptions that may have occurred.  If any occurred between any of the
        # valid mutex lock/unlock statements, perform the unlock now.  Notice that a create can
        # also allocate items, so we want to special case catching the failure of the allocation.
        if is_create or is_sempath_query:
            next_validate_func += '    } catch (std::bad_alloc&) {\n'
            next_validate_func += '        result = XR_ERROR_OUT_OF_MEMORY;\n'
        next_validate_func += '    } catch (...) {\n'
        if has_return:
            next_validate_func += '        result = XR_ERROR_VALIDATION_FAILURE;\n'
        next_validate_func += '    }\n'
        if has_return:
            next_validate_func += '    return result;\n'
        next_validate_func += '}\n\n'
        return next_validate_func

    # Generate a top-level automatic C++ validation function which will be used until
    # a manual function is defined.
    #   self            the ValidationSourceOutputGenerator object
    #   cur_command     the command generated in automatic_source_generator.py to validate
    #   has_return      Boolean indicating that the command must return a value (usually XrResult)
    def genAutoValidateFunc(self, cur_command, has_return):
        auto_validate_func = ''
        prototype = cur_command.cdecl
        prototype = prototype.replace(" xr", " GenValidUsageXr")
        prototype = prototype.replace("API_ATTR ", "")
        prototype = prototype.replace("XRAPI_CALL ", "")
        prototype = prototype.replace(";", " {")
        auto_validate_func += '%s\n' % (prototype)
        auto_validate_func += self.writeIndent(1)
        if has_return:
            auto_validate_func += '%s test_result = ' % cur_command.return_type.text
        # Define the pre-validate call
        auto_validate_func += '%s(' % cur_command.name.replace("xr",
                                                               "GenValidUsageInputsXr")
        count = 0
        for param in cur_command.params:
            if count > 0:
                auto_validate_func += ', '
            count = count + 1
            auto_validate_func += param.name
        auto_validate_func += ');\n'
        if has_return and cur_command.return_type.text == 'XrResult':
            auto_validate_func += self.writeIndent(1)
            auto_validate_func += 'if (XR_SUCCESS != test_result) {\n'
            auto_validate_func += self.writeIndent(2)
            auto_validate_func += 'return test_result;\n'
            auto_validate_func += self.writeIndent(1)
            auto_validate_func += '}\n'
        # Make the calldown to the next layer
        auto_validate_func += self.writeIndent(1)
        if has_return:
            auto_validate_func += 'return '
        auto_validate_func += '%s(' % cur_command.name.replace("xr",
                                                               "GenValidUsageNextXr")
        count = 0
        for param in cur_command.params:
            if count > 0:
                auto_validate_func += ', '
            count = count + 1
            auto_validate_func += param.name
        auto_validate_func += ');\n'
        auto_validate_func += '}\n\n'
        return auto_validate_func

    # Implementation for generated validation commands
    #   self                the ValidationSourceOutputGenerator object
    def outputValidationSourceFuncs(self):
        commands = []
        validation_source_funcs = ''
        cur_extension_name = ''

        # First, output the mapping and mutex items
        validation_source_funcs += '// Unordered Map associating pointer to a vector of session label information to a session\'s handle\n'
        validation_source_funcs += 'std::unordered_map<XrSession, std::vector<GenValidUsageXrInternalSessionLabel*>*> g_xr_session_labels;\n\n'
        for handle in self.api_handles:
            base_handle_name = handle.name[2:].lower()
            if handle.protect_value:
                validation_source_funcs += '#if %s\n' % handle.protect_string
            if handle.name == 'XrInstance':
                validation_source_funcs += 'std::unordered_map<%s, GenValidUsageXrInstanceInfo*> g_%s_info_map;\n' % (
                    handle.name, base_handle_name)
            else:
                validation_source_funcs += 'std::unordered_map<%s, GenValidUsageXrHandleInfo*> g_%s_info_map;\n' % (
                    handle.name, base_handle_name)
            validation_source_funcs += 'std::mutex g_%s_dispatch_mutex;\n' % base_handle_name
            if handle.protect_value:
                validation_source_funcs += '#endif // %s\n' % handle.protect_string
        validation_source_funcs += '\n'
        validation_source_funcs += self.outputValidationInternalProtos()
        validation_source_funcs += '// Template function to reduce duplicating the map locking, searching, and deleting.`\n'
        validation_source_funcs += 'void EraseAllInstanceTableMapElements(GenValidUsageXrInstanceInfo *search_value) {\n'
        validation_source_funcs += '    for (auto it = g_instance_info_map.begin(); it != g_instance_info_map.end();) {\n'
        validation_source_funcs += '        if (it->second == search_value) {\n'
        validation_source_funcs += '            g_instance_info_map.erase(it++);\n'
        validation_source_funcs += '        } else {\n'
        validation_source_funcs += '            ++it;\n'
        validation_source_funcs += '        }\n'
        validation_source_funcs += '    }\n'
        validation_source_funcs += '}\n'
        validation_source_funcs += '\n'
        validation_source_funcs += '// Template function to reduce duplicating the map locking, searching, and deleting.`\n'
        validation_source_funcs += 'template <typename MapType>\n'
        validation_source_funcs += 'void EraseAllTableMapElements(MapType &search_map, std::mutex &mutex, GenValidUsageXrInstanceInfo *search_value) {\n'
        validation_source_funcs += '    std::unique_lock<std::mutex> mlock(mutex);\n'
        validation_source_funcs += '    for (auto it = search_map.begin(); it != search_map.end();) {\n'
        validation_source_funcs += '        GenValidUsageXrHandleInfo *map_handle_info = it->second;\n'
        validation_source_funcs += '        if (nullptr != map_handle_info && map_handle_info->instance_info == search_value) {\n'
        validation_source_funcs += '            search_map.erase(it++);\n'
        validation_source_funcs += '            delete map_handle_info;\n'
        validation_source_funcs += '        } else {\n'
        validation_source_funcs += '            ++it;\n'
        validation_source_funcs += '        }\n'
        validation_source_funcs += '    }\n'
        validation_source_funcs += '}\n'
        validation_source_funcs += '\n'
        validation_source_funcs += '// Function used to clean up any residual map values that point to an instance prior to that\n'
        validation_source_funcs += '// instance being deleted.\n'
        validation_source_funcs += 'void GenValidUsageCleanUpMaps(GenValidUsageXrInstanceInfo *instance_info) {\n'
        for handle in self.api_handles:
            base_handle_name = handle.name[2:].lower()
            if handle.protect_value:
                validation_source_funcs += '#if %s\n' % handle.protect_string
            if handle.name == 'XrInstance':
                validation_source_funcs += '    EraseAllInstanceTableMapElements(instance_info);\n'
            else:
                validation_source_funcs += '    EraseAllTableMapElements<std::unordered_map<%s, GenValidUsageXrHandleInfo*>>(g_%s_info_map, ' % (
                    handle.name, base_handle_name)
                validation_source_funcs += 'g_%s_dispatch_mutex, instance_info);\n' % base_handle_name
            if handle.protect_value:
                validation_source_funcs += '#endif // %s\n' % handle.protect_string
        validation_source_funcs += '}\n'
        validation_source_funcs += '\n'
        validation_source_funcs += '// Function to convert XrObjectType to string\n'
        validation_source_funcs += 'std::string GenValidUsageXrObjectTypeToString(const XrObjectType& type) {\n'
        validation_source_funcs += '    std::string object_string;\n'
        count = 0
        for object_type in self.api_object_types:
            object_string = object_type.name.replace("XR_OBJECT_TYPE_", "")
            object_string = object_string.replace("_", "")
            if object_string == "UNKNOWN":
                if count == 0:
                    validation_source_funcs += '    if '
                else:
                    validation_source_funcs += '    } else if '
                validation_source_funcs += '(type == XR_OBJECT_TYPE_UNKNOWN) {\n'
                validation_source_funcs += '        object_string = "Unknown XR Object";\n'
            else:
                for handle in self.api_handles:
                    handle_name = handle.name[2:].upper()
                    if handle_name != object_string:
                        continue
                    if object_type.protect_value:
                        validation_source_funcs += '#if %s\n' % object_type.protect_string
                    if count == 0:
                        validation_source_funcs += '    if '
                    else:
                        validation_source_funcs += '    } else if '
                    validation_source_funcs += '(type == %s) {\n' % object_type.name
                    validation_source_funcs += '        object_string = "%s";\n' % handle.name
                    if object_type.protect_value:
                        validation_source_funcs += '#endif // %s\n' % object_type.protect_string
            count = count + 1
        validation_source_funcs += '    }\n'
        validation_source_funcs += '    return object_string;\n'
        validation_source_funcs += '}\n\n'
        validation_source_funcs += self.outputValidationStateCheckStructs()
        validation_source_funcs += self.outputValidationSourceNextChainProtos()
        validation_source_funcs += self.outputValidationSourceFlagBitValues()
        validation_source_funcs += self.outputValidationSourceEnumValues()
        validation_source_funcs += self.writeVerifyExtensions()
        validation_source_funcs += self.writeValidateHandleChecks()
        validation_source_funcs += self.writeValidateHandleParent()
        validation_source_funcs += self.writeValidateStructFuncs()
        validation_source_funcs += self.outputValidationSourceNextChainFunc()

        for x in range(0, 2):
            if x == 0:
                commands = self.core_commands
            else:
                commands = self.ext_commands

            for cur_cmd in commands:
                if cur_cmd.ext_name != cur_extension_name:
                    if 'XR_VERSION_' in cur_cmd.ext_name:
                        validation_source_funcs += '\n// ---- Core %s commands\n' % cur_cmd.ext_name[11:].replace(
                            "_", ".")
                    else:
                        validation_source_funcs += '\n// ---- %s extension commands\n' % cur_cmd.ext_name
                    cur_extension_name = cur_cmd.ext_name

                if cur_cmd.name in VALID_USAGE_DONT_GEN:
                    continue

                # We fill in the GetInstanceProcAddr manually at the end
                if cur_cmd.name == 'xrGetInstanceProcAddr':
                    continue

                if cur_cmd.protect_value:
                    validation_source_funcs += '#if %s\n' % cur_cmd.protect_string
                    validation_source_funcs += '\n'

                is_create = False
                is_destroy = False
                has_return = False
                is_sempath_query = False

                if ('xrCreate' in cur_cmd.name or 'xrConnect' in cur_cmd.name) and cur_cmd.params[-1].is_handle:
                    is_create = True
                    has_return = True
                elif ('xrDestroy' in cur_cmd.name or 'xrDisconnect' in cur_cmd.name) and cur_cmd.params[-1].is_handle:
                    is_destroy = True
                    has_return = True
                elif (cur_cmd.return_type != None):
                    has_return = True

                validation_source_funcs += self.genValidateInputsFunc(cur_cmd)
                validation_source_funcs += self.genNextValidateFunc(
                    cur_cmd, has_return, is_create, is_destroy, is_sempath_query)
                if not cur_cmd.name in VALID_USAGE_MANUALLY_DEFINED:
                    validation_source_funcs += self.genAutoValidateFunc(
                        cur_cmd, has_return)

                if cur_cmd.protect_value:
                    validation_source_funcs += '#endif // %s\n' % cur_cmd.protect_string
                    validation_source_funcs += '\n'

        validation_source_funcs += '\n// API Layer\'s xrGetInstanceProcAddr\n'
        validation_source_funcs += 'XrResult GenValidUsageXrGetInstanceProcAddr(\n'
        validation_source_funcs += '    XrInstance          instance,\n'
        validation_source_funcs += '    const char*         name,\n'
        validation_source_funcs += '    PFN_xrVoidFunction* function) {\n'
        validation_source_funcs += '    try {\n'
        validation_source_funcs += '        std::string func_name = name;\n'
        validation_source_funcs += '        std::vector<GenValidUsageXrObjectInfo> objects;\n'
        validation_source_funcs += '        if (!CHECK_FOR_NULL_HANDLE(instance)) {\n'
        validation_source_funcs += '            std::unique_lock<std::mutex> mlock(g_instance_dispatch_mutex);\n'
        validation_source_funcs += '            // Make sure the instance is valid if it is not XR_NULL_HANDLE\n'
        validation_source_funcs += '            if (nullptr == g_instance_info_map[instance]) {\n'
        validation_source_funcs += '                std::vector<GenValidUsageXrObjectInfo> objects;\n'
        validation_source_funcs += '                objects.resize(1);\n'
        validation_source_funcs += '                objects[0].handle = CONVERT_HANDLE_TO_GENERIC(instance);\n'
        validation_source_funcs += '                objects[0].type = XR_OBJECT_TYPE_INSTANCE;\n'
        validation_source_funcs += '                CoreValidLogMessage(nullptr, "VUID-xrGetInstanceProcAddr-instance-parameter",\n'
        validation_source_funcs += '                                    VALID_USAGE_DEBUG_SEVERITY_ERROR, "xrGetInstanceProcAddr", objects,\n'
        validation_source_funcs += '                                    "Invalid instance handle provided.");\n'
        validation_source_funcs += '            }\n'
        validation_source_funcs += '        }\n'
        validation_source_funcs += '        // NOTE: Can\'t validate "VUID-xrGetInstanceProcAddr-name-parameter" null-termination\n'
        validation_source_funcs += '        // If we setup the function, just return\n'
        validation_source_funcs += '        if (function == nullptr) {\n'
        validation_source_funcs += '            CoreValidLogMessage(nullptr, "VUID-xrGetInstanceProcAddr-function-parameter",\n'
        validation_source_funcs += '                                VALID_USAGE_DEBUG_SEVERITY_ERROR, "xrGetInstanceProcAddr", objects,\n'
        validation_source_funcs += '                                "function is NULL");\n'
        validation_source_funcs += '            return XR_ERROR_VALIDATION_FAILURE;\n'
        validation_source_funcs += '        }\n'

        count = 0
        for x in range(0, 2):
            if x == 0:
                commands = self.core_commands
            else:
                commands = self.ext_commands

            for cur_cmd in commands:
                if cur_cmd.ext_name != cur_extension_name:
                    if 'XR_VERSION_' in cur_cmd.ext_name:
                        validation_source_funcs += '\n        // ---- Core %s commands\n' % cur_cmd.ext_name[11:].replace(
                            "_", ".")
                    else:
                        validation_source_funcs += '\n        // ---- %s extension commands\n' % cur_cmd.ext_name
                    cur_extension_name = cur_cmd.ext_name

                if cur_cmd.name in VALID_USAGE_DONT_GEN:
                    continue

                has_return = False
                if (cur_cmd.return_type != None):
                    has_return = True

                if cur_cmd.name in VALID_USAGE_MANUALLY_DEFINED:
                    # Remove 'xr' from proto name and use manual name
                    layer_command_name = cur_cmd.name.replace(
                        "xr", "CoreValidationXr")
                else:
                    # Remove 'xr' from proto name and use generated name
                    layer_command_name = cur_cmd.name.replace(
                        "xr", "GenValidUsageXr")

                if cur_cmd.protect_value:
                    validation_source_funcs += '#if %s\n' % cur_cmd.protect_string

                if count == 0:
                    validation_source_funcs += '        if (func_name == "%s") {\n' % cur_cmd.name
                else:
                    validation_source_funcs += '        } else if (func_name == "%s") {\n' % cur_cmd.name
                count = count + 1

                validation_source_funcs += '            *function = reinterpret_cast<PFN_xrVoidFunction>(%s);\n' % layer_command_name
                if cur_cmd.protect_value:
                    validation_source_funcs += '#endif // %s\n' % cur_cmd.protect_string

        validation_source_funcs += '        }\n'
        validation_source_funcs += '        // If we setup the function, just return\n'
        validation_source_funcs += '        if (*function != nullptr) {\n'
        validation_source_funcs += '            return XR_SUCCESS;\n'
        validation_source_funcs += '        }\n'
        validation_source_funcs += '        // We have not found it, so pass it down to the next layer/runtime\n'
        validation_source_funcs += '        std::unique_lock<std::mutex> mlock(g_instance_dispatch_mutex);\n'
        validation_source_funcs += '        GenValidUsageXrInstanceInfo* instance_valid_usage_info = g_instance_info_map[instance];\n'
        validation_source_funcs += '        mlock.unlock();\n'
        validation_source_funcs += '        if (nullptr == instance_valid_usage_info) {\n'
        validation_source_funcs += '            return XR_ERROR_HANDLE_INVALID;\n'
        validation_source_funcs += '        }\n'
        validation_source_funcs += '        return instance_valid_usage_info->dispatch_table->GetInstanceProcAddr(instance, name, function);\n'
        validation_source_funcs += '    } catch (...) {\n'
        validation_source_funcs += '        return XR_ERROR_VALIDATION_FAILURE;\n'
        validation_source_funcs += '    }\n'
        validation_source_funcs += '}\n'
        return validation_source_funcs
