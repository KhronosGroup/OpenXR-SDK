#!/usr/bin/env python3
# Copyright (c) 2019 The Khronos Group Inc.

from itertools import product

from shared import (BITS, TRUE_FALSE, VS_VERSIONS, make_win_artifact_name,
                    output_json)

if __name__ == "__main__":

    configs = {}
    for vsver, bits, debug, dynamic in product(VS_VERSIONS.keys(), BITS, (False,), TRUE_FALSE):
        label = [str(vsver)]
        config = []
        generator = VS_VERSIONS[vsver]
        if bits == 64:
            config.append('-A x64')
        else:
            config.append('-A Win32')
        label.append(str(bits))
        if dynamic:
            label.append('dynamic')
            config.append('-DDYNAMIC_LOADER=ON')
        else:
            label.append('static')
            config.append('-DDYNAMIC_LOADER=OFF')
        if debug:
            label.append('debug')
        name = '_'.join(label)
        configs[name] = {
            'generator': generator,
            'buildType': 'Debug' if debug else 'RelWithDebInfo',
            'cmakeArgs': ' '.join(config),
            'dynamic': dynamic,
            'bits': bits
        }
        if not debug:
            configs[name]['artifactName'] = make_win_artifact_name(
                vsver, dynamic, bits)

    output_json(configs)
