#!/usr/bin/env python3
# Copyright (c) 2019 The Khronos Group Inc.

from itertools import product
from pathlib import Path
import sys

from shared import BITS, TRUE_FALSE, VS_VERSIONS, make_win_artifact_name

CWD = Path.cwd()


def move(src, dest):

    print(str(src), '->', str(dest))
    src.replace(dest)


if __name__ == "__main__":

    configs = {}
    workspace = Path(sys.argv[1])
    outbase = Path(sys.argv[2])

    for vsver, dynamic in product(VS_VERSIONS.keys(), TRUE_FALSE):
        base = outbase / 'msvs{}_{}'.format(vsver,
                                            'dynamic' if dynamic else 'static')
        base.mkdir(parents=True, exist_ok=True)
        name_64 = make_win_artifact_name(vsver, dynamic, 64)
        name_32 = make_win_artifact_name(vsver, dynamic, 32)
        artifact_64 = workspace / name_64
        artifact_32 = workspace / name_32
        # Move over one set of includes
        move(artifact_32 / 'include', base / 'include')

        # lib files
        move(artifact_32 / 'lib', base / 'lib32')
        move(artifact_64 / 'lib', base / 'lib')

        if dynamic:
            # dll files
            move(artifact_32 / 'bin', base / 'bin32')
            move(artifact_64 / 'bin', base / 'bin')
