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

import os,re,sys
from generator import *

# CGeneratorOptions - subclass of GeneratorOptions.
#
# Adds options used by COutputGenerator objects during C language header
# generation.
#
# Additional members
#   prefixText - list of strings to prefix generated header with
#     (usually a copyright statement + calling convention macros).
#   protectFile - True if multiple inclusion protection should be
#     generated (based on the filename) around the entire header.
#   protectFeature - True if #ifndef..#endif protection should be
#     generated around a feature interface in the header file.
#   genFuncPointers - True if function pointer typedefs should be
#     generated
#   protectProto - If conditional protection should be generated
#     around prototype declarations, set to either '#ifdef'
#     to require opt-in (#ifdef protectProtoStr) or '#ifndef'
#     to require opt-out (#ifndef protectProtoStr). Otherwise
#     set to None.
#   protectProtoStr - #ifdef/#ifndef symbol to use around prototype
#     declarations, if protectProto is set
#   apicall - string to use for the function declaration prefix,
#     such as APICALL on Windows.
#   apientry - string to use for the calling convention macro,
#     in typedefs, such as APIENTRY.
#   apientryp - string to use for the calling convention macro
#     in function pointer typedefs, such as APIENTRYP.
#   indentFuncProto - True if prototype declarations should put each
#     parameter on a separate line
#   indentFuncPointer - True if typedefed function pointers should put each
#     parameter on a separate line
#   alignFuncParam - if nonzero and parameters are being put on a
#     separate line, align parameter names at the specified column
class CGeneratorOptions(GeneratorOptions):
    """Represents options during C interface generation for headers"""

    def __init__(self,
                 filename = None,
                 directory = '.',
                 apiname = None,
                 profile = None,
                 versions = '.*',
                 emitversions = '.*',
                 defaultExtensions = None,
                 addExtensions = None,
                 removeExtensions = None,
                 emitExtensions = None,
                 sortProcedure = regSortFeatures,
                 prefixText = "",
                 genFuncPointers = True,
                 protectFile = True,
                 protectFeature = True,
                 protectProto = None,
                 protectProtoStr = None,
                 apicall = '',
                 apientry = '',
                 apientryp = '',
                 indentFuncProto = True,
                 indentFuncPointer = False,
                 alignFuncParam = 0,
                 genEnumBeginEndRange = False):
        GeneratorOptions.__init__(self, filename, directory, apiname, profile,
                                  versions, emitversions, defaultExtensions,
                                  addExtensions, removeExtensions,
                                  emitExtensions, sortProcedure)
        self.prefixText      = prefixText
        self.genFuncPointers = genFuncPointers
        self.protectFile     = protectFile
        self.protectFeature  = protectFeature
        self.protectProto    = protectProto
        self.protectProtoStr = protectProtoStr
        self.apicall         = apicall
        self.apientry        = apientry
        self.apientryp       = apientryp
        self.indentFuncProto = indentFuncProto
        self.indentFuncPointer = indentFuncPointer
        self.alignFuncParam  = alignFuncParam
        self.genEnumBeginEndRange = genEnumBeginEndRange

