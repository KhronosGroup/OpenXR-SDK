#!/usr/bin/python3
#
# Copyright (c) 2017 The Khronos Group Inc.
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

import sys, getopt, os, platform

cur_layer_json_version = '1.0.0'

# Get the relative path of a JSON's library file using the JSON
# file itself as a starting point.
#   json_file       the JSON file we're creating
#   library_file    the library (so/dll) file we're referencing in the JSON file
def getRelativePath(json_file, library_file):
    directory_slash = '/'
    relative_path = ''
    relative_list = []
    up_folder_count = 0
    json_path = os.path.normpath(json_file)
    json_split = json_path.split(os.sep)
    json_len = len(json_split)
    lib_path = os.path.normpath(library_file)
    lib_split = lib_path.split(os.sep)
    lib_len = len(lib_split)

    smallest_count = json_len - 1
    if lib_len < smallest_count:
        smallest_count = lib_len - 1

    cur_folder = 0
    diff_base_found = False
    for i in range(smallest_count):
        if json_split[i] != lib_split[i]:
            diff_base_found = True

        if diff_base_found:
            up_folder_count = up_folder_count + 1
            relative_list.append(lib_split[i])

        cur_folder = cur_folder + 1

    if json_len - 1 > smallest_count:
        up_folder_count += json_len - smallest_count - 1

    for i in range(smallest_count, lib_len):
        relative_list.append(lib_split[i])

    if up_folder_count == 0:
        relative_path += '.'
        relative_path += directory_slash
    else:
        for i in range(up_folder_count):
            relative_path += '..'
            relative_path += directory_slash

    for i in range(len(relative_list)):
        if i > 0:
            relative_path += directory_slash
        relative_path += relative_list[i]

    print(relative_path)
    return relative_path

