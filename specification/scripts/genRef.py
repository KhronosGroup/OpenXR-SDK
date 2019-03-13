#!/usr/bin/python3
#
# Copyright (c) 2016-2019 The Khronos Group Inc.
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

# genRef.py - create OpenXR ref pages from spec source files
#
#
# Usage: genRef.py files

from reflib import *
from xrapi import *
from reg import Registry
from collections import OrderedDict
import argparse, copy, io, os, pdb, re, string, sys


def makeExtensionInclude(name):
    """Return an include command, given an extension name."""
    return 'include::../sources/chapters/extensions/meta/refpage.{}.adoc[]'.format(
        name)


# Return True if name is an OpenXR extension name (ends with an upper-case
# author ID). This assumes that author IDs are at least two characters.
def isextension(name):
    return name[-2:].isalpha() and name[-2:].isupper()

# Print Khronos CC-BY copyright notice on open file fp. If comment is
# True, print as an asciidoc comment block, which copyrights the source
# file. Otherwise print as an asciidoc include of the copyright in markup,
# which copyrights the outputs. Also include some asciidoc boilerplate
# needed by all the standalone ref pages.
def printCopyrightSourceComments(fp):
        print('// Copyright (c) 2014-2019 Khronos Group. This work is licensed under a', file=fp)
        print('// Creative Commons Attribution 4.0 International License; see', file=fp)
        print('// http://creativecommons.org/licenses/by/4.0/', file=fp)
        print('', file=fp)

def printFooter(fp):
    print('include::footer.txt[]', file=fp)
    print('', file=fp)


# Add a spec asciidoc macro prefix to an OpenXR name, depending on its type
# (protos, structs, enums, etc.)
def macroPrefix(name):
    if name in basetypes.keys():
        return 'basetype:' + name
    elif name in defines.keys():
        return 'dlink:' + name
    elif name in enums.keys():
        return 'elink:' + name
    elif name in flags.keys():
        return 'elink:' + name
    elif name in funcpointers.keys():
        return 'tlink:' + name
    elif name in handles.keys():
        return 'slink:' + name
    elif name in protos.keys():
        return 'flink:' + name
    elif name in structs.keys():
        return 'slink:' + name
    elif name == 'TBD':
        return 'No cross-references are available'
    else:
        return 'UNKNOWN:' + name

# Return an asciidoc string with a list of 'See Also' references for the
# OpenXR entity 'name', based on the relationship mapping in xrapi.py and
# the additional references in explicitRefs. If no relationships are
# available, return None.
def seeAlsoList(apiName, explicitRefs = None):
    refs = {}

    # Add all the implicit references to refs
    if apiName in mapDict.keys():
        for name in sorted(mapDict[apiName].keys()):
            refs[name] = None

    # Add all the explicit references
    if explicitRefs != None:
        if isinstance(explicitRefs, str):
            explicitRefs = explicitRefs.split()
        for name in explicitRefs:
            refs[name] = None

    names = [macroPrefix(name) for name in sorted(refs.keys())]
    if len(names) > 0:
        return ', '.join(names) + '\n'
    else:
        return None

# Remap include directives in a list of lines so they can be extracted to a
# different directory. Returns remapped lines.
#
# lines - text to remap
# baseDir - target directory
# specDir - source directory
def remapIncludes(lines, baseDir, specDir):
    # This should be compiled only once
    includePat = re.compile(r'^include::(?P<path>.*)\[\]')

    newLines = []
    for line in lines:
        matches = includePat.search(line)
        if matches != None:
            path = matches.group('path')

            # Relative path to include file from here
            incPath = specDir + '/' + path
            # Remap to be relative to baseDir
            newPath = os.path.relpath(incPath, baseDir)
            newLine = 'include::' + newPath + '[]\n'
            logDiag('remapIncludes: remapping', line, '->', newLine)
            newLines.append(newLine)
        else:
            newLines.append(line)
    return newLines


def refPageShell(pageName, pageDesc, fp, sections=None, tail_content=None, man_section=3):
    printCopyrightSourceComments(fp)

    print(':data-uri:',
          ':icons: font',
          '',
          sep='\n', file=fp)

    s = '{}({})'.format(pageName, man_section)
    print('= ' + s,
          '',
          sep='\n', file=fp)
    if pageDesc.strip() == '':
        pageDesc = 'NO SHORT DESCRIPTION PROVIDED'
        logWarn('refPageHead: no short description provided for', pageName)

    print('== Name',
          '{} - {}'.format(pageName, pageDesc),
          '',
          sep='\n', file=fp)

    if sections:
        for title, content in sections.items():
            print('== {}'.format(title),
                  '',
                  content,
                  '',
                  sep='\n', file=fp)

    if tail_content:
        print(tail_content,
              '',
              sep='\n', file=fp)


