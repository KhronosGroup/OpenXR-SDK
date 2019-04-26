#!/usr/bin/python3
#
# Copyright (c) 2018 Collabora, Ltd.
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
# Author(s):    Ryan Pavlik <ryan.pavlik@collabora.com>
#
# Purpose:      This script searches for and extracts embedded source code
#               from specification chapters.

import argparse
import errno
import re
from abc import ABC, abstractmethod
from enum import Enum, unique
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
ALL_DOCS = sorted((ROOT / 'specification/sources/').glob('**/*.adoc'))

CODEDIR = ROOT / 'specification/example-builds'
GENCODEDIR = CODEDIR / 'generated'
# if it already exists, that's OK
try:
    GENCODEDIR.mkdir(parents=True)
except OSError as e:
    if e.errno != errno.EEXIST:
        raise


class LinewiseFileProcessor(ABC):
    def __init__(self):
        self._lines = []
        self._lineNum = 0
        self._line = ''
        self._filename = Path()

    @property
    def filename(self):
        """The Path object of the currently processed file"""
        return self._filename

    @property
    def relativeFilename(self):
        """The current file's Path relative to the current working directory"""
        return self.filename.relative_to(Path('.').resolve())

    @property
    def line(self):
        """The current line, including any trailing whitespace and the line ending."""
        return self._line

    @property
    def lineNumber(self):
        """Get 1-indexed line number."""
        return self._lineNum

    @property
    def rstrippedLine(self):
        """The current line without any trailing whitespace."""
        return self.line.rstrip()

    @property
    def trailingWhitespace(self):
        """The trailing whitespace of the current line that gets removed when accessing rstrippedLine"""
        nonWhitespaceLength = len(self.rstrippedLine)
        return self.line[nonWhitespaceLength:]

    def precedingLine(self, relativeIndex):
        """Retrieve the line at an line number at the given relative index, if one exists. Returns None if there is no line there."""
        if relativeIndex >= 0:
            raise RuntimeError(
                'relativeIndex must be negative, to retrieve a preceding line.')
        if relativeIndex + self.lineNumber <= 0:
            # There is no line at this index
            return None
        return self._lines[self.lineNumber + relativeIndex - 1]

    def getPrecedingLines(self, num):
        """Get *up to* the preceding num lines. Fewer may be returned if the requested number aren't available."""
        return self._lines[- (num + 1):-1]

    @abstractmethod
    def processLine(self, lineNum, line):
        """Implement in your subclass to handle each new line."""
        raise NotImplementedError

    def _internalProcessFile(self, fileHandle):

        for lineNum, line in enumerate(fileHandle, 1):
            self._lineNum = lineNum
            self._line = line
            self._lines.append(line)
            self.processLine(lineNum, line)

    def processFile(self, filename, fileHandle=None):
        """Main entry point - call with a filename and optionally the file handle to read from."""
        if isinstance(filename, str):
            filename = Path(filename).resolve()

        self._filename = filename

        if fileHandle:
            self._internalProcessFile(fileHandle)
        else:
            with self._filename.open('r', encoding='utf-8') as f:
                self._internalProcessFile(f)


@unique
class Language(Enum):
    C = 'C'
    CPP = 'C++'
    XML = 'XML'
    ASCIIDOC = 'asciidoc'
    JSON = 'JSON'
    SH = 'sh'

    UNKNOWN = 'UNKNOWN'

    def __str__(self):
        return self.value

    @property
    def extension(self):
        if self == Language.UNKNOWN:
            raise RuntimeError(
                "Can't get extension for UNKNOWN language")
        if self == Language.ASCIIDOC:
            return 'adoc'
        return str(self).lower().replace('+', 'p')

    @classmethod
    def from_string(self, s):
        s = s.upper()
        for val in Language:
            if s == str(val).upper():
                return val
        return Language.UNKNOWN