def main(argv):
    output_file = ''
    layer_name = ''
    library_location = ''
    api_version = ''
    implementation_version = ''
    description = ''
    generate_badjson_jsons = False

    usage =  '\ngenerate_api_layer_manifest.py <ARGS>\n'
    usage += '    -f/--file <filename>\n'
    usage += '    -n/--name <layer name>\n'
    usage += '    -l/--lib <library_location>\n'
    usage += '    -a/--api <OpenXR API version>\n'
    usage += '    -v/--ver <layer implementation version>\n'
    usage += '    -d/--desc <Description>\n'
    usage += '    -b/--bad\n'

    try:
        opts, _ = getopt.getopt(argv,"hbf:n:l:a:v:d:",["bad","file=","name=","lib=","api=","ver=","desc="])
    except getopt.GetoptError:
        print(usage)
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print(usage)
            sys.exit()
        elif opt in ("-f", "--file"):
            output_file = arg.strip()
        elif opt in ("-n", "--name"):
            layer_name = arg.strip()
        elif opt in ("-l", "--lib"):
            library_location = arg.strip()
        elif opt in ("-a", "--api"):
            api_version = arg.strip()
        elif opt in ("-v", "--ver"):
            implementation_version = arg.strip()
        elif opt in ("-d", "--desc"):
            description = arg.strip()
        elif opt in ("-b", "--bad"):
            generate_badjson_jsons = True

    file_text  = '{\n'
    file_text += '    "file_format_version": "%s",\n' % cur_layer_json_version
    file_text += '    "api_layer": {\n'
    file_text += '        "name": "XR_APILAYER_LUNARG_%s",\n' % layer_name
    file_text += '        "library_path": "%s",\n' % library_location
    file_text += '        "api_version": "%s",\n' % api_version
    file_text += '        "implementation_version": "%s",\n' % implementation_version
    file_text += '        "description": "%s"' % description

    # If testing bad JSONs, then add in a fake extension
    if generate_badjson_jsons:
        file_text += ',\n'
        file_text += '        "instance_extensions": [\n'
        file_text += '            {\n'
        file_text += '                "name": "XR_KHR_fake_ext2",\n'
        file_text += '                "spec_version": "12"\n'
        file_text += '            },\n'
        file_text += '            {\n'
        file_text += '                "name": "XR_KHR_fake_ext3",\n'
        file_text += '                "spec_version": "42"\n'
        file_text += '            }\n'
        file_text += '        ]'
    file_text += '\n'
    file_text += '    }\n'
    file_text += '}\n'
    f = open(output_file, 'w')
    f.write(file_text)
    f.close()

    if generate_badjson_jsons:
        # Bad File format versions
        ####################################

        # Missing
        bad_name = '_badjson_file_ver_missing'
        file_text  = '{\n'
        file_text += '    "api_layer": {\n'
        file_text += '        "name": "XR_APILAYER_LUNARG_%s%s",\n' % (layer_name, bad_name)
        file_text += '        "library_path": "%s",\n' % library_location
        file_text += '        "api_version": "%s",\n' % api_version
        file_text += '        "implementation_version": "%s",\n' % implementation_version
        file_text += '        "description": "%s"\n' % description
        file_text += '    }\n'
        file_text += '}\n'
        bad_name += '.json'
        bad_file = output_file.replace(".json", bad_name)
        f = open(bad_file, 'w')
        f.write(file_text)
        f.close()

        # Use int
        bad_name = '_badjson_file_ver_int'
        file_text  = '{\n'
        file_text += '    "file_format_version": 1,\n'
        file_text += '    "api_layer": {\n'
        file_text += '        "name": "XR_APILAYER_LUNARG_%s%s",\n' % (layer_name, bad_name)
        file_text += '        "library_path": "%s",\n' % library_location
        file_text += '        "api_version": "%s",\n' % api_version
        file_text += '        "implementation_version": "%s",\n' % implementation_version
        file_text += '        "description": "%s"\n' % description
        file_text += '    }\n'
        file_text += '}\n'
        bad_name += '.json'
        bad_file = output_file.replace(".json", bad_name)
        f = open(bad_file, 'w')
        f.write(file_text)
        f.close()

        # Use invalid string
        bad_name = '_badjson_file_ver_string'
        file_text  = '{\n'
        file_text += '    "file_format_version": "invalid string",\n'
        file_text += '    "api_layer": {\n'
        file_text += '        "name": "XR_APILAYER_LUNARG_%s%s",\n' % (layer_name, bad_name)
        file_text += '        "library_path": "%s",\n' % library_location
        file_text += '        "api_version": "%s",\n' % api_version
        file_text += '        "implementation_version": "%s",\n' % implementation_version
        file_text += '        "description": "%s"\n' % description
        file_text += '    }\n'
        file_text += '}\n'
        bad_name += '.json'
        bad_file = output_file.replace(".json", bad_name)
        f = open(bad_file, 'w')
        f.write(file_text)
        f.close()

        # Too low of a version
        bad_name = '_badjson_file_ver_all_low'
        file_text  = '{\n'
        file_text += '    "file_format_version": "0.0.0",\n'
        file_text += '    "api_layer": {\n'
        file_text += '        "name": "XR_APILAYER_LUNARG_%s%s",\n' % (layer_name, bad_name)
        file_text += '        "library_path": "%s",\n' % library_location
        file_text += '        "api_version": "%s",\n' % api_version
        file_text += '        "implementation_version": "%s",\n' % implementation_version
        file_text += '        "description": "%s"\n' % description
        file_text += '    }\n'
        file_text += '}\n'
        bad_name += '.json'
        bad_file = output_file.replace(".json", bad_name)
        f = open(bad_file, 'w')
        f.write(file_text)
        f.close()

        # Too high of a major version
        bad_name = '_badjson_file_ver_major_high'
        file_text  = '{\n'
        file_text += '    "file_format_version": "15.0.0",\n'
        file_text += '    "api_layer": {\n'
        file_text += '        "name": "XR_APILAYER_LUNARG_%s%s",\n' % (layer_name, bad_name)
        file_text += '        "library_path": "%s",\n' % library_location
        file_text += '        "api_version": "%s",\n' % api_version
        file_text += '        "implementation_version": "%s",\n' % implementation_version
        file_text += '        "description": "%s"\n' % description
        file_text += '    }\n'
        file_text += '}\n'
        bad_name += '.json'
        bad_file = output_file.replace(".json", bad_name)
        f = open(bad_file, 'w')
        f.write(file_text)
        f.close()

        # Too high of a minor version
        bad_name = '_badjson_file_ver_minor_high'
        file_text  = '{\n'
        file_text += '    "file_format_version": "1.15.0",\n'
        file_text += '    "api_layer": {\n'
        file_text += '        "name": "XR_APILAYER_LUNARG_%s%s",\n' % (layer_name, bad_name)
        file_text += '        "library_path": "%s",\n' % library_location
        file_text += '        "api_version": "%s",\n' % api_version
        file_text += '        "implementation_version": "%s",\n' % implementation_version
        file_text += '        "description": "%s"\n' % description
        file_text += '    }\n'
        file_text += '}\n'
        bad_name += '.json'
        bad_file = output_file.replace(".json", bad_name)
        f = open(bad_file, 'w')
        f.write(file_text)
        f.close()

        # No layer section
        ####################################

        # Completely Missing
        bad_name = '_badjson_layer_missing'
        file_text  = '{\n'
        file_text += '    "file_format_version": "%s",\n' % cur_layer_json_version
        file_text += '    "name": "XR_APILAYER_LUNARG_%s%s",\n' % (layer_name, bad_name)
        file_text += '    "library_path": "%s",\n' % library_location
        file_text += '    "api_version": "%s",\n' % api_version
        file_text += '    "implementation_version": "%s",\n' % implementation_version
        file_text += '    "description": "%s"\n' % description
        file_text += '}\n'
        bad_name += '.json'
        bad_file = output_file.replace(".json", bad_name)
        f = open(bad_file, 'w')
        f.write(file_text)
        f.close()

        # Empty
        bad_name = '_badjson_layer_empty'
        file_text  = '{\n'
        file_text += '    "file_format_version": "%s",\n' % cur_layer_json_version
        file_text += '    "api_layer": {\n'
        file_text += '    },\n'
        file_text += '    "name": "XR_APILAYER_LUNARG_%s%s",\n' % (layer_name, bad_name)
        file_text += '    "library_path": "%s",\n' % library_location
        file_text += '    "api_version": "%s",\n' % api_version
        file_text += '    "implementation_version": "%s",\n' % implementation_version
        file_text += '    "description": "%s"\n' % description
        file_text += '}\n'
        bad_name += '.json'
        bad_file = output_file.replace(".json", bad_name)
        f = open(bad_file, 'w')
        f.write(file_text)
        f.close()

        # Bad Layer Name
        ####################################

        # Use int
        bad_name = '_badjson_name_int'
        file_text  = '{\n'
        file_text += '    "file_format_version": "%s",\n' % cur_layer_json_version
        file_text += '    "api_layer": {\n'
        file_text += '        "name": 1,\n'
        file_text += '        "library_path": "%s",\n' % library_location
        file_text += '        "api_version": "%s",\n' % api_version
        file_text += '        "implementation_version": "%s",\n' % implementation_version
        file_text += '        "description": "%s"\n' % description
        file_text += '    }\n'
        file_text += '}\n'
        bad_name += '.json'
        bad_file = output_file.replace(".json", bad_name)
        f = open(bad_file, 'w')
        f.write(file_text)
        f.close()

        # Missing
        bad_name = '_badjson_name_missing'
        file_text  = '{\n'
        file_text += '    "file_format_version": "%s",\n' % cur_layer_json_version
        file_text += '    "api_layer": {\n'
        file_text += '        "library_path": "%s",\n' % library_location
        file_text += '        "api_version": "%s",\n' % api_version
        file_text += '        "implementation_version": "%s",\n' % implementation_version
        file_text += '        "description": "%s"\n' % description
        file_text += '    }\n'
        file_text += '}\n'
        bad_name += '.json'
        bad_file = output_file.replace(".json", bad_name)
        f = open(bad_file, 'w')
        f.write(file_text)
        f.close()

        # Bad path
        ####################################

        # Missing
        bad_name = '_badjson_path_missing'
        file_text  = '{\n'
        file_text += '    "file_format_version": "%s",\n' % cur_layer_json_version
        file_text += '    "api_layer": {\n'
        file_text += '        "name": "XR_APILAYER_LUNARG_%s%s",\n' % (layer_name, bad_name)
        file_text += '        "api_version": "%s",\n' % api_version
        file_text += '        "implementation_version": "%s",\n' % implementation_version
        file_text += '        "description": "%s"\n' % description
        file_text += '    }\n'
        file_text += '}\n'
        bad_name += '.json'
        bad_file = output_file.replace(".json", bad_name)
        f = open(bad_file, 'w')
        f.write(file_text)
        f.close()

        # Use int
        bad_name = '_badjson_path_int'
        file_text  = '{\n'
        file_text += '    "file_format_version": "%s",\n' % cur_layer_json_version
        file_text += '    "api_layer": {\n'
        file_text += '        "name": "XR_APILAYER_LUNARG_%s%s",\n' % (layer_name, bad_name)
        file_text += '        "library_path": 1,\n'
        file_text += '        "api_version": "%s",\n' % api_version
        file_text += '        "implementation_version": "%s",\n' % implementation_version
        file_text += '        "description": "%s"\n' % description
        file_text += '    }\n'
        file_text += '}\n'
        bad_name += '.json'
        bad_file = output_file.replace(".json", bad_name)
        f = open(bad_file, 'w')
        f.write(file_text)
        f.close()

        # Replace valid path with invalid one
        bad_name = '_badjson_path_no_file'
        file_text  = '{\n'
        file_text += '    "file_format_version": "%s",\n' % cur_layer_json_version
        file_text += '    "api_layer": {\n'
        file_text += '        "name": "XR_APILAYER_LUNARG_%s%s",\n' % (layer_name, bad_name)
        file_text += '        "library_path": "%s",\n' % library_location.replace("test_layers","not_real")
        file_text += '        "api_version": "%s",\n' % api_version
        file_text += '        "implementation_version": "%s",\n' % implementation_version
        file_text += '        "description": "%s"\n' % description
        file_text += '    }\n'
        file_text += '}\n'
        bad_name += '.json'
        bad_file = output_file.replace(".json", bad_name)
        f = open(bad_file, 'w')
        f.write(file_text)
        f.close()

        # Bad API Version
        ####################################

        # Missing
        bad_name = '_badjson_api_int'
        file_text  = '{\n'
        file_text += '    "file_format_version": "%s",\n' % cur_layer_json_version
        file_text += '    "api_layer": {\n'
        file_text += '        "name": "XR_APILAYER_LUNARG_%s%s",\n' % (layer_name, bad_name)
        file_text += '        "library_path": "%s",\n' % library_location
        file_text += '        "implementation_version": "%s",\n' % implementation_version
        file_text += '        "description": "%s"\n' % description
        file_text += '    }\n'
        file_text += '}\n'
        bad_name += '.json'
        bad_file = output_file.replace(".json", bad_name)
        f = open(bad_file, 'w')
        f.write(file_text)
        f.close()

        # Use int
        bad_name = '_badjson_api_int'
        file_text  = '{\n'
        file_text += '    "file_format_version": "%s",\n' % cur_layer_json_version
        file_text += '    "api_layer": {\n'
        file_text += '        "name": "XR_APILAYER_LUNARG_%s%s",\n' % (layer_name, bad_name)
        file_text += '        "library_path": "%s",\n' % library_location
        file_text += '        "api_version": 1,\n'
        file_text += '        "implementation_version": "%s",\n' % implementation_version
        file_text += '        "description": "%s"\n' % description
        file_text += '    }\n'
        file_text += '}\n'
        bad_name += '.json'
        bad_file = output_file.replace(".json", bad_name)
        f = open(bad_file, 'w')
        f.write(file_text)
        f.close()

        # Use float
        bad_name = '_badjson_api_float'
        file_text  = '{\n'
        file_text += '    "file_format_version": "%s",\n' % cur_layer_json_version
        file_text += '    "api_layer": {\n'
        file_text += '        "name": "XR_APILAYER_LUNARG_%s%s",\n' % (layer_name, bad_name)
        file_text += '        "library_path": "%s",\n' % library_location
        file_text += '        "api_version": 1.0,\n'
        file_text += '        "implementation_version": "%s",\n' % implementation_version
        file_text += '        "description": "%s"\n' % description
        file_text += '    }\n'
        file_text += '}\n'
        bad_name += '.json'
        bad_file = output_file.replace(".json", bad_name)
        f = open(bad_file, 'w')
        f.write(file_text)
        f.close()

        # Bad string
        bad_name = '_badjson_api_string'
        file_text  = '{\n'
        file_text += '    "file_format_version": "%s",\n' % cur_layer_json_version
        file_text += '    "api_layer": {\n'
        file_text += '        "name": "XR_APILAYER_LUNARG_%s%s",\n' % (layer_name, bad_name)
        file_text += '        "library_path": "%s",\n' % library_location
        file_text += '        "api_version": "string",\n'
        file_text += '        "implementation_version": "%s",\n' % implementation_version
        file_text += '        "description": "%s"\n' % description
        file_text += '    }\n'
        file_text += '}\n'
        bad_name += '.json'
        bad_file = output_file.replace(".json", bad_name)
        f = open(bad_file, 'w')
        f.write(file_text)
        f.close()

        # Too high of a major API version
        bad_name = '_badjson_api_major_high'
        file_text  = '{\n'
        file_text += '    "file_format_version": "%s",\n' % cur_layer_json_version
        file_text += '    "api_layer": {\n'
        file_text += '        "name": "XR_APILAYER_LUNARG_%s%s",\n' % (layer_name, bad_name)
        file_text += '        "library_path": "%s",\n' % library_location
        file_text += '        "api_version": "15.0",\n'
        file_text += '        "implementation_version": "%s",\n' % implementation_version
        file_text += '        "description": "%s"\n' % description
        file_text += '    }\n'
        file_text += '}\n'
        bad_name += '.json'
        bad_file = output_file.replace(".json", bad_name)
        f = open(bad_file, 'w')
        f.write(file_text)
        f.close()

        # Valid JSON, but invalid Negotiate
        ####################################

        # Always fail negotiate
        bad_name = '_badnegotiate_always'
        file_text  = '{\n'
        file_text += '    "file_format_version": "%s",\n' % cur_layer_json_version
        file_text += '    "api_layer": {\n'
        file_text += '        "name": "XR_APILAYER_LUNARG_%s%s",\n' % (layer_name, bad_name)
        file_text += '        "library_path": "%s",\n' % library_location
        file_text += '        "api_version": "%s",\n' % api_version
        file_text += '        "implementation_version": "%s",\n' % implementation_version
        file_text += '        "description": "%s",\n' % description
        file_text += '        "functions": {\n'
        file_text += '           "xrNegotiateLoaderApiLayerInterface":\n'
        file_text += '               "TestLayerAlwaysFailNegotiateLoaderApiLayerInterface"\n'
        file_text += '       }\n'
        file_text += '    }\n'
        file_text += '}\n'
        bad_name += '.json'
        bad_file = output_file.replace(".json", bad_name)
        f = open(bad_file, 'w')
        f.write(file_text)
        f.close()

        # Pass negotiate, but return null GIPA
        bad_name = '_badnegotiate_invalid_gipa'
        file_text  = '{\n'
        file_text += '    "file_format_version": "%s",\n' % cur_layer_json_version
        file_text += '    "api_layer": {\n'
        file_text += '        "name": "XR_APILAYER_LUNARG_%s%s",\n' % (layer_name, bad_name)
        file_text += '        "library_path": "%s",\n' % library_location
        file_text += '        "api_version": "%s",\n' % api_version
        file_text += '        "implementation_version": "%s",\n' % implementation_version
        file_text += '        "description": "%s",\n' % description
        file_text += '        "functions": {\n'
        file_text += '           "xrNegotiateLoaderApiLayerInterface":\n'
        file_text += '               "TestLayerNullGipaNegotiateLoaderApiLayerInterface"\n'
        file_text += '       }\n'
        file_text += '    }\n'
        file_text += '}\n'
        bad_name += '.json'
        bad_file = output_file.replace(".json", bad_name)
        f = open(bad_file, 'w')
        f.write(file_text)
        f.close()

        # Pass negotiate, but return invalid interface version
        bad_name = '_badnegotiate_invalid_interface'
        file_text  = '{\n'
        file_text += '    "file_format_version": "%s",\n' % cur_layer_json_version
        file_text += '    "api_layer": {\n'
        file_text += '        "name": "XR_APILAYER_LUNARG_%s%s",\n' % (layer_name, bad_name)
        file_text += '        "library_path": "%s",\n' % library_location
        file_text += '        "api_version": "%s",\n' % api_version
        file_text += '        "implementation_version": "%s",\n' % implementation_version
        file_text += '        "description": "%s",\n' % description
        file_text += '        "functions": {\n'
        file_text += '           "xrNegotiateLoaderApiLayerInterface":\n'
        file_text += '               "TestLayerInvalidInterfaceNegotiateLoaderApiLayerInterface"\n'
        file_text += '       }\n'
        file_text += '    }\n'
        file_text += '}\n'
        bad_name += '.json'
        bad_file = output_file.replace(".json", bad_name)
        f = open(bad_file, 'w')
        f.write(file_text)
        f.close()

        # Pass negotiate, but return invalid api version
        bad_name = '_badnegotiate_invalid_api'
        file_text  = '{\n'
        file_text += '    "file_format_version": "%s",\n' % cur_layer_json_version
        file_text += '    "api_layer": {\n'
        file_text += '        "name": "XR_APILAYER_LUNARG_%s%s",\n' % (layer_name, bad_name)
        file_text += '        "library_path": "%s",\n' % library_location
        file_text += '        "api_version": "%s",\n' % api_version
        file_text += '        "implementation_version": "%s",\n' % implementation_version
        file_text += '        "description": "%s",\n' % description
        file_text += '        "functions": {\n'
        file_text += '           "xrNegotiateLoaderApiLayerInterface":\n'
        file_text += '               "TestLayerInvalidApiNegotiateLoaderApiLayerInterface"\n'
        file_text += '       }\n'
        file_text += '    }\n'
        file_text += '}\n'
        bad_name += '.json'
        bad_file = output_file.replace(".json", bad_name)
        f = open(bad_file, 'w')
        f.write(file_text)
        f.close()

        # Valid JSON, with relative path to library
        ####################################
        relative_lib = getRelativePath(output_file, library_location)

        # Provide a good relative path
        layer_suffix_name = '_good_relative_path'
        file_text  = '{\n'
        file_text += '    "file_format_version": "%s",\n' % cur_layer_json_version
        file_text += '    "api_layer": {\n'
        file_text += '        "name": "XR_APILAYER_LUNARG_%s%s",\n' % (layer_name, layer_suffix_name)
        file_text += '        "library_path": "%s",\n' % relative_lib
        file_text += '        "api_version": "%s",\n' % api_version
        file_text += '        "implementation_version": "%s",\n' % implementation_version
        file_text += '        "description": "%s"\n' % description
        file_text += '    }\n'
        file_text += '}\n'
        layer_suffix_name += '.json'
        bad_file = output_file.replace(".json", layer_suffix_name)
        f = open(bad_file, 'w')
        f.write(file_text)
        f.close()

        # Provide a bad relative path
        layer_suffix_name = '_badjson_relative_path'
        file_text  = '{\n'
        file_text += '    "file_format_version": "%s",\n' % cur_layer_json_version
        file_text += '    "api_layer": {\n'
        file_text += '        "name": "XR_APILAYER_LUNARG_%s%s",\n' % (layer_name, layer_suffix_name)
        file_text += '        "library_path": "%s",\n' % relative_lib.replace("test_layers","not_real")
        file_text += '        "api_version": "%s",\n' % api_version
        file_text += '        "implementation_version": "%s",\n' % implementation_version
        file_text += '        "description": "%s"\n' % description
        file_text += '    }\n'
        file_text += '}\n'
        layer_suffix_name += '.json'
        bad_file = output_file.replace(".json", layer_suffix_name)
        f = open(bad_file, 'w')
        f.write(file_text)
        f.close()

if __name__ == "__main__":
    main(sys.argv[1:])