# Generate header of a reference page
# pageName - string name of the page
# pageDesc - string short description of the page
# specText - string that goes in the "C Specification" section
# fieldName - string heading an additional section following specText, if not None
# fieldText - string that goes in the additional section
# descText - string that goes in the "Description" section
# fp - file to write to
def refPageHead(pageName, pageDesc, specText, fieldName, fieldText, descText, fp):
    sections = OrderedDict()
    sections['C Specification'] = specText
    if fieldName != None:
        sections[fieldName] = fieldText

    if descText is None or descText.strip() == '':
        logWarn('refPageHead: no description provided for', pageName)

    if descText is not None:
        sections['Description'] = descText

    refPageShell(pageName, pageDesc, fp, sections=sections)

def refPageTail(pageName, seeAlso, fp, auto = False):
    # This is difficult to get working properly in asciidoc
    # specURL = 'link:{xrspecpath}/xrspec.html'

    # Where to find the current all-extensions OpenXR HTML spec, so xrefs in
    # the asciidoc source that aren't to ref pages can link into it instead.
    specURL = 'https://www.khronos.org/registry/openxr/specs/1.0-extensions/html/xrspec.html'

    if seeAlso == None:
        seeAlso = 'No cross-references are available\n'

    notes = [
        'For more information, see the OpenXR Specification at URL',
        '',
        specURL + '#' + pageName,
        '',
        ]

    if auto:
        notes.extend([
            'This page is a generated document.',
            'Fixes and changes should be made to the generator scripts, '
            'not directly.',
            ])
    else:
        notes.extend([
            'This page is extracted from the OpenXR Specification. ',
            'Fixes and changes should be made to the Specification, '
            'not directly.',
            ])

    print('== See Also',
          '',
          seeAlso,
          '',
          sep='\n', file=fp)

    print('== Document Notes',
          '',
          '\n'.join(notes),
          '',
          sep='\n', file=fp)

    printFooter(fp)

# Extract a single reference page into baseDir
#   baseDir - base directory to emit page into
#   specDir - directory extracted page source came from
#   pi - pageInfo for this page relative to file
#   file - list of strings making up the file, indexed by pi
def emitPage(baseDir, specDir, pi, file):
    pageName = baseDir + '/' + pi.name + '.txt'

    # Add a dictionary entry for this page
    global genDict
    genDict[pi.name] = None
    logDiag('emitPage:', pageName)

    # Short description
    if pi.desc == None:
        pi.desc = '(no short description available)'

    # Not sure how this happens yet
    if pi.include == None:
        logWarn('emitPage:', pageName, 'INCLUDE == None, no page generated')
        return

    # Specification text
    lines = remapIncludes(file[pi.begin:pi.include+1], baseDir, specDir)
    specText = ''.join(lines)

    # Member/parameter list, if there is one
    field = None
    fieldText = None
    if pi.param != None:
        if pi.type == 'structs':
            field = 'Members'
        elif pi.type in ['protos', 'funcpointers']:
            field = 'Parameters'
        else:
            logWarn('emitPage: unknown field type:', pi.type,
                'for', pi.name)
        lines = remapIncludes(file[pi.param:pi.body], baseDir, specDir)
        fieldText = ''.join(lines)

    # Description text
    if pi.body != pi.include:
        lines = remapIncludes(file[pi.body:pi.end+1], baseDir, specDir)
        descText = ''.join(lines)
    else:
        descText = None
        logWarn('emitPage: INCLUDE == BODY, so description will be empty for', pi.name)
        if pi.begin != pi.include:
            logWarn('emitPage: Note: BEGIN != INCLUDE, so the description might be incorrectly located before the API include!')

    # Substitute xrefs to point at the main spec
    specLinksPattern = re.compile(r'<<([^>,]+)[,]?[ \t\n]*([^>,]*)>>')
    specLinksSubstitute = r"link:{html_spec_relative}#\1[\2]"
    specText, _ = specLinksPattern.subn(specLinksSubstitute, specText)
    if fieldText != None:
        fieldText, _ = specLinksPattern.subn(specLinksSubstitute, fieldText)
    if descText is not None:
        descText, _ = specLinksPattern.subn(specLinksSubstitute, descText)

    fp = open(pageName, 'w', encoding='utf-8')
    refPageHead(pi.name,
                pi.desc,
                specText,
                field, fieldText,
                descText,
                fp)
    refPageTail(pi.name, seeAlsoList(pi.name, pi.refs), fp, auto = False)
    fp.close()