# COutputGenerator - subclass of OutputGenerator.
# Generates C-language API interfaces.
#
# ---- methods ----
# COutputGenerator(errFile, warnFile, diagFile) - args as for
#   OutputGenerator. Defines additional internal state.
# ---- methods overriding base class ----
# beginFile(genOpts)
# endFile()
# beginFeature(interface, emit)
# endFeature()
# genType(typeinfo,name)
# genStruct(typeinfo,name)
# genGroup(groupinfo,name)
# genEnum(enuminfo, name)
# genCmd(cmdinfo)
class COutputGenerator(OutputGenerator):
    """Generate specified API interfaces in a specific style, such as a C header"""
    # This is an ordered list of sections in the header file.
    TYPE_SECTIONS = ['include', 'define', 'basetype', 'handle', 'enum',
                     'group', 'bitmask', 'struct', 'funcpointer']
    ALL_SECTIONS = TYPE_SECTIONS + ['commandPointer', 'command']

    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        OutputGenerator.__init__(self, errFile, warnFile, diagFile)
        # Internal state - accumulators for different inner block text
        self.sections = {section: [] for section in self.ALL_SECTIONS}
        self.feature_not_empty = False
        self.need_platform_include = False
        self.may_alias = None

    def beginFile(self, genOpts):
        OutputGenerator.beginFile(self, genOpts)
        # C-specific
        #
        # Multiple inclusion protection & C++ wrappers.
        if (genOpts.protectFile and self.genOpts.filename):
            headerSym = re.sub(r'\.h', '_h_',
                               os.path.basename(self.genOpts.filename)).upper()
            write('#ifndef', headerSym, file=self.outFile)
            write('#define', headerSym, '1', file=self.outFile)
            self.newline()
        write('#ifdef __cplusplus', file=self.outFile)
        write('extern "C" {', file=self.outFile)
        write('#endif', file=self.outFile)
        self.newline()
        #
        # User-supplied prefix text, if any (list of strings)
        if (genOpts.prefixText):
            for s in genOpts.prefixText:
                write(s, file=self.outFile)

    def endFile(self):
        # C-specific
        # Finish C++ wrapper and multiple inclusion protection
        self.newline()
        write('#ifdef __cplusplus', file=self.outFile)
        write('}', file=self.outFile)
        write('#endif', file=self.outFile)
        if (self.genOpts.protectFile and self.genOpts.filename):
            self.newline()
            write('#endif', file=self.outFile)
        # Finish processing in superclass
        OutputGenerator.endFile(self)

    def beginFeature(self, interface, emit):
        # Start processing in superclass
        OutputGenerator.beginFeature(self, interface, emit)
        # C-specific
        # Accumulate includes, defines, types, enums, function pointer typedefs,
        # end function prototypes separately for this feature. They're only
        # printed in endFeature().
        self.sections = {section: [] for section in self.ALL_SECTIONS}
        self.feature_not_empty = False

    def endFeature(self):
        # C-specific
        # Actually write the interface to the output file.
        if (self.emit):
            if self.feature_not_empty:
                is_protected = self.featureExtraProtect != None
                if ((is_protected and self.genOpts.filename == 'openxr_platform.h') or
                    (not is_protected and self.genOpts.filename != 'openxr_platform.h')):
                    self.newline()
                    if (self.genOpts.protectFeature):
                        write('#ifndef', self.featureName, file=self.outFile)
                    # If type declarations are needed by other features based on
                    # this one, it may be necessary to suppress the ExtraProtect,
                    # or move it below the 'for section...' loop.
                    if (self.featureExtraProtect != None):
                        write('#ifdef', self.featureExtraProtect, file=self.outFile)
                    self.newline()
                    write('#define', self.featureName, '1', file=self.outFile)
                    for section in self.TYPE_SECTIONS:
                        # If we need the explicit include of the external platform header,
                        # put it right before the function pointer definitions
                        if section == "funcpointer" and self.need_platform_include:
                            write('// Include for OpenXR Platform-Specific Types', file=self.outFile)
                            write('#include "openxr_platform.h"', file=self.outFile)
                            self.newline()
                            self.need_platform_include = False
                        contents = self.sections[section]
                        if contents:
                            write('\n'.join(contents), file=self.outFile)
                    if (self.genOpts.genFuncPointers and self.sections['commandPointer']):
                        write('\n'.join(self.sections['commandPointer']), file=self.outFile)
                        self.newline()
                    if (self.sections['command']):
                        if (self.genOpts.protectProto):
                            write(self.genOpts.protectProto,
                                self.genOpts.protectProtoStr, file=self.outFile)
                        write('\n'.join(self.sections['command']), end='', file=self.outFile)
                        if (self.genOpts.protectProto):
                            write('#endif', file=self.outFile)
                        else:
                            self.newline()
                    if (self.featureExtraProtect != None):
                        write('#endif /*', self.featureExtraProtect, '*/', file=self.outFile)
                    if (self.genOpts.protectFeature):
                        write('#endif /*', self.featureName, '*/', file=self.outFile)
        # Finish processing in superclass
        OutputGenerator.endFeature(self)

    #
    # Append a definition to the specified section
    def appendSection(self, section, text):
        # self.sections[section].append('SECTION: ' + section + '\n')
        self.sections[section].append(text)
        self.feature_not_empty = True

    #
    # Type generation
    def genType(self, typeinfo, name):
        OutputGenerator.genType(self, typeinfo, name)
        typeElem = typeinfo.elem
        # If the type is a struct type, traverse the embedded <member> tags
        # generating a structure. Otherwise, emit the tag text.
        category = typeElem.get('category')
        if category in ('struct', 'union'):
            self.genStruct(typeinfo, name)
        # Replace <apientry /> tags with an APIENTRY-style string
        # (from self.genOpts). Copy other text through unchanged.
        # If the resulting text is an empty string, don't emit it.
        s = noneStr(typeElem.text)
        for elem in typeElem:
            if (elem.tag == 'apientry'):
                s += self.genOpts.apientry + noneStr(elem.tail)
            else:
                s += noneStr(elem.text) + noneStr(elem.tail)
        if s:
            # Add extra newline after multi-line entries.
            if '\n' in s:
                s += '\n'
            # This is a temporary workaround for Vulkan internal issue #877,
            # while we consider other approaches. The problem is that
            # function pointer types can have dependencies on structures
            # and vice-versa, so they can't be strictly separated into
            # sections. The workaround is to define those types in the
            # same section, in dependency order.
            if (category == 'funcpointer'):
                self.appendSection('struct', s)
            elif not (category == 'struct' or category == 'union'):
                self.appendSection(category, s)

    #
    # Protection string generation
    # Protection strings are the strings defining the OS/Platform/Graphics
    # requirements for a given OpenXR command.  When generating the
    # language header files, we need to make sure the items specific to a
    # graphics API or OS platform are properly wrapped in #ifs.
    def genProtectString(self, protect_str):
        protect_if_str = ''
        protect_end_str = ''
        protect_list = []
        if protect_str:
            if ',' in protect_str:
                protect_list.extend(protect_str.split(","))
                protect_def_str = ''
                count = 0
                for protect_define in protect_list:
                    if count > 0:
                        protect_def_str += ' &&'
                    protect_def_str += ' defined(%s)' % protect_define
                    count = count + 1
                    count = count + 1
                protect_if_str  = '#if'
                protect_if_str += protect_def_str
                protect_if_str += '\n'
                protect_end_str  = '#endif //'
                protect_end_str += protect_def_str
                protect_end_str += '\n'
            else:
                protect_if_str += '#ifdef %s\n' % protect_str
                protect_end_str += '#endif // %s\n' % protect_str
        return (protect_if_str, protect_end_str)

    def typeMayAlias(self, typeName):
        if not self.may_alias:
            # First time we've asked if a type may alias.
            # So, let's populate the set of all names of types that may.

            # Everyone with an explicit mayalias="true"
            self.may_alias = set(
                [typeName for typeName, data in self.registry.typedict.items()
                 if data.elem.get('mayalias') == 'true'])

            # Every type mentioned in some other type's parentstruct attribute.
            self.may_alias.update(set(x for x in
                                      [otherType.elem.get('parentstruct')
                                       for _, otherType in self.registry.typedict.items()]
                                      if x is not None
                                      ))
        return typeName in self.may_alias

    #
    # Struct (e.g. C "struct" type) generation.
    # This is a special case of the <type> tag where the contents are
    # interpreted as a set of <member> tags instead of freeform C
    # C type declarations. The <member> tags are just like <param>
    # tags - they are a declaration of a struct or union member.
    # Only simple member declarations are supported (no nested
    # structs etc.)
    def genStruct(self, typeinfo, typeName):
        OutputGenerator.genStruct(self, typeinfo, typeName)
        body = ''
        (protect_begin, protect_end) = self.genProtectString(typeinfo.elem.get('protect'))
        if len(protect_begin) > 0:
            body += protect_begin
        body += 'typedef ' + typeinfo.elem.get('category')

        if self.typeMayAlias(typeName):
            body += ' XR_MAY_ALIAS'

        body += ' ' + typeName + ' {\n'
        # paramdecl = self.makeCParamDecl(typeinfo.elem, self.genOpts.alignFuncParam)
        targetLen = 0
        for member in typeinfo.elem.findall('.//member'):
            targetLen = max(targetLen, self.getCParamTypeLength(member))
        for member in typeinfo.elem.findall('.//member'):
            body += self.makeCParamDecl(member, targetLen + 4)
            body += ';\n'
        body += '} ' + typeName + ';\n'
        if len(protect_end) > 0:
            body += protect_end
        self.appendSection('struct', body)

    #
    # Group (e.g. C "enum" type) generation.
    # These are concatenated together with other types.
    def genGroup(self, groupinfo, groupName):
        OutputGenerator.genGroup(self, groupinfo, groupName)
        (section, body) = self.buildEnumCDecl(self.genOpts.genEnumBeginEndRange, groupinfo, groupName)
        self.appendSection(section, "\n" + body)

    # Enumerant generation
    # <enum> tags may specify their values in several ways, but are usually
    # just integers.
    def genEnum(self, enuminfo, name):
        OutputGenerator.genEnum(self, enuminfo, name)
        (_, strVal) = self.enumToValue(enuminfo.elem, False)
        body = '#define ' + name.ljust(33) + ' ' + strVal
        self.appendSection('enum', body)

    #
    # Command generation
    def genCmd(self, cmdinfo, name):
        OutputGenerator.genCmd(self, cmdinfo, name)
        #
        decls = self.makeCDecls(cmdinfo.elem)
        self.appendSection('command', decls[0] + '\n')
        if (self.genOpts.genFuncPointers):
            self.appendSection('commandPointer', decls[1])
