#!/usr/bin/python3
#
# Copyright (c) 2018-2019 Collabora, Ltd.
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
# Purpose:      This file contains tests for check_spec_links.py
#               that depend on the API being used.

import pytest
from check_spec_links import MessageId, makeMacroChecker
from test_check_spec_links import CheckerWrapper, allMessages, msgReplacement


@pytest.fixture
def ckr(capsys):
    """Fixture - add an arg named ckr to your test function to automatically get one passed to you."""
    return CheckerWrapper(capsys)


def test_openxr_refpage_mismatch(ckr):
    """Test the REFPAGE_MISMATCH message."""
    ckr.enabled([MessageId.REFPAGE_MISMATCH])
    # Should not error: The include is for FlagBits inside of refpage for Flags
    # which is special-cased to not error.
    assert(not ckr.check(
        """[open,refpage='MyFlags']
        --
        include::../../generated/api/enums/MyFlagBits.txt[]""").messages)
    assert(not ckr.check(
        """[open,refpage='MyFlags']
        --
        include::../../generated/validity/enums/MyFlagBits.txt[]""").messages)


def test_openxr_refpage_missing(ckr):
    """OpenXR-specific tests of the REFPAGE_MISSING message."""
    ckr.enabled([MessageId.REFPAGE_MISSING])

    # Should not error: all flags includes are stuck in the Appendix for now.
    assert(not ckr.check(
        "include::../../generated/api/flags/XrEndFrameDescriptionFlags.txt[]").messages)


def test_openxr_refpage_block(ckr):
    """OpenXR-specific tests of the REFPAGE_BLOCK message."""
    ckr.enabled([MessageId.REFPAGE_BLOCK])

    # Should have 1 error: line before '--' isn't tag
    assert(ckr.check(
        """--
        bla
        --""").numDiagnostics() == 1)

    # Should have 3 errors:
    #  - line before '--' isn't tag (refpage gets opened anyway),
    #  - tag is inside refpage (which auto-closes the previous refpage block, then re-opens a refpage)
    #  - line after tag isn't '--'
    result = ckr.check(
        """--
        [open,]
        bla
        --""")
    assert(result.numDiagnostics() == 3)
    # Internally, it's as if the following were the spec source, after putting in the "fake" lines
    # (each of the added lines comes from one message):
    #
    # [open,]
    # --
    # --
    # [open,]
    # --
    # bla
    # --
    assert(len([x for x in allMessages(result)
                if "but did not find, a line containing only -- following a reference page tag" in x]) == 1)
    assert(len([x for x in allMessages(result)
                if "containing only -- outside of a reference page block" in x]) == 1)
    assert(len([x for x in allMessages(result)
                if "we are already in a refpage block" in x]) == 1)