# Autogenerate a single reference page in baseDir
# Script only knows how to do this for /enums/ pages, at present
#   baseDir - base directory to emit page into
#   pi - pageInfo for this page relative to file
#   file - list of strings making up the file, indexed by pi
def autoGenEnumsPage(baseDir, pi, file):
    pageName = baseDir + '/' + pi.name + '.txt'
    fp = open(pageName, 'w', encoding='utf-8')

    # Add a dictionary entry for this page
    global genDict
    genDict[pi.name] = None
    logDiag('autoGenEnumsPage:', pageName)

    # Short description
    if pi.desc == None:
        pi.desc = '(no short description available)'

    # Description text. Allow for the case where an enum definition
    # is not embedded.
    if not pi.embed:
        embedRef = ''
    else:
        embedRef = ''.join([
                        '  * The reference page for ',
                        macroPrefix(pi.embed),
                        ', where this interface is defined.\n' ])

    txt = ''.join([
        'For more information, see:\n\n',
        embedRef,
        '  * The See Also section for other reference pages using this type.\n',
        '  * The OpenXR Specification.\n' ])

    refPageHead(pi.name,
                pi.desc,
                ''.join(file[pi.begin:pi.include+1]),
                None, None,
                txt,
                fp)
    refPageTail(pi.name, seeAlsoList(pi.name, pi.refs), fp, auto = True)
    fp.close()

# Pattern to break apart a Vk*Flags{authorID} name, used in autoGenFlagsPage.
flagNamePat = re.compile(r'(?P<name>\w+)Flags(?P<author>[A-Z]*)')

# Autogenerate a single reference page in baseDir for a Vk*Flags type
#   baseDir - base directory to emit page into
#   flagName - Vk*Flags name
def autoGenFlagsPage(baseDir, flagName):
    pageName = baseDir + '/' + flagName + '.txt'
    fp = open(pageName, 'w', encoding='utf-8')

    # Add a dictionary entry for this page
    global genDict
    genDict[flagName] = None
    logDiag('autoGenFlagsPage:', pageName)

    # Short description
    matches = flagNamePat.search(flagName)
    if matches != None:
        name = matches.group('name')
        author = matches.group('author')
        logDiag('autoGenFlagsPage: split name into', name, 'Flags', author)
        flagBits = name + 'FlagBits' + author
        desc = 'Bitmask of ' + flagBits
    else:
        logWarn('autoGenFlagsPage:', pageName, 'does not end in "Flags{author ID}". Cannot infer FlagBits type.')
        flagBits = None
        desc = 'Unknown OpenXR flags type'

    # Description text
    if flagBits != None:
        txt = ''.join([
            'etext:' + flagName,
            ' is a mask of zero or more elink:' + flagBits + '.\n',
            'It is used as a member and/or parameter of the structures and commands\n',
            'in the See Also section below.\n' ])
    else:
        txt = ''.join([
            'etext:' + flagName,
            ' is an unknown OpenXR type, assumed to be a bitmask.\n' ])

    refPageHead(flagName,
                desc,
                'include::../api/flags/' + flagName + '.txt[]\n',
                None, None,
                txt,
                fp)
    refPageTail(flagName, seeAlsoList(flagName), fp, auto = True)
    fp.close()

# Autogenerate a single handle page in baseDir for a Vk* handle type
#   baseDir - base directory to emit page into
#   handleName - Vk* handle name
# @@ Need to determine creation function & add handles/ include for the
# @@ interface in generator.py.
def autoGenHandlePage(baseDir, handleName):
    pageName = baseDir + '/' + handleName + '.txt'
    fp = open(pageName, 'w', encoding='utf-8')

    # Add a dictionary entry for this page
    global genDict
    genDict[handleName] = None
    logDiag('autoGenHandlePage:', pageName)

    # Short description
    desc = 'OpenXR object handle'

    descText = ''.join([
        'sname:' + handleName,
        ' is an object handle type, referring to an object used\n',
        'by the OpenXR implementation. These handles are created or allocated\n',
        'by the xr @@ TBD @@ function, and used by other OpenXR structures\n',
        'and commands in the See Also section below.\n' ])

    refPageHead(handleName,
                desc,
                'include::../api/handles/' + handleName + '.txt[]\n',
                None, None,
                descText,
                fp)
    refPageTail(handleName, seeAlsoList(handleName), fp, auto = True)
    fp.close()

