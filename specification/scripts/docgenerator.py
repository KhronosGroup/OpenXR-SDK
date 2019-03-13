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

import sys

from generator import GeneratorOptions, OutputGenerator, regSortFeatures, noneStr, write


class DocGeneratorOptions(GeneratorOptions):
    """DocGeneratorOptions - subclass of GeneratorOptions.

    Shares many members with CGeneratorOptions, since
    both are writing C-style declarations:

    conventions - may be mandatory for some generators:
        an object that implements ConventionsBase
    prefixText - list of strings to prefix generated header with
        (usually a copyright statement + calling convention macros).
    apicall - string to use for the function declaration prefix,
        such as APICALL on Windows.
    apientry - string to use for the calling convention macro,
        in typedefs, such as APIENTRY.
    apientryp - string to use for the calling convention macro
        in function pointer typedefs, such as APIENTRYP.
    directory - directory into which to generate include files
    indentFuncProto - True if prototype declarations should put each
        parameter on a separate line
    indentFuncPointer - True if typedefed function pointers should put each
        parameter on a separate line
    alignFuncParam - if nonzero and parameters are being put on a
        separate line, align parameter names at the specified column
    """

    def __init__(self,
                 conventions = None,
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
                 apicall = '',
                 apientry = '',
                 apientryp = '',
                 indentFuncProto = True,
                 indentFuncPointer = False,
                 alignFuncParam = 0,
                 expandEnumerants = True):
        GeneratorOptions.__init__(self, filename, directory, apiname, profile,
                                  versions, emitversions, defaultExtensions,
                                  addExtensions, removeExtensions,
                                  emitExtensions, sortProcedure)
        self.prefixText      = prefixText
        self.apicall         = apicall
        self.apientry        = apientry
        self.apientryp       = apientryp
        self.indentFuncProto = indentFuncProto
        self.indentFuncPointer = indentFuncPointer
        self.alignFuncParam  = alignFuncParam
        self.expandEnumerants = expandEnumerants
        self.conventions = conventions

