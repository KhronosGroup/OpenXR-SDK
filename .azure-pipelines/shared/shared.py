# Copyright (c) 2019 The Khronos Group Inc.
# SPDX-License-Identifier: Apache-2.0

import json
import sys

VS_VERSION = 'Visual Studio 16 2019'

PLATFORMS = ('Win32', 'x64', 'ARM', 'ARM64')

TRUE_FALSE = (True, False)


def make_win_artifact_name(platform, uwp):
    return 'loader_{}{}'.format(
        platform.lower(),
        '_uwp' if uwp else '',
    )


def output_json(data):
    if len(sys.argv) == 2:
        print(
            "##vso[task.setVariable variable={};isOutput=true]{}".format(sys.argv[1], json.dumps(data)))
    else:
        print(json.dumps(data, indent=4))