# Extract reference pages from a spec asciidoc source file
#   specFile - filename to extract from
#   baseDir - output directory to generate page in
#
def genRef(specFile, baseDir):
    file = loadFile(specFile)
    if file == None:
        return

    # Save the path to this file for later use in rewriting relative includes
    specDir = os.path.dirname(os.path.abspath(specFile))

    pageMap = findRefs(file, specFile)
    logDiag(specFile + ': found', len(pageMap.keys()), 'potential pages')

    sys.stderr.flush()

    # Fix up references in pageMap
    fixupRefs(pageMap, specFile, file)

    # Create each page, if possible

    for name in sorted(pageMap.keys()):
        pi = pageMap[name]

        printPageInfo(pi, file)

        if pi.Warning:
            logDiag('genRef:', pi.name + ':', pi.Warning)

        if pi.extractPage:
            emitPage(baseDir, specDir, pi, file)
        elif pi.type == 'enums':
            autoGenEnumsPage(baseDir, pi, file)
        elif pi.type == 'flags':
            autoGenFlagsPage(baseDir, pi.name)
        else:
            # Don't extract this page
            logWarn('genRef: Cannot extract or autogenerate:', pi.name)

# Generate baseDir/apispec.txt, the single-page version of the ref pages.
# This assumes there's a page for everything in the xrapi.py dictionaries.
# Extensions (KHR, EXT, etc.) are currently skipped
def genSinglePageRef(baseDir):
    # Accumulate head of page
    head = io.StringIO()

    printCopyrightSourceComments(head)

    print('= OpenXR API Reference Pages',
          ':data-uri:',
          ':icons: font',
          ':doctype: book',
          ':numbered!:',
          ':max-width: 200',
          ':data-uri:',
          ':toc2:',
          ':toclevels: 2',
          '',
          sep='\n', file=head)

    print('include::copyright-ccby.txt[]', file=head)
    print('', file=head)
    # Inject the table of contents. Asciidoc really ought to be generating
    # this for us.

    sections = [
        [ protos,       'protos',       'OpenXR Commands' ],
        [ handles,      'handles',      'Object Handles' ],
        [ structs,      'structs',      'Structures' ],
        [ enums,        'enums',        'Enumerations' ],
        [ flags,        'flags',        'Flags' ],
        [ funcpointers, 'funcpointers', 'Function Pointer Types' ],
        [ basetypes,    'basetypes',    'OpenXR Scalar types' ],
        [ defines,      'defines',      'C Macro Definitions' ],
        [ extensions,   'extensions',   'OpenXR Extensions' ] ]

    # Accumulate body of page
    body = io.StringIO()

    for (apiDict,label,title) in sections:
        # Add section title/anchor header to body
        anchor = '[[' + label + ',' + title + ']]'
        print(anchor,
              '== ' + title,
              '',
              ':leveloffset: 2',
              '',
              sep='\n', file=body)

        if label == 'extensions':
            # preserve order of extensions since we already sorted the way we want.
            keys = apiDict.keys()
        else:
            keys = sorted(apiDict.keys())

        for refPage in keys:
            # Add page to body
            if 'FlagBits' in refPage:
                # OpenXR does not create separate ref pages for FlagBits:
                # the FlagBits includes go in the Flags refpage.
                continue
            print('include::' + refPage + '.txt[]', file=body)

        print('\n' + ':leveloffset: 0' + '\n', file=body)

    # Write head and body to the output file
    pageName = baseDir + '/apispec.txt'
    fp = open(pageName, 'w', encoding='utf-8')

    print(head.getvalue(), file=fp, end='')
    print(body.getvalue(), file=fp, end='')

    head.close()
    body.close()
    fp.close()


