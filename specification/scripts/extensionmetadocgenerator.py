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

import os,re,sys,functools
from generator import *
from functools import total_ordering

# ExtensionMetaDocGeneratorOptions - subclass of GeneratorOptions.
class ExtensionMetaDocGeneratorOptions(GeneratorOptions):
    """Represents options during extension metainformation generation for Asciidoc"""
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
                 sortProcedure = regSortFeatures):
        GeneratorOptions.__init__(self, filename, directory, apiname, profile,
                                  versions, emitversions, defaultExtensions,
                                  addExtensions, removeExtensions,
                                  emitExtensions, sortProcedure)

@total_ordering
class Extension:
    def __init__(self,
                 generator, # needed for logging
                 filename,
                 name,
                 number,
                 type,
                 requires,
                 requiresCore,
                 contact,
                 promotedTo,
                 deprecatedBy,
                 obsoletedBy,
                 revision ):
        self.generator = generator
        self.filename = filename
        self.name = name
        self.number = number
        self.type = type
        self.requires = requires
        self.requiresCore = requiresCore
        self.contact = contact
        self.promotedTo = promotedTo
        self.deprecatedBy = deprecatedBy
        self.obsoletedBy = obsoletedBy
        self.revision = revision

        self.deprecationType = None
        self.supercedingXrVersion = None
        self.supercedingExtension = None

        if self.promotedTo is not None and self.deprecatedBy is not None and self.obsoletedBy is not None:
            self.generator.logMsg('warn', 'All \'promotedto\', \'deprecatedby\' and \'obsoletedby\' attributes used on extension ' + self.name + '! Ignoring \'promotedto\' and \'deprecatedby\'.')
        elif self.promotedTo is not None and self.deprecatedBy is not None:
            self.generator.logMsg('warn', 'Both \'promotedto\' and \'deprecatedby\' attributes used on extension ' + self.name + '! Ignoring \'deprecatedby\'.')
        elif self.promotedTo is not None and self.obsoletedBy is not None:
            self.generator.logMsg('warn', 'Both \'promotedto\' and \'obsoletedby\' attributes used on extension ' + self.name + '! Ignoring \'promotedto\'.')
        elif self.deprecatedBy is not None and self.obsoletedBy is not None:
            self.generator.logMsg('warn', 'Both \'deprecatedby\' and \'obsoletedby\' attributes used on extension ' + self.name + '! Ignoring \'deprecatedby\'.')

        superceededBy = None
        if self.promotedTo is not None:
            self.deprecationType = 'promotion'
            superceededBy = promotedTo
        elif self.deprecatedBy is not None:
            self.deprecationType = 'deprecation'
            superceededBy = deprecatedBy
        elif self.obsoletedBy is not None:
            self.deprecationType = 'obsoletion'
            superceededBy = obsoletedBy

        if superceededBy is not None:
            if superceededBy == '' and not self.deprecationType == 'promotion':
                pass # supercedingXrVersion, supercedingExtension == None
            elif superceededBy.startswith('XR_VERSION_'):
                self.supercedingXrVersion = superceededBy
            elif superceededBy.startswith('XR_'):
                self.supercedingExtension = superceededBy
            else:
                self.generator.logMsg('error', 'Unrecognized ' + self.deprecationType + ' attribute value \'' + superceededBy + '\'!')

    def __str__(self): return self.name

    def __eq__(self, other): return self.name == other.name
    def __ne__(self, other): return self.name != other.name
    def __lt__(self, other):
        me_is_KHR = self.name.startswith( 'XR_KHR' )
        me_is_EXT = self.name.startswith( 'XR_EXT' )
        he_is_KHR = other.name.startswith( 'XR_KHR' )
        he_is_EXT = other.name.startswith( 'XR_EXT' )

        swap = False
        if me_is_KHR and not he_is_KHR:
            return not swap
        elif he_is_KHR and not me_is_KHR:
            return swap
        elif me_is_EXT and not he_is_EXT:
            return not swap
        elif he_is_EXT and not me_is_EXT:
            return swap
        else:
            return self.name < other.name

    def typeToStr(self):
        if self.type == 'instance':
            return 'Instance extension'
        elif self.type == 'device':
            return 'Device extension'
        elif self.type != None:
            self.generator.logMsg('warn', 'The type attribute of ' + self.name + ' extension is neither \'instance\' nor \'device\'. That is invalid (at the time this script was written).')
        else: # should be unreachable
            self.generator.logMsg('error', 'Logic error in typeToStr(): Missing type attribute!')

    def conditionalLinkCoreXr(self, xrVersion, linkSuffix):
        versionMatch = re.match(r'XR_VERSION_(\d+)_(\d+)', xrVersion)
        major = versionMatch.group(1)
        minor = versionMatch.group(2)

        dottedVersion = major + '.' + minor

        doc  = 'ifdef::' + xrVersion + '[]\n'
        doc += '    <<versions-' + dottedVersion + linkSuffix + ', OpenXR ' + dottedVersion + '>>\n'
        doc += 'endif::' + xrVersion + '[]\n'
        doc += 'ifndef::' + xrVersion + '[]\n'
        doc += '    OpenXR ' + dottedVersion + '\n'
        doc += 'endif::' + xrVersion + '[]\n'

        return doc

    def conditionalLinkExt(self, extName, indent = '    '):
        doc  = 'ifdef::' + extName + '[]\n'
        doc +=  indent + '<<' + extName + '>>\n'
        doc += 'endif::' + extName + '[]\n'
        doc += 'ifndef::' + extName + '[]\n'
        doc += indent + '`' + extName + '`\n'
        doc += 'endif::' + extName + '[]\n'

        return doc

    def resolveDeprecationChain(self, extensionsList, succeededBy, file):
        ext = next(x for x in extensionsList if x.name == succeededBy)

        if ext.deprecationType:
            if ext.deprecationType == 'promotion':
                if ext.supercedingXrVersion:
                    write('  ** Which in turn was _promoted_ to\n' + ext.conditionalLinkCoreXr(ext.supercedingXrVersion, '-promotions'), file=file)
                else: # ext.supercedingExtension
                    write('  ** Which in turn was _promoted_ to extension\n' + ext.conditionalLinkExt(ext.supercedingExtension), file=file)
                    ext.resolveDeprecationChain(extensionsList, ext.supercedingExtension, file)
            elif ext.deprecationType == 'deprecation':
                if ext.supercedingXrVersion:
                    write('  ** Which in turn was _deprecated_ by\n' + ext.conditionalLinkCoreXr(ext.supercedingXrVersion, '-new-feature'), file=file)
                elif ext.supercedingExtension:
                    write('  ** Which in turn was _deprecated_ by\n' + ext.conditionalLinkExt(ext.supercedingExtension) + '    extension', file=file)
                    ext.resolveDeprecationChain(extensionsList, ext.supercedingExtension, file)
                else:
                    write('  ** Which in turn was _deprecated_ without replacement', file=file)
            elif ext.deprecationType == 'obsoletion':
                if ext.supercedingXrVersion:
                    write('  ** Which in turn was _obsoleted_ by\n' + ext.conditionalLinkCoreXr(ext.supercedingXrVersion, '-new-feature'), file=file)
                elif ext.supercedingExtension:
                    write('  ** Which in turn was _obsoleted_ by\n' + ext.conditionalLinkExt(ext.supercedingExtension) + '    extension', file=file)
                    ext.resolveDeprecationChain(extensionsList, ext.supercedingExtension, file)
                else:
                    write('  ** Which in turn was _obsoleted_ without replacement', file=file)
            else: # should be unreachable
                self.generator.logMsg('error', 'Logic error in resolveDeprecationChain(): deprecationType is neither \'promotion\', \'deprecation\' nor \'obsoletion\'!')


    def makeMetafile(self, extensionsList):
        fp = self.generator.newFile(self.filename)

        write('[[' + self.name + ']]', file=fp)
        write('=== ' + self.name, file=fp)
        write('', file=fp)

        write('*Name String*::', file=fp)
        write('    `' + self.name + '`', file=fp)

        write('*Extension Type*::', file=fp)
        write('    ' + self.typeToStr(), file=fp)

        write('*Registered Extension Number*::', file=fp)
        write('    ' + self.number, file=fp)

        write('*Revision*::', file=fp)
        write('    ' + self.revision, file=fp)

        # Only OpenXR extension dependencies are coded in XML, others are explicit
        write('*Extension and Version Dependencies*::', file=fp)
        write('  * Requires OpenXR ' + self.requiresCore, file=fp)
        if self.requires:
            for dep in self.requires.split(','):
                write('  * Requires `<<' + dep + '>>`', file=fp)

        if self.deprecationType:
            write('*Deprecation state*::', file=fp)

            if self.deprecationType == 'promotion':
                if self.supercedingXrVersion:
                    write('  * _Promoted_ to\n' + self.conditionalLinkCoreXr(self.supercedingXrVersion, '-promotions'), file=fp)
                else: # ext.supercedingExtension
                    write('  * _Promoted_ to\n' + self.conditionalLinkExt(self.supercedingExtension) + '    extension', file=fp)
                    self.resolveDeprecationChain(extensionsList, self.supercedingExtension, fp)
            elif self.deprecationType == 'deprecation':
                if self.supercedingXrVersion:
                    write('  * _Deprecated_ by\n' + self.conditionalLinkCoreXr(self.supercedingXrVersion, '-new-features'), file=fp)
                elif self.supercedingExtension:
                    write('  * _Deprecated_ by\n' + self.conditionalLinkExt(self.supercedingExtension) + '    extension' , file=fp)
                    self.resolveDeprecationChain(extensionsList, self.supercedingExtension, fp)
                else:
                    write('  * _Deprecated_ without replacement' , file=fp)
            elif self.deprecationType == 'obsoletion':
                if self.supercedingXrVersion:
                    write('  * _Obsoleted_ by\n' + self.conditionalLinkCoreXr(self.supercedingXrVersion, '-new-features'), file=fp)
                elif self.supercedingExtension:
                    write('  * _Obsoleted_ by\n' + self.conditionalLinkExt(self.supercedingExtension) + '    extension' , file=fp)
                    self.resolveDeprecationChain(extensionsList, self.supercedingExtension, fp)
                else:
                    # TODO: Does not make sense to retroactively ban use of extensions from 1.0.
                    #       Needs some tweaks to the semantics and this message, when such extension(s) occur.
                    write('  * _Obsoleted_ without replacement' , file=fp)
            else: # should be unreachable
                self.generator.logMsg('error', 'Logic error in makeMetafile(): deprecationType is neither \'promotion\', \'deprecation\' nor \'obsoletion\'!')

        #write('*Contact*::', file=fp)
        #contacts = self.contact.split(',')
        #for c in contacts:
        #    write('  * ' + c, file=fp)

        fp.close()

        # Now make the refpage include
        fp = self.generator.newFile(self.filename.replace('meta/', 'meta/refpage.'))

        write('== Registered Extension Number', file=fp)
        write(self.number, file=fp)
        write('', file=fp)

        write('== Revision', file=fp)
        write(self.revision, file=fp)
        write('', file=fp)

        # Only OpenXR extension dependencies are coded in XML, others are explicit
        write('== Extension and Version Dependencies', file=fp)
        write('  * Requires OpenXR ' + self.requiresCore, file=fp)
        if self.requires:
            for dep in self.requires.split(','):
                write('  * Requires `<<' + dep + '>>`', file=fp)
        write('', file=fp)

        if self.deprecationType:
            write('== Deprecation state', file=fp)

            if self.deprecationType == 'promotion':
                if self.supercedingXrVersion:
                    write('  * _Promoted_ to\n' + self.conditionalLinkCoreXr(self.supercedingXrVersion, '-promotions'), file=fp)
                else: # ext.supercedingExtension
                    write('  * _Promoted_ to\n' + self.conditionalLinkExt(self.supercedingExtension) + '    extension', file=fp)
                    self.resolveDeprecationChain(extensionsList, self.supercedingExtension, fp)
            elif self.deprecationType == 'deprecation':
                if self.supercedingXrVersion:
                    write('  * _Deprecated_ by\n' + self.conditionalLinkCoreXr(self.supercedingXrVersion, '-new-features'), file=fp)
                elif self.supercedingExtension:
                    write('  * _Deprecated_ by\n' + self.conditionalLinkExt(self.supercedingExtension) + '    extension' , file=fp)
                    self.resolveDeprecationChain(extensionsList, self.supercedingExtension, fp)
                else:
                    write('  * _Deprecated_ without replacement' , file=fp)
            elif self.deprecationType == 'obsoletion':
                if self.supercedingXrVersion:
                    write('  * _Obsoleted_ by\n' + self.conditionalLinkCoreXr(self.supercedingXrVersion, '-new-features'), file=fp)
                elif self.supercedingExtension:
                    write('  * _Obsoleted_ by\n' + self.conditionalLinkExt(self.supercedingExtension) + '    extension' , file=fp)
                    self.resolveDeprecationChain(extensionsList, self.supercedingExtension, fp)
                else:
                    # TODO: Does not make sense to retroactively ban use of extensions from 1.0.
                    #       Needs some tweaks to the semantics and this message, when such extension(s) occur.
                    write('  * _Obsoleted_ without replacement' , file=fp)
            else: # should be unreachable
                self.generator.logMsg('error', 'Logic error in makeMetafile(): deprecationType is neither \'promotion\', \'deprecation\' nor \'obsoletion\'!')

            write('', file=fp)

        fp.close()


