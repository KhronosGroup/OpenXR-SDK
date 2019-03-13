#!/usr/bin/python3
#
# Copyright (c) 2017-2019 The Khronos Group Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KINdD, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys, getopt

cur_runtime_json_version = '1.0.0'

def main(argv):
   output_file = ''
   library_location = ''
   generate_badjson_jsons = False

   usage =  '\ngenerate_runtime_manifest.py <ARGS>\n'
   usage += '    -f/--file <filename>\n'
   usage += '    -l/--lib <library_location>\n'
   usage += '    -b/--bad\n'

   try:
      opts, args = getopt.getopt(argv,"hbf:l:",["bad","file=","lib="])
   except getopt.GetoptError:
      print(usage)
      sys.exit(2)
   for opt, arg in opts:
      if opt == '-h':
         print(usage)
         sys.exit()
      elif opt in ("-f", "--file"):
         output_file = arg.strip()
      elif opt in ("-l", "--lib"):
         library_location = arg.strip()
      elif opt in ("-b", "--bad"):
         generate_badjson_jsons = True

   file_text  = '{\n'
   file_text += '    "file_format_version": "%s",\n' % cur_runtime_json_version
   file_text += '    "runtime": {\n'
   file_text += '        "library_path": "%s"\n' % library_location
   file_text += '    }\n'
   file_text += '}\n'
   f = open(output_file, 'w')
   f.write(file_text)
   f.close()

   if generate_badjson_jsons:
      # Bad File format versions
      ####################################

      # Missing
      bad_name = '_badjson_file_ver_missing.json'
      file_text  = '{\n'
      file_text += '    "runtime": {\n'
      file_text += '        "library_path": "%s",\n' % library_location
      file_text += '    }\n'
      file_text += '}\n'
      bad_file = output_file.replace(".json", bad_name)
      f = open(bad_file, 'w')
      f.write(file_text)
      f.close()

      # Use int
      bad_name = '_badjson_file_ver_int.json'
      file_text  = '{\n'
      file_text += '    "file_format_version": 1,\n'
      file_text += '    "runtime": {\n'
      file_text += '        "library_path": "%s",\n' % library_location
      file_text += '    }\n'
      file_text += '}\n'
      bad_file = output_file.replace(".json", bad_name)
      f = open(bad_file, 'w')
      f.write(file_text)
      f.close()

      # Use invalid string
      bad_name = '_badjson_file_ver_string.json'
      file_text  = '{\n'
      file_text += '    "file_format_version": "invalid string",\n'
      file_text += '    "runtime": {\n'
      file_text += '        "library_path": "%s",\n' % library_location
      file_text += '    }\n'
      file_text += '}\n'
      bad_file = output_file.replace(".json", bad_name)
      f = open(bad_file, 'w')
      f.write(file_text)
      f.close()

      # Too low of a version
      bad_name = '_badjson_file_ver_all_low.json'
      file_text  = '{\n'
      file_text += '    "file_format_version": "0.0.0",\n'
      file_text += '    "runtime": {\n'
      file_text += '        "library_path": "%s",\n' % library_location
      file_text += '    }\n'
      file_text += '}\n'
      bad_file = output_file.replace(".json", bad_name)
      f = open(bad_file, 'w')
      f.write(file_text)
      f.close()

      # Too high of a major version
      bad_name = '_badjson_file_ver_major_high.json'
      file_text  = '{\n'
      file_text += '    "file_format_version": "15.0.0",\n'
      file_text += '    "runtime": {\n'
      file_text += '        "library_path": "%s",\n' % library_location
      file_text += '    }\n'
      file_text += '}\n'
      bad_file = output_file.replace(".json", bad_name)
      f = open(bad_file, 'w')
      f.write(file_text)
      f.close()

      # Too high of a minor version
      bad_name = '_badjson_file_ver_minor_high.json'
      file_text  = '{\n'
      file_text += '    "file_format_version": "1.15.0",\n'
      file_text += '    "runtime": {\n'
      file_text += '        "library_path": "%s",\n' % library_location
      file_text += '    }\n'
      file_text += '}\n'
      bad_file = output_file.replace(".json", bad_name)
      f = open(bad_file, 'w')
      f.write(file_text)
      f.close()

      # No runtime section
      ####################################

      # Completely Missing
      bad_name = '_badjson_runtime_missing.json'
      file_text  = '{\n'
      file_text += '    "file_format_version": "%s",\n' % cur_runtime_json_version
      file_text += '    "library_path": "%s",\n' % library_location
      file_text += '}\n'
      bad_file = output_file.replace(".json", bad_name)
      f = open(bad_file, 'w')
      f.write(file_text)
      f.close()

      # Empty
      bad_name = '_badjson_runtime_empty.json'
      file_text  = '{\n'
      file_text += '    "file_format_version": "%s",\n' % cur_runtime_json_version
      file_text += '    "runtime": {\n'
      file_text += '    },\n'
      file_text += '    "library_path": "%s",\n' % library_location
      file_text += '}\n'
      bad_file = output_file.replace(".json", bad_name)
      f = open(bad_file, 'w')
      f.write(file_text)
      f.close()

      # Bad path
      ####################################

      # Missing
      bad_name = '_badjson_path_missing.json'
      file_text  = '{\n'
      file_text += '    "file_format_version": "%s",\n' % cur_runtime_json_version
      file_text += '    "runtime": {\n'
      file_text += '    }\n'
      file_text += '}\n'
      bad_file = output_file.replace(".json", bad_name)
      f = open(bad_file, 'w')
      f.write(file_text)
      f.close()

      # Use int
      bad_name = '_badjson_path_int.json'
      file_text  = '{\n'
      file_text += '    "file_format_version": "%s",\n' % cur_runtime_json_version
      file_text += '    "runtime": {\n'
      file_text += '        "library_path": 1,\n'
      file_text += '    }\n'
      file_text += '}\n'
      bad_file = output_file.replace(".json", bad_name)
      f = open(bad_file, 'w')
      f.write(file_text)
      f.close()

      # Replace valid path with invalid one
      bad_name = '_badjson_path_no_file.json'
      file_text  = '{\n'
      file_text += '    "file_format_version": "%s",\n' % cur_runtime_json_version
      file_text += '    "runtime": {\n'
      file_text += '        "library_path": "%s",\n' % library_location.replace("test_runtimes","not_real")
      file_text += '    }\n'
      file_text += '}\n'
      bad_file = output_file.replace(".json", bad_name)
      f = open(bad_file, 'w')
      f.write(file_text)
      f.close()

      # Valid JSON, but invalid Negotiate
      ####################################

      # Always fail negotiate
      bad_name = '_badnegotiate_always.json'
      file_text  = '{\n'
      file_text += '    "file_format_version": "%s",\n' % cur_runtime_json_version
      file_text += '    "runtime": {\n'
      file_text += '        "library_path": "%s",\n' % library_location
      file_text += '        "functions": {\n'
      file_text += '           "xrNegotiateLoaderRuntimeInterface":\n'
      file_text += '               "TestRuntimeAlwaysFailNegotiateLoaderRuntimeInterface"\n'
      file_text += '       }\n'
      file_text += '    }\n'
      file_text += '}\n'
      bad_file = output_file.replace(".json", bad_name)
      f = open(bad_file, 'w')
      f.write(file_text)
      f.close()

      # Pass negotiate, but return null GIPA
      bad_name = '_badnegotiate_invalid_gipa.json'
      file_text  = '{\n'
      file_text += '    "file_format_version": "%s",\n' % cur_runtime_json_version
      file_text += '    "runtime": {\n'
      file_text += '        "library_path": "%s",\n' % library_location
      file_text += '        "functions": {\n'
      file_text += '           "xrNegotiateLoaderRuntimeInterface":\n'
      file_text += '               "TestRuntimeNullGipaNegotiateLoaderRuntimeInterface"\n'
      file_text += '       }\n'
      file_text += '    }\n'
      file_text += '}\n'
      bad_file = output_file.replace(".json", bad_name)
      f = open(bad_file, 'w')
      f.write(file_text)
      f.close()

      # Pass negotiate, but return invalid interface version
      bad_name = '_badnegotiate_invalid_interface.json'
      file_text  = '{\n'
      file_text += '    "file_format_version": "%s",\n' % cur_runtime_json_version
      file_text += '    "runtime": {\n'
      file_text += '        "library_path": "%s",\n' % library_location
      file_text += '        "functions": {\n'
      file_text += '           "xrNegotiateLoaderRuntimeInterface":\n'
      file_text += '               "TestRuntimeInvalidInterfaceNegotiateLoaderRuntimeInterface"\n'
      file_text += '       }\n'
      file_text += '    }\n'
      file_text += '}\n'
      bad_file = output_file.replace(".json", bad_name)
      f = open(bad_file, 'w')
      f.write(file_text)
      f.close()

      # Pass negotiate, but return invalid api version
      bad_name = '_badnegotiate_invalid_api.json'
      file_text  = '{\n'
      file_text += '    "file_format_version": "%s",\n' % cur_runtime_json_version
      file_text += '    "runtime": {\n'
      file_text += '        "library_path": "%s",\n' % library_location
      file_text += '        "functions": {\n'
      file_text += '           "xrNegotiateLoaderRuntimeInterface":\n'
      file_text += '               "TestRuntimeInvalidApiNegotiateLoaderRuntimeInterface"\n'
      file_text += '       }\n'
      file_text += '    }\n'
      file_text += '}\n'
      bad_file = output_file.replace(".json", bad_name)
      f = open(bad_file, 'w')
      f.write(file_text)
      f.close()

if __name__ == "__main__":
   main(sys.argv[1:])