class CodeExtractor(LinewiseFileProcessor):
    def __init__(self, outputLineNumbers=False, quiet=False):
        super().__init__()
        self.MIN_LINES = 5

        self.outputLineNumbers = outputLineNumbers
        self.quiet = quiet

        self.nextSnippetId = 0
        self.inCodeBlock = False
        self.blockPattern = re.compile(r'\[source(,?)(?P<tags>.*)\]')
        self.languagesToExtract = set((Language.CPP, Language.C))
        self.codeLines = None

        self.generatedFiles = []

        # list of (generated file path, include path) pairs
        self.deps = []

        # key: generated file path
        # value: line number where a code snippet starts
        self.origins = {}

    def getUniqueId(self):
        ret = self.nextSnippetId
        self.nextSnippetId += 1
        return ret

    def makeNumberedFilename(self, language):
        name = self.filename.with_suffix('.{num}.{ext}'.format(
            num=self.getUniqueId(), ext=language.extension)).name
        return GENCODEDIR / name

    def printMessage(self, s):
        if not self.quiet:
            print('{}:{}: {}'.format(self.filename, self.lineNumber, s))

    def processStartOfCodeBlock(self):
        prevLine = self.precedingLine(-1)
        if not prevLine:
            # No previous line to find language.
            return

        codeBlockTag = self.blockPattern.match(prevLine.rstrip())
        if not codeBlockTag:
            # Not going to handle this.
            return

        tags = set(codeBlockTag.group('tags').upper().split(','))

        self.language = Language.UNKNOWN
        for lang in Language:
            if str(lang).upper() in tags:
                self.language = lang
                break
        if self.language == Language.UNKNOWN:
            self.printMessage('Not extracting code snippet introduced with {} (tags = {})'.format(
                codeBlockTag.group(), tags))
            return

        if self.language not in self.languagesToExtract:
            self.printMessage('Not extracting code snippet identified as {}'.format(
                self.language))
            return
        if 'SUPPRESS-BUILD' in tags:
            self.printMessage(
                'Suppressing extraction of code snippet because we saw "suppress-build"')
            return

        # self.printMessage('Considering code snippet identified as {}'.format(
        #     self.language))
        self.codeLines = []
        self.startOfCodeBlock = self.lineNumber

    def processEndOfCodeBlock(self):
        if self.codeLines is not None:
            codeLines = self.codeLines
            self.codeLines = None

            if len(codeLines) < self.MIN_LINES:
                self.printMessage(
                    'Not extracting code snippet - only {} lines.'.format(len(codeLines)))
                return

            outFilename = self.makeNumberedFilename(self.language)
            self.printMessage('Writing {} extracted lines to file {}\n'.format(
                len(codeLines), outFilename.relative_to(Path('.').resolve())))
            self.generatedFiles.append(outFilename)

            self.origins[outFilename] = self.startOfCodeBlock

            includeFile = CODEDIR / outFilename.with_suffix('.h').name

            with outFilename.open('w', encoding='utf-8') as f:
                f.write('#include "common_include.h"\n')
                if includeFile.exists():
                    f.write('#include "{}"\n\n'.format(includeFile.name))
                    self.deps.append((outFilename, includeFile))
                f.write('void func() {\n')
                f.write(''.join(codeLines))
                f.write('\n}\n')

    def processCodeBlockLine(self):
        if self.codeLines is not None:
            if self.outputLineNumbers:
                self.codeLines.append('# {} "{}"\n'.format(
                    self.lineNumber, self.filename))
            self.codeLines.append(self.line)

    def processLine(self, lineNum, line):
        if line.startswith('---'):
            # Toggle code block status.
            self.inCodeBlock = not self.inCodeBlock

            if self.inCodeBlock:
                # We just started a code block
                self.processStartOfCodeBlock()
            else:
                # We just ended one.
                self.processEndOfCodeBlock()

        elif self.inCodeBlock:
            self.processCodeBlockLine()