def genExtension(baseDir, name, info):
    # Add a dictionary entry for this page
    global genDict
    genDict[name] = None
    declares = []
    elem = info.elem

    ext_type = elem.get('type')

    for required in elem.find('require'):
        req_name = required.get('name')
        if not req_name:
            # This isn't what we're looking for
            continue
        if req_name.endswith('_SPEC_VERSION') or req_name.endswith('_EXTENSION_NAME'):
            # Don't link to spec version or extension name - those ref pages aren't created.
            continue

        if required.get('extends'):
            # These are either extensions of enums,
            # or enum values: neither of which get a ref page.
            continue

        if req_name not in genDict:
            logWarn('ERROR: {} (in extension {}) does not have a ref page.'.format(req_name, name))

        declares.append(req_name)
    pageName = baseDir + '/' + name + '.txt'
    logDiag('genExtension:', pageName)

    fp = open(pageName, 'w', encoding='utf-8')

    sections = OrderedDict()
    sections['Specification'] = 'See link:{html_spec_relative}#%s[ %s] in the main specification for complete information.' % (
        name, name)
    refPageShell(name,
                 "{} extension".format(ext_type),
                 fp,
                 sections=sections,
                 tail_content=makeExtensionInclude(name))
    refPageTail(name, seeAlsoList(name, declares), fp, auto=True)
    fp.close()


if __name__ == '__main__':
    global genDict
    global extensions
    genDict = {}
    extensions = OrderedDict()

    parser = argparse.ArgumentParser()

    parser.add_argument('-diag', action='store', dest='diagFile',
                        help='Set the diagnostic file')
    parser.add_argument('-warn', action='store', dest='warnFile',
                        help='Set the warning file')
    parser.add_argument('-log', action='store', dest='logFile',
                        help='Set the log file for both diagnostics and warnings')
    parser.add_argument('-basedir', action='store', dest='baseDir',
                        default='man',
                        help='Set the base directory in which pages are generated')
    parser.add_argument('-noauto', action='store_true',
                        help='Don\'t generate inferred ref pages automatically')
    parser.add_argument('files', metavar='filename', nargs='*',
                        help='a filename to extract ref pages from')
    parser.add_argument('--version', action='version', version='%(prog)s 1.0')
    parser.add_argument('-extension', action='append',
                        default=[],
                        help='Specify an extension or extensions to add to targets')
    parser.add_argument('-registry', action='store',
                        default='registry/xr.xml',
                        help='Use specified registry file instead of xr.xml')

    results = parser.parse_args()

    setLogFile(True,  True, results.logFile)
    setLogFile(True, False, results.diagFile)
    setLogFile(False, True, results.warnFile)

    baseDir = results.baseDir

    for file in results.files:
        genRef(file, baseDir)

    # Now figure out which pages *weren't* generated from the spec.
    # This relies on the dictionaries of API constructs in xrapi.py.

    # For Flags (e.g. Vk*Flags types), it's easy to autogenerate pages.
    if not results.noauto:

        registry = Registry()
        registry.loadFile(results.registry)

        # Only extensions with a supported="..." attribute in this set
        # will be considered for extraction/generation.
        supported_strings = set('openxr')
        ext_names = set(
            [k for k, v in registry.extdict.items() if v.supported in supported_strings])

        desired_extensions = ext_names.intersection(set(results.extension))
        for prefix in ['XR_KHR', 'XR_KHX', 'XR_EXT', 'XR']:
            # Splits up into chunks, sorted within each chunk.
            filtered_extensions = sorted(
                [name for name in ext_names
                 if name.startswith(prefix) and name not in extensions])
            for name in filtered_extensions:
                extensions[name] = None
                genExtension(baseDir, name, registry.extdict[name])

        # autoGenFlagsPage is no longer needed because they are added to
        # the spec sources now.
        # for page in flags.keys():
            # if not (page in genDict.keys()):
                # autoGenFlagsPage(baseDir, page)

        # autoGenHandlePage is no longer needed because they are added to
        # the spec sources now.
        # for page in structs.keys():
        #    if typeCategory[page] == 'handle':
        #        autoGenHandlePage(baseDir, page)

        sections = [
            [ flags,        'Flag Types' ],
            [ enums,        'Enumerated Types' ],
            [ structs,      'Structures' ],
            [ protos,       'Prototypes' ],
            [ funcpointers, 'Function Pointers' ],
            [ basetypes,    'OpenXR Scalar Types' ],
            [ extensions,   'OpenXR Extensions'] ]

        for (apiDict,title) in sections:
            flagged = False
            for page in apiDict.keys():
                if not (page in genDict.keys()):
                    if not flagged:
                        logWarn(title, 'with no ref page generated:')
                        flagged = True
                    logWarn('    ', page)

        genSinglePageRef(baseDir)