# DocOutputGenerator - subclass of OutputGenerator.
# Generates AsciiDoc includes with C-language API interfaces, for reference
# pages and the OpenXR specification. Similar to COutputGenerator, but
# each interface is written into a different file as determined by the
# options, only actual C types are emitted, and none of the boilerplate
# preprocessor code is emitted.
#
# ---- methods ----
# DocOutputGenerator(errFile, warnFile, diagFile) - args as for
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
class DocOutputGenerator(OutputGenerator):
    """Generate specified API interfaces in a specific style, such as a C header"""

    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        OutputGenerator.__init__(self, errFile, warnFile, diagFile)
        # Keep track of all extension numbers
        self.extension_numbers = []

    def beginFile(self, genOpts):
        OutputGenerator.beginFile(self, genOpts)

    def endFile(self):
        OutputGenerator.endFile(self)

    def beginFeature(self, interface, emit):
        # Start processing in superclass
        OutputGenerator.beginFeature(self, interface, emit)
        # Verify that each extension has a unique number during doc generation
        extension_number = interface.get('number')
        if extension_number is not None and extension_number != "0":
            if extension_number in self.extension_numbers:
                self.logMsg('error', 'Duplicate extension number ', extension_number, ' detected in feature ', interface.get('name'), '\n')
                exit(1)
            else:
                self.extension_numbers.append(extension_number)

    def endFeature(self):
        # Finish processing in superclass
        OutputGenerator.endFeature(self)

    #
    # Generate an include file
    #
    # directory - subdirectory to put file in
    # basename - base name of the file
    # contents - contents of the file (Asciidoc boilerplate aside)
    def writeInclude(self, directory, basename, contents):
        # Create subdirectory, if needed
        directory = self.genOpts.directory + '/' + directory
        self.makeDir(directory)

        # Create file
        filename = directory + '/' + basename + '.txt'
        self.logMsg('diag', '# Generating include file:', filename)
        fp = open(filename, 'w', encoding='utf-8')

        # Asciidoc anchor
        write('// WARNING: DO NOT MODIFY! This file is automatically generated from the xr.xml registry', file=fp)
        write('[[{0},{0}]]'.format(basename), file=fp)
        write('[source,c++]', file=fp)
        write('----', file=fp)
        write(contents, file=fp)
        write('----', file=fp)
        fp.close()

        # Create secondary no cross-reference include file
        filename = directory + '/' + basename + '.no-xref.txt'
        self.logMsg('diag', '# Generating include file:', filename)
        fp = open(filename, 'w', encoding='utf-8')

        # Asciidoc anchor
        write('// WARNING: DO NOT MODIFY! This file is automatically generated from the xr.xml registry', file=fp)
        write('// Include this no-xref version without cross reference id for multiple includes of same file', file=fp)
        write('[source,c++]', file=fp)
        write('----', file=fp)
        write(contents, file=fp)
        write('----', file=fp)
        fp.close()

    #
    # Type generation
    def genType(self, typeinfo, name):
        OutputGenerator.genType(self, typeinfo, name)
        typeElem = typeinfo.elem
        # If the type is a struct type, traverse the embedded <member> tags
        # generating a structure. Otherwise, emit the tag text.
        category = typeElem.get('category')
        if (category == 'struct' or category == 'union'):
            self.genStruct(typeinfo, name)
        else:
            # Replace <apientry /> tags with an APIENTRY-style string
            # (from self.genOpts). Copy other text through unchanged.
            # If the resulting text is an empty string, don't emit it.
            s = noneStr(typeElem.text)
            for elem in typeElem:
                if (elem.tag == 'apientry'):
                    s += self.genOpts.apientry + noneStr(elem.tail)
                else:
                    s += noneStr(elem.text) + noneStr(elem.tail)
            if (len(s) > 0):
                if (category in OutputGenerator.categoryToPath.keys()):
                    self.writeInclude(OutputGenerator.categoryToPath[category],
                        name, s + '\n')
                else:
                    self.logMsg('diag', '# NOT writing include file for type:',
                        name, 'category: ', category)
            else:
                self.logMsg('diag', '# NOT writing empty include file for type', name)
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
        s = 'typedef ' + typeinfo.elem.get('category') + ' ' + typeName + ' {\n'
        # paramdecl = self.makeCParamDecl(typeinfo.elem, self.genOpts.alignFuncParam)
        targetLen = 0
        for member in typeinfo.elem.findall('.//member'):
            targetLen = max(targetLen, self.getCParamTypeLength(member))
        for member in typeinfo.elem.findall('.//member'):
            s += self.makeCParamDecl(member, targetLen + 4)
            s += ';\n'
        s += '} ' + typeName + ';'
        self.writeInclude('structs', typeName, s)

    #
    # Group (e.g. C "enum" type) generation.
    # These are concatenated together with other types.
    def genGroup(self, groupinfo, groupName):
        OutputGenerator.genGroup(self, groupinfo, groupName)
        expand = self.genOpts.expandEnumerants
        (_, body) = self.buildEnumCDecl(expand, groupinfo, groupName)
        self.writeInclude('enums', groupName, body)

    # Enumerant generation
    # <enum> tags may specify their values in several ways, but are usually
    # just integers.

    def genEnum(self, enuminfo, name):
        OutputGenerator.genEnum(self, enuminfo, name)
        self.logMsg('diag', '# NOT writing compile-time constant', name)
        # (_, strVal) = self.enumToValue(enuminfo.elem, False)
        # s = '#define ' + name.ljust(33) + ' ' + strVal
        # self.writeInclude('consts', name, s)

    #
    # Command generation
    def genCmd(self, cmdinfo, name):
        OutputGenerator.genCmd(self, cmdinfo, name)
        return_type = cmdinfo.elem.find('proto/type')
        if return_type is not None and return_type.text == 'XrResult':
            # This command is an XrResult, so check that it returns at least the required errors.
            required_errors = [ 'XR_ERROR_VALIDATION_FAILURE' ]
            errorcodes = cmdinfo.elem.attrib.get('errorcodes').split(',')
            if not set(required_errors).issubset(set(errorcodes)):
                self.logMsg('error', 'Missing required error code for command: ', name, '\n')
                exit(1)
        decls = self.makeCDecls(cmdinfo.elem)
        self.writeInclude('protos', name, decls[0])