class CodeExtractorGroup(object):
    def __init__(self, outputLineNumbers=False, quiet=False):
        self.outputLineNumbers = outputLineNumbers
        self.quiet = quiet

        # key: adoc file path. value: list of generated source files.
        self.generatedFiles = {}

        # all generated sources files
        self.allGenerated = []

        # list of (generated file path, include path) pairs
        self.deps = []

        # key: generated file path
        # value: (adoc file path, line number where a code snippet starts) pair
        self.origins = {}

    def process(self, files):
        for fn in files:
            extractor = CodeExtractor(outputLineNumbers=self.outputLineNumbers,
                                      quiet=self.quiet)
            extractor.processFile(fn)

            if extractor.generatedFiles:
                self.generatedFiles[extractor.filename] = extractor.generatedFiles
                self.allGenerated.extend(extractor.generatedFiles)
                self.deps.extend(extractor.deps)
                self.origins.update({fn: (extractor.filename, lineNum)
                                     for fn, lineNum in extractor.origins.items()})

    def outputMakefile(self, makefile):
        with open(makefile, 'w', encoding='utf-8') as f:

            generated_c_string = ' \\\n'.join(str(fn)
                                              for fn in self.allGenerated if fn.suffix == '.c')
            generated_cpp_string = ' \\\n'.join(str(fn)
                                                for fn in self.allGenerated if fn.suffix == '.cpp')
            deps_string = '\n'.join('{}: {} $(CODEDIR)/common_include.h'.format(fn.with_suffix('.o'), dep)
                                    for fn, dep in self.deps)
            extra_arg = ''
            if self.outputLineNumbers:
                extra_arg = '--linenumbers'
            f.write("""
OUTDIR  ?= $(CURDIR)/{out}
CODEDIR ?= $(CURDIR)/{codedir}
PYTHON   ?= python3
QUIET    ?= @

GENERATED_C := {c}
C_OBJECTS := $(patsubst %.c,%.o,$(GENERATED_C))

GENERATED_CPP := {cpp}
CPP_OBJECTS := $(patsubst %.cpp,%.o,$(GENERATED_CPP))

build-examples: $(C_OBJECTS) $(CPP_OBJECTS)
.PHONY: build-examples

clean-examples:
\trm -f $(C_OBJECTS) $(CPP_OBJECTS)
.PHONY: clean-examples

$(C_OBJECTS) : %.o : %.c $(OUTDIR)/openxr/openxr.h
\t@echo '$(ORIGIN)'
\t$(QUIET)gcc -std=gnu99 -c -I$(OUTDIR) -I$(CODEDIR) $< -o $@

$(CPP_OBJECTS) : %.o : %.cpp $(OUTDIR)/openxr/openxr.h
\t@echo '$(ORIGIN)'
\t$(QUIET)g++ -std=gnu++11 -c -I$(OUTDIR) -I$(CODEDIR) $< -o $@

ifeq ($(strip $(QUIET)),@)
EXTRACT_QUIET := --quiet
endif

$(GENERATED_C) $(GENERATED_CPP) {makefile}: {script} {inputs}
\t$(QUIET)$(PYTHON) $< {extra} --makefile={makefile} $(EXTRACT_QUIET)

gen: {script}
\t$(QUIET)$(PYTHON) $< {extra} --makefile={makefile} $(EXTRACT_QUIET)
.PHONY: gen

{deps}
""".format(out=(ROOT / 'specification/out/prerelease').relative_to(Path('.').resolve()),
                codedir=CODEDIR.relative_to(Path('.').resolve()),
                c=generated_c_string,
                cpp=generated_cpp_string,
                makefile=makefile,
                script=Path(__file__),
                extra=extra_arg,
                inputs=' '.join(str(infile) for infile in self.generatedFiles),
                deps=deps_string))
            for fn, gen in self.generatedFiles.items():
                f.write('{stem}: {files}\n.PHONY: {stem}\n'.format(
                    stem=fn.stem, files=' '.join(str(g.with_suffix('.o')) for g in gen)))
            width = max(len(generated.name) for generated in self.origins)

            for generated, origin in self.origins.items():
                pad = ' ' * (width - len(generated.name))
                originFile, originLine = origin
                if generated.suffix == '.cpp':
                    compiler = '[c++] '
                else:
                    compiler = '[cc]  '
                originStr = '{} {} {} extracted from {}:{}'.format(compiler, generated.name, pad,
                                                                   originFile, originLine)
                f.write('{obj}: ORIGIN := {originstr}\n'.format(
                    obj=generated.with_suffix('.o'), originstr=originStr))


parser = argparse.ArgumentParser()
parser.add_argument("file",
                    help="Only extract from the indicated file(s). By default, all chapters and extensions are examined.",
                    nargs="*")
parser.add_argument("--linenumbers",
                    help='Add lines of the form "# linenumber filename" to the output, for build errors/warnings that point to the adoc files.',
                    action='store_true')
parser.add_argument('--makefile',
                    help='Output a makefile with a build-examples target (and matching clean-examples target).',
                    type=str)
parser.add_argument('--quiet', '-q',
                    help="Don't output debug information about what we are extracting and not extracting.",
                    action='store_true')
# type=argparse.FileType('w', encoding='UTF-8'))
args = parser.parse_args()


if args.file:
    files = [Path(f).resolve() for f in args.file]
else:
    files = ALL_DOCS

extractors = CodeExtractorGroup(outputLineNumbers=args.linenumbers,
                                quiet=args.quiet)
extractors.process(files)

if args.makefile:
    extractors.outputMakefile(args.makefile)