# ExtensionMetaDocOutputGenerator - subclass of OutputGenerator.
# Generates AsciiDoc includes with metainformation for the OpenXR extension
# appendices. The fields used from <extension> tags in xr.xml are:
#
# name          extension name string
# number        extension number (optional)
# contact       name and github login or email address (optional)
# type          'instance' | 'device' (optional)
# requires      list of comma-separate requires OpenXR extensions (optional)
# requiresCore  required core version of OpenXR (optional)
# promotedTo    extension or OpenXR version it was promoted to
# deprecatedBy   extension or OpenXR version which deprecated this extension,
#                or empty string if deprecated without replacement
# obsoletedBy   extension or OpenXR version which obsoleted this extension,
#                or empty string if obsoleted without replacement
#
# ---- methods ----
# ExtensionMetaDocOutputGenerator(errFile, warnFile, diagFile) - args as for
#   OutputGenerator. Defines additional internal state.
# ---- methods overriding base class ----
# beginFile(genOpts)
# endFile()
# beginFeature(interface, emit)
# endFeature()
class ExtensionMetaDocOutputGenerator(OutputGenerator):
    """Generate specified API interfaces in a specific style, such as a C header"""
    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        OutputGenerator.__init__(self, errFile, warnFile, diagFile)
        self.extensions = []
        # List of strings containing all vendor tags
        self.vendor_tags = []

    def newFile(self, filename):
        self.logMsg('diag', '# Generating include file:', filename)
        fp = open(filename, 'w', encoding='utf-8')
        write('// WARNING: DO NOT MODIFY! This file is automatically generated from the xr.xml registry', file=fp)
        return fp

    def beginFile(self, genOpts):
        OutputGenerator.beginFile(self, genOpts)

        self.directory = self.genOpts.directory

        # Iterate over all 'tag' Elements and add the names of all the valid vendor
        # tags to the list
        root = self.registry.tree.getroot()
        for tag in root.findall('tags/tag'):
            self.vendor_tags.append(tag.get('name'))

        # Create subdirectory, if needed
        self.makeDir(self.directory)

    def conditionalExt(self, extName, content, ifdef = None, condition = None):
        doc = ''

        innerdoc  = 'ifdef::' + extName + '[]\n'
        innerdoc += content + '\n'
        innerdoc += 'endif::' + extName + '[]\n'

        if ifdef:
            if ifdef == 'ifndef':
                doc += 'ifndef::' + condition + '[]\n'
                doc += innerdoc
                doc += 'endif::' + condition + '[]\n'
            elif ifdef == 'ifdef':
                doc += 'ifdef::' + condition + '+' + extName + '[]\n'
                doc += content + '\n'
                doc += 'endif::' + condition + '+' + extName + '[]\n'
            else: # should be unreachable
                self.logMsg('error', 'Logic error in conditionalExt(): ifdef is neither \'ifdef \' nor \'ifndef\'!')
        else:
            doc += innerdoc

        return doc

    def outputExtensionInclude(self, name, vendor, deprecation_type, conditions, current_fp, current_toc_fp, deprecated_fp, deprecated_toc_fp, guard_fp):
        include = 'include::../' + vendor.lower() + '/' + name[3:].lower()  + '.adoc[]'
        link = '  * <<' + name + '>>'

        if deprecation_type is None:
            write(self.conditionalExt(name, include), file=current_fp)
            write(self.conditionalExt(name, link), file=current_toc_fp)
        else:
            write(self.conditionalExt(name, include, 'ifndef', conditions), file=current_fp)
            write(self.conditionalExt(name, link, 'ifndef', conditions), file=current_toc_fp)

            write(self.conditionalExt(name, include, 'ifdef', conditions), file=deprecated_fp)
            write(self.conditionalExt(name, link, 'ifdef', conditions), file=deprecated_toc_fp)

            write(self.conditionalExt(name, ':HAS_DEPRECATED_EXTENSIONS:', 'ifdef', conditions), file=guard_fp)

    def endFile(self):
        self.extensions.sort()

        for ext in self.extensions:
            ext.makeMetafile(self.extensions)

        promotedExtensions = {}
        for ext in self.extensions:
            if ext.deprecationType == 'promotion' and ext.supercedingXrVersion:
                promotedExtensions.setdefault(ext.supercedingXrVersion, []).append(ext)

        for coreVersion, extensions in promotedExtensions.items():
            promoted_extensions_fp = self.newFile(self.directory + '/promoted_extensions_' + coreVersion + '.adoc')

            for ext in extensions:
                indent = ''
                write('  * {blank}\n+\n' + ext.conditionalLinkExt(ext.name, indent), file=promoted_extensions_fp)

            promoted_extensions_fp.close()

        current_extensions_appendix_fp = self.newFile(self.directory + '/current_extensions_appendix.adoc')
        deprecated_extensions_appendix_fp = self.newFile(self.directory + '/deprecated_extensions_appendix.adoc')
        current_extension_appendices_fp = self.newFile(self.directory + '/current_extension_appendices.adoc')
        current_extension_appendices_toc_fp = self.newFile(self.directory + '/current_extension_appendices_toc.adoc')
        deprecated_extension_appendices_fp = self.newFile(self.directory + '/deprecated_extension_appendices.adoc')
        deprecated_extension_appendices_toc_fp = self.newFile(self.directory + '/deprecated_extension_appendices_toc.adoc')
        deprecated_extensions_guard_macro_fp = self.newFile(self.directory + '/deprecated_extensions_guard_macro.adoc')

        write('include::deprecated_extensions_guard_macro.adoc[]', file=current_extensions_appendix_fp)
        write('', file=current_extensions_appendix_fp)
        write('ifndef::HAS_DEPRECATED_EXTENSIONS[]', file=current_extensions_appendix_fp)
        write('[[extension-appendices-list]]', file=current_extensions_appendix_fp)
        write('== List of Extensions', file=current_extensions_appendix_fp)
        write('endif::HAS_DEPRECATED_EXTENSIONS[]', file=current_extensions_appendix_fp)
        write('ifdef::HAS_DEPRECATED_EXTENSIONS[]', file=current_extensions_appendix_fp)
        write('[[extension-appendices-list]]', file=current_extensions_appendix_fp)
        write('== List of Current Extensions', file=current_extensions_appendix_fp)
        write('endif::HAS_DEPRECATED_EXTENSIONS[]', file=current_extensions_appendix_fp)
        write('', file=current_extensions_appendix_fp)
        write('include::current_extension_appendices_toc.adoc[]', file=current_extensions_appendix_fp)
        write('<<<', file=current_extensions_appendix_fp)
        write('include::current_extension_appendices.adoc[]', file=current_extensions_appendix_fp)

        write('include::deprecated_extensions_guard_macro.adoc[]', file=deprecated_extensions_appendix_fp)
        write('', file=deprecated_extensions_appendix_fp)
        write('ifdef::HAS_DEPRECATED_EXTENSIONS[]', file=deprecated_extensions_appendix_fp)
        write('[[deprecated-extension-appendices-list]]', file=deprecated_extensions_appendix_fp)
        write('== List of Deprecated Extensions', file=deprecated_extensions_appendix_fp)
        write('include::deprecated_extension_appendices_toc.adoc[]', file=deprecated_extensions_appendix_fp)
        write('<<<', file=deprecated_extensions_appendix_fp)
        write('include::deprecated_extension_appendices.adoc[]', file=deprecated_extensions_appendix_fp)
        write('endif::HAS_DEPRECATED_EXTENSIONS[]', file=deprecated_extensions_appendix_fp)

        # add include guard to allow multiple includes
        write('ifndef::DEPRECATED_EXTENSIONS_GUARD_MACRO_INCLUDE_GUARD[]', file=deprecated_extensions_guard_macro_fp)
        write(':DEPRECATED_EXTENSIONS_GUARD_MACRO_INCLUDE_GUARD:\n', file=deprecated_extensions_guard_macro_fp)

        self.extensions.sort(key=lambda ext: ext.name.lower())

        # Output all KHR extensions
        for ext in self.extensions:
            if ext.name.startswith('XR_KHR_'):
                condition = ext.supercedingXrVersion if ext.supercedingXrVersion else ext.supercedingExtension
                self.outputExtensionInclude(ext.name,
                                            'KHR',
                                            ext.deprecationType,
                                            condition,
                                            current_extension_appendices_fp,
                                            current_extension_appendices_toc_fp,
                                            deprecated_extension_appendices_fp,
                                            deprecated_extension_appendices_toc_fp,
                                            deprecated_extensions_guard_macro_fp)

        # Next, output all EXT extensions
        for ext in self.extensions:
            if ext.name.startswith('XR_EXT_'):
                condition = ext.supercedingXrVersion if ext.supercedingXrVersion else ext.supercedingExtension
                self.outputExtensionInclude(ext.name,
                                            'EXT',
                                            ext.deprecationType,
                                            condition,
                                            current_extension_appendices_fp,
                                            current_extension_appendices_toc_fp,
                                            deprecated_extension_appendices_fp,
                                            deprecated_extension_appendices_toc_fp,
                                            deprecated_extensions_guard_macro_fp)

        # Finally, output the rest
        for ext in self.extensions:
            # Skip KHR and EXT
            if ext.name.startswith('XR_KHR_') or ext.name.startswith('XR_EXT_'):
                continue
            found_vendor_tag = False
            vendor_tag = ''
            for cur_vendor_tag in self.vendor_tags:
                vendor_tag_prefix = 'XR_' + cur_vendor_tag + '_'
                if ext.name.startswith(vendor_tag_prefix):
                    found_vendor_tag = True
                    vendor_tag = cur_vendor_tag
                    break

            if not found_vendor_tag:
                self.logMsg('warn', 'Unknown vendor tag for extension ' + ext.name + ', dropping from list.')

            condition = ext.supercedingXrVersion if ext.supercedingXrVersion else ext.supercedingExtension
            self.outputExtensionInclude(ext.name,
                                        vendor_tag,
                                        ext.deprecationType,
                                        condition,
                                        current_extension_appendices_fp,
                                        current_extension_appendices_toc_fp,
                                        deprecated_extension_appendices_fp,
                                        deprecated_extension_appendices_toc_fp,
                                        deprecated_extensions_guard_macro_fp)

        current_extensions_appendix_fp.close()
        deprecated_extensions_appendix_fp.close()
        current_extension_appendices_fp.close()
        current_extension_appendices_toc_fp.close()
        deprecated_extension_appendices_fp.close()
        deprecated_extension_appendices_toc_fp.close()

        write('endif::DEPRECATED_EXTENSIONS_GUARD_MACRO_INCLUDE_GUARD[]', file=deprecated_extensions_guard_macro_fp)
        deprecated_extensions_guard_macro_fp.close()

        OutputGenerator.endFile(self)

    def beginFeature(self, interface, emit):
        # Start processing in superclass
        OutputGenerator.beginFeature(self, interface, emit)

        if interface.tag != 'extension':
            self.logMsg('diag', 'beginFeature: ignoring non-extension feature', self.featureName)
            return

        # These attributes must exist
        name = self.featureName
        number = self.getAttrib(interface, 'number')
        type = self.getAttrib(interface, 'type')
        revision = self.getSpecVersion(interface, name)

        # These attributes are optional
        OPTIONAL = False
        requires = self.getAttrib(interface, 'requires', OPTIONAL)
        requiresCore = self.getAttrib(interface, 'requiresCore', OPTIONAL, '0.90')
        contact = self.getAttrib(interface, 'contact', OPTIONAL)
        promotedTo = self.getAttrib(interface, 'promotedto', OPTIONAL)
        deprecatedBy = self.getAttrib(interface, 'deprecatedby', OPTIONAL)
        obsoletedBy = self.getAttrib(interface, 'obsoletedby', OPTIONAL)

        filename = self.directory + '/' + name + '.adoc'

        self.extensions.append( Extension(self, filename, name, number, type, requires, requiresCore, contact, promotedTo, deprecatedBy, obsoletedBy, revision) )

    def endFeature(self):
        # Finish processing in superclass
        OutputGenerator.endFeature(self)

    # Query an attribute from an element, or return a default value
    #   elem - element to query
    #   attribute - attribute name
    #   required - whether attribute must exist
    #   default - default value if attribute not present
    def getAttrib(self, elem, attribute, required=True, default=None):
            attrib = elem.get(attribute, default)
            if required and (attrib is None):
                name = elem.get('name', 'UNKNOWN')
                self.logMsg('error', 'While processing \'' + self.featureName + ', <' + elem.tag + '> \'' + name + '\' does not contain required attribute \'' + attribute + '\'')
            return attrib

    def numbersToWords(self, name):
        whitelist = ['WIN32', 'INT16', 'D3D1']

        # temporarily replace whitelist items
        for i, w in enumerate(whitelist):
            name = re.sub(w, '{' + str(i) + '}', name)

        name = re.sub(r'(?<=[A-Z])(\d+)(?![A-Z])', r'_\g<1>', name)

        # undo whitelist substitution
        for i, w in enumerate(whitelist):
            name = re.sub('\\{' + str(i) + '}', w, name)

        return name

    #
    # Determine the extension revision from the EXTENSION_NAME_SPEC_VERSION
    # enumerant.
    #
    #   elem - <extension> element to query
    #   extname - extension name from the <extension> 'name' attribute
    #   default - default value if SPEC_VERSION token not present
    def getSpecVersion(self, elem, extname, default=None):
        # The literal enumerant name to match
        versioningEnumName = self.numbersToWords(extname) + '_SPEC_VERSION'

        for enum in elem.findall('./require/enum'):
            enumName = self.getAttrib(enum, 'name')
            if enumName == versioningEnumName:
                return self.getAttrib(enum, 'value')

        #if not found:
        for enum in elem.findall('./require/enum'):
            enumName = self.getAttrib(enum, 'name')
            if enumName.find('SPEC_VERSION') != -1:
                self.logMsg('warn', 'Missing ' + versioningEnumName + '! Potential misnamed candidate ' + enumName + '.')
                return self.getAttrib(enum, 'value')

        self.logMsg('error', 'Missing ' + versioningEnumName + '!')
