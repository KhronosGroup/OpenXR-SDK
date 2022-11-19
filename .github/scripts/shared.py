# Copyright (c) 2019 The Khronos Group Inc.
# SPDX-License-Identifier: Apache-2.0

import json
import sys
from dataclasses import dataclass
from itertools import product

VS_VERSION = "Visual Studio 17 2022"

PLATFORMS = ("Win32", "x64", "ARM", "ARM64")

TRUE_FALSE = (True, False)


@dataclass
class BuildConfig:
    arch: str
    uwp: bool

    def should_skip(self) -> bool:
        # ARM/ARM64 is only built for the UWP platform.
        return "ARM" in self.arch and not self.uwp

        # can switch to just doing x64 for speed of testing
        # return self.arch != "x64"

    def preset(self) -> str:
        if self.uwp:
            return f"{self.arch.lower()}_uwp"

        return self.arch.lower()

    def win_artifact_name(self) -> str:
        return f"loader_{self.preset()}"

    def platform_dirname(self) -> str:
        if self.uwp:
            return f"{self.arch}_uwp"
        return self.arch


_UNFILTERED_BUILD_CONFIGS = [
    BuildConfig(arch, uwp) for arch, uwp in product(PLATFORMS, TRUE_FALSE)
]

BUILD_CONFIGS = [c for c in _UNFILTERED_BUILD_CONFIGS if not c.should_skip()]


def output_json(data, variable_name=None):
    if variable_name:
        print(f"::set-output name={variable_name}::{json.dumps(data)}")
    else:
        print(json.dumps(data, indent=4))
