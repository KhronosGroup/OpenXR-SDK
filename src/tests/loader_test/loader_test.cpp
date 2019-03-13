// Copyright (c) 2017-2019 The Khronos Group Inc.
// Copyright (c) 2017-2019 Valve Corporation
// Copyright (c) 2017-2019 LunarG, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: Mark Young <marky@lunarg.com>
//

#include <iostream>
#include <sstream>
#include <cstring>
#include <vector>

#include "filesystem_utils.hpp"
#include "loader_test_utils.hpp"

#include "xr_dependencies.h"
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

// Filter out the loader's messages to std::cerr if this is defined to 1.  This allows a
// clean output for the test.
#define FILTER_OUT_LOADER_ERRORS 1

enum LoaderTestGraphicsApiToUse { GRAPHICS_API_UNKONWN = 0, GRAPHICS_API_OPENGL, GRAPHICS_API_VULKAN, GRAPHICS_API_D3D };
LoaderTestGraphicsApiToUse g_graphics_api_to_use = GRAPHICS_API_UNKONWN;
bool g_debug_utils_exists = false;

void CleanupEnvironmentVariables() {
    LoaderTestUnsetEnvironmentVariable("XR_ENABLE_API_LAYERS");
    LoaderTestUnsetEnvironmentVariable("XR_API_LAYER_PATH");
    LoaderTestUnsetEnvironmentVariable("XR_RUNTIME_JSON");
}

// Test the xrEnumerateApiLayerProperties function through the loader.
void TestEnumLayers(uint32_t& total, uint32_t& passed, uint32_t& skipped, uint32_t& failed) {
    uint32_t local_total = 0;
    uint32_t local_passed = 0;
    uint32_t local_skipped = 0;
    uint32_t local_failed = 0;
#if FILTER_OUT_LOADER_ERRORS == 1
    std::streambuf* original_cerr = nullptr;
#endif
    try {
        XrResult test_result = XR_SUCCESS;
        uint32_t num_before_explicit = 0;
        const uint32_t num_expected_valid_jsons = 6;
        std::vector<XrApiLayerProperties> properties;

#if FILTER_OUT_LOADER_ERRORS == 1
        // Re-direct std::cerr to a string since we're intentionally causing errors and we don't
        // want it polluting the output stream.
        std::stringstream buffer;
        original_cerr = std::cerr.rdbuf(buffer.rdbuf());
#endif

        std::cout << "    Starting TestEnumLayers" << std::endl;
        ;

        for (uint32_t test = 0; test < 2; ++test) {
            uint32_t in_layer_value = 0;
            uint32_t out_layer_value = 0;
            std::string subtest_name;

            switch (test) {
                // No Explicit layers set
                case 0:
                    subtest_name = "No explicit layers";
                    // NOTE: Implicit layers will still be present, need to figure out what to do here.
                    LoaderTestUnsetEnvironmentVariable("XR_ENABLE_API_LAYERS");
                    LoaderTestUnsetEnvironmentVariable("XR_API_LAYER_PATH");
                    break;
                default:
                    subtest_name = "Simple explicit layers";
                    LoaderTestSetEnvironmentVariable("XR_API_LAYER_PATH", "resources/layers");
                    LoaderTestSetEnvironmentVariable("XR_ENABLE_API_LAYERS", "XR_APILAYER_LUNARG_test");
                    break;
            }

            // Test number query
            local_total++;
            std::cout << "        " << subtest_name << " layer count check: ";
            test_result = xrEnumerateApiLayerProperties(in_layer_value, &out_layer_value, nullptr);
            if (XR_SUCCESS != test_result) {
                std::cout << "Failed with return " << std::to_string(test_result) << std::endl;
                local_failed++;
            } else {
                if (test > 0 && out_layer_value != num_before_explicit + num_expected_valid_jsons) {
                    std::cout << "Failed, expected count " << std::to_string(num_before_explicit + num_expected_valid_jsons)
                              << ", got " << std::to_string(out_layer_value) << std::endl;
                    local_failed++;
                } else {
                    local_passed++;
                    std::cout << "Passed" << std::endl;
                }
            }

            // Try property return
            in_layer_value = out_layer_value + 1;
            out_layer_value = 0;
            properties.resize(in_layer_value);
            local_total++;
            std::cout << "        " << subtest_name << " layer props query: ";
            test_result = xrEnumerateApiLayerProperties(in_layer_value, &out_layer_value, properties.data());
            if (XR_SUCCESS != test_result) {
                std::cout << "Failed with return " << std::to_string(test_result) << std::endl;
                local_failed++;
            } else {
                bool found_bad = false;
                bool found_good_absolute_test = false;
                bool found_good_relative_test = false;
                uint16_t expected_major = XR_VERSION_MAJOR(XR_CURRENT_API_VERSION);
                uint16_t expected_minor = XR_VERSION_MINOR(XR_CURRENT_API_VERSION);
                for (uint32_t iii = 0; iii < properties.size(); ++iii) {
                    std::string layer_name = properties[iii].layerName;
                    if ("XR_APILAYER_LUNARG_test" == layer_name && 1 == properties[iii].implementationVersion &&
                        XR_MAKE_VERSION(expected_major, expected_minor, 0U) == properties[iii].specVersion &&
                        0 == strcmp(properties[iii].description, "Test_description")) {
                        found_good_absolute_test = true;
                    } else if ("XR_APILAYER_LUNARG_test_good_relative_path" == layer_name &&
                               1 == properties[iii].implementationVersion &&
                               XR_MAKE_VERSION(expected_major, expected_minor, 0U) == properties[iii].specVersion &&
                               0 == strcmp(properties[iii].description, "Test_description")) {
                        found_good_relative_test = true;
                    } else if (std::string::npos != layer_name.find("_badjson")) {
                        std::cout << "Failed, found bad layer " << layer_name << " in list" << std::endl;
                        found_bad = true;
                        local_failed++;
                        break;
                    }
                }

                if (test > 0 && (!found_good_absolute_test || !found_good_relative_test)) {
                    std::cout << "Failed, did not find ";
                    if (!found_good_absolute_test) {
                        std::cout << "XR_APILAYER_LUNARG_test";
                    }
                    if (!found_good_relative_test) {
                        if (!found_good_absolute_test) {
                            std::cout << " or ";
                        }
                        std::cout << "XR_APILAYER_LUNARG_test_good_relative_path";
                    }
                    std::cout << std::endl;
                    local_failed++;
                } else if (!found_bad) {
                    std::cout << "Passed" << std::endl;
                    local_passed++;
                }
            }

            if (test == 0) {
                num_before_explicit = out_layer_value;
            }
        }
    } catch (...) {
        std::cout << "Exception triggered during test, automatic failure" << std::endl;
        local_failed++;
        local_total++;
    }

#if FILTER_OUT_LOADER_ERRORS == 1
    // Restore std::cerr to the original buffer
    std::cerr.rdbuf(original_cerr);
#endif

    // Cleanup
    CleanupEnvironmentVariables();

    // Output results for this test
    std::cout << "    Finished TestEnumLayers : ";
    if (local_failed > 0) {
        std::cout << "Failed (Local - Passed: " << std::to_string(local_passed) << ", Failed: " << std::to_string(local_failed)
                  << ", Skipped: " << std::to_string(local_skipped) << ")" << std::endl
                  << std::endl;
    } else {
        std::cout << "Passed (Local - Passed: " << std::to_string(local_passed) << ", Failed: " << std::to_string(local_failed)
                  << ", Skipped: " << std::to_string(local_skipped) << ")" << std::endl
                  << std::endl;
    }
    total += local_total;
    passed += local_passed;
    failed += local_failed;
    skipped += local_skipped;
}

// Test the xrEnumerateInstanceExtensionProperties function through the loader.
void TestEnumInstanceExtensions(uint32_t& total, uint32_t& passed, uint32_t& skipped, uint32_t& failed) {
    uint32_t local_total = 0;
    uint32_t local_passed = 0;
    uint32_t local_skipped = 0;
    uint32_t local_failed = 0;
#if FILTER_OUT_LOADER_ERRORS == 1
    std::streambuf* original_cerr = nullptr;
#endif

    try {
        XrResult test_result = XR_SUCCESS;
        uint32_t in_extension_value;
        uint32_t out_extension_value;
        std::vector<XrExtensionProperties> properties;

        std::cout << "    Starting TestEnumInstanceExtensions" << std::endl;

#if FILTER_OUT_LOADER_ERRORS == 1
        // Re-direct std::cerr to a string since we're intentionally causing errors and we don't
        // want it polluting the output stream.
        std::stringstream buffer;
        original_cerr = std::cerr.rdbuf(buffer.rdbuf());
#endif

        for (uint32_t test = 0; test < 2; ++test) {
            std::string subtest_name;
            std::string valid_runtime_path;
            uint32_t expected_extension_count;

            switch (test) {
                // No Explicit layers set
                case 0:
                    subtest_name = "with no explicit API layers";
                    // NOTE: Implicit layers will still be present, need to figure out what to do here.
                    LoaderTestUnsetEnvironmentVariable("XR_ENABLE_API_LAYERS");
                    LoaderTestUnsetEnvironmentVariable("XR_API_LAYER_PATH");
                    expected_extension_count = 3;
                    break;
                default:
                    subtest_name = "with explicit API layers";
                    LoaderTestSetEnvironmentVariable("XR_API_LAYER_PATH", "resources/layers");
                    LoaderTestSetEnvironmentVariable("XR_ENABLE_API_LAYERS", "XR_APILAYER_LUNARG_test");
                    expected_extension_count = 4;
                    break;
            }

            // Test just runtimes
            std::vector<std::string> files;
            std::string relative_path = "resources";
            relative_path += TEST_DIRECTORY_SYMBOL;
            relative_path += "runtimes";
            if (FileSysUtilsFindFilesInPath(relative_path, files)) {
                for (std::string& cur_file : files) {
                    std::string relative_file_name;
                    std::string full_name;
                    FileSysUtilsGetCurrentPath(full_name);
                    FileSysUtilsCombinePaths("resources/runtimes", cur_file, relative_file_name);
                    full_name += TEST_DIRECTORY_SYMBOL;
                    full_name += relative_file_name;
                    if (std::string::npos != cur_file.find("_badjson_") || std::string::npos != cur_file.find("_badnegotiate_")) {
                        // This is the "bad" runtime path.
                        FileSysUtilsGetCurrentPath(full_name);
                        LoaderTestSetEnvironmentVariable("XR_RUNTIME_JSON", full_name);

                        in_extension_value = 0;
                        out_extension_value = 0;
                        local_total++;
                        std::cout << "        JSON " << cur_file << " extension enum count query (" << subtest_name << "): ";
                        test_result =
                            xrEnumerateInstanceExtensionProperties(nullptr, in_extension_value, &out_extension_value, nullptr);
                        if (XR_SUCCESS == test_result) {
                            std::cout << "Failed" << std::endl;
                            local_failed++;
                        } else {
                            std::cout << "Passed" << std::endl;
                            local_passed++;
                        }
                    } else if (std::string::npos != cur_file.find("test_runtime.json")) {
                        // This is a "good" runtime, so it should pass with some set of results.
                        LoaderTestSetEnvironmentVariable("XR_RUNTIME_JSON", full_name);
                        // Save the valid runtime path for later
                        valid_runtime_path = full_name;

                        // Query the count (should return 2)
                        in_extension_value = 0;
                        out_extension_value = 0;
                        local_total++;
                        std::cout << "        JSON " << cur_file << " extension enum count query (" << subtest_name << "): ";
                        test_result =
                            xrEnumerateInstanceExtensionProperties(nullptr, in_extension_value, &out_extension_value, nullptr);
                        if (XR_SUCCESS != test_result || out_extension_value != expected_extension_count) {
                            std::cout << "Failed" << std::endl;
                            local_failed++;
                        } else {
                            std::cout << "Passed" << std::endl;
                            local_passed++;
                        }

                        // Get the properties
                        properties.resize(out_extension_value);
                        for (uint32_t prop = 0; prop < out_extension_value; ++prop) {
                            properties[prop] = {};
                            properties[prop].type = XR_TYPE_EXTENSION_PROPERTIES;
                            properties[prop].next = nullptr;
                        }
                        in_extension_value = out_extension_value;
                        out_extension_value = 0;
                        local_total++;
                        std::cout << "        JSON " << cur_file << " extension enum properties query (" << subtest_name << "): ";
                        test_result = xrEnumerateInstanceExtensionProperties(nullptr, in_extension_value, &out_extension_value,
                                                                             properties.data());
                        if (XR_SUCCESS != test_result || out_extension_value != expected_extension_count) {
                            std::cout << "Failed" << std::endl;
                            local_failed++;
                        } else {
                            bool found_fake1 = false;
                            bool found_fake2 = false;
                            bool found_fake3 = false;
                            for (uint32_t prop = 0; prop < properties.size(); ++prop) {
                                std::string extension_name = properties[prop].extensionName;
                                if ("XR_KHR_fake_ext1" && properties[prop].specVersion == 57) {
                                    found_fake1 = true;
                                    // NOTE: The layer, if it is present, has a newer version than this.  But,
                                    //       the spec states that the layer should always lose to the runtime.
                                    //       Therefore, the runtime version should always be returned, even with
                                    //       the layer forced on.
                                } else if ("XR_KHR_fake_ext2" && properties[prop].specVersion == 3) {
                                    found_fake2 = true;
                                } else if ("XR_KHR_fake_ext3" && properties[prop].specVersion == 42) {
                                    found_fake3 = true;
                                }
                            }
                            if (found_fake1 && found_fake2 && (test == 0 || (test == 1 && found_fake3))) {
                                std::cout << "Passed" << std::endl;
                                local_passed++;
                            } else {
                                std::cout << "Failed, missing expected extension" << std::endl;
                                local_failed++;
                            }
                        }
                    }
                }
            }

            // With a valid runtime, we need
            if (0 == valid_runtime_path.size()) {
                std::cout << "Failed, finding valid runtime" << std::endl;
                local_total++;
                local_failed++;
                continue;
            }
            LoaderTestSetEnvironmentVariable("XR_RUNTIME_JSON", valid_runtime_path);

            // Try with a garbage layer name
            in_extension_value = 0;
            out_extension_value = 0;
            local_total++;
            std::cout << "        Garbage Layer extension enum count query (" << subtest_name << "): ";
            test_result = xrEnumerateInstanceExtensionProperties("XR_APILAYER_LUNARG_no_such_layer", in_extension_value,
                                                                 &out_extension_value, nullptr);
            if (XR_ERROR_API_LAYER_NOT_PRESENT != test_result) {
                std::cout << "Failed" << std::endl;
                local_failed++;
            } else {
                std::cout << "Passed" << std::endl;
                local_passed++;
            }

            // Try with a valid layer name (if layer is present)
            in_extension_value = 0;
            out_extension_value = 0;
            local_total++;
            std::cout << "        Valid Layer extension enum count query (" << subtest_name << "): ";
            test_result = xrEnumerateInstanceExtensionProperties("XR_APILAYER_LUNARG_test", in_extension_value,
                                                                 &out_extension_value, nullptr);
            if ((test == 0 && XR_ERROR_API_LAYER_NOT_PRESENT != test_result) || (test == 1 && XR_SUCCESS != test_result)) {
                std::cout << "Failed" << std::endl;
                local_failed++;
            } else {
                std::cout << "Passed" << std::endl;
                local_passed++;
            }
        }
    } catch (...) {
        std::cout << "Exception triggered during test, automatic failure" << std::endl;
        local_failed++;
        local_total++;
    }

#if FILTER_OUT_LOADER_ERRORS == 1
    // Restore std::cerr to the original buffer
    std::cerr.rdbuf(original_cerr);
#endif

    // Cleanup
    CleanupEnvironmentVariables();

    // Output results for this test
    std::cout << "    Finished TestEnumInstanceExtensions : ";
    if (local_failed > 0) {
        std::cout << "Failed (Local - Passed: " << std::to_string(local_passed) << ", Failed: " << std::to_string(local_failed)
                  << ", Skipped: " << std::to_string(local_skipped) << ")" << std::endl
                  << std::endl;
    } else {
        std::cout << "Passed (Local - Passed: " << std::to_string(local_passed) << ", Failed: " << std::to_string(local_failed)
                  << ", Skipped: " << std::to_string(local_skipped) << ")" << std::endl
                  << std::endl;
    }
    total += local_total;
    passed += local_passed;
    failed += local_failed;
    skipped += local_skipped;
}

#define DEFINE_TEST(test_name) void test_name(uint32_t& total, uint32_t& passed, uint32_t& skipped, uint32_t& failed)

#define INIT_TEST(test_name)    \
    uint32_t local_total = 0;   \
    uint32_t local_passed = 0;  \
    uint32_t local_skipped = 0; \
    uint32_t local_failed = 0;  \
    std::cout << "    Starting " << #test_name << std::endl;

#define TEST_REPORT(test_name)                                                                                                  \
    std::cout << "    Finished " << #test_name << ": ";                                                                         \
    if (local_failed > 0) {                                                                                                     \
        std::cout << "Failed (Local - Passed: " << std::to_string(local_passed) << ", Failed: " << std::to_string(local_failed) \
                  << ", Skipped: " << std::to_string(local_skipped) << ")" << std::endl                                         \
                  << std::endl;                                                                                                 \
    } else {                                                                                                                    \
        std::cout << "Passed (Local - Passed: " << std::to_string(local_passed) << ", Failed: " << std::to_string(local_failed) \
                  << ", Skipped: " << std::to_string(local_skipped) << ")" << std::endl                                         \
                  << std::endl;                                                                                                 \
    }                                                                                                                           \
    total += local_total;                                                                                                       \
    passed += local_passed;                                                                                                     \
    failed += local_failed;                                                                                                     \
    skipped += local_skipped;

#define TEST_EQUAL(test, expected, cout_string)                            \
    local_total++;                                                         \
    if (expected != (test)) {                                              \
        std::cout << "        " << cout_string << ": Failed" << std::endl; \
        local_failed++;                                                    \
    } else {                                                               \
        std::cout << "        " << cout_string << ": Passed" << std::endl; \
        local_passed++;                                                    \
    }

#define TEST_NOT_EQUAL(test, expected, cout_string)                        \
    local_total++;                                                         \
    if (expected == (test)) {                                              \
        std::cout << "        " << cout_string << ": Failed" << std::endl; \
        local_failed++;                                                    \
    } else {                                                               \
        std::cout << "        " << cout_string << ": Passed" << std::endl; \
        local_passed++;                                                    \
    }

#define TEST_FAIL(cout_string) \
    local_total++;             \
    local_failed++;            \
    std::cout << "        " << cout_string << ": Failed" << std::endl;

// Test creating and destroying an OpenXR instance through the loader.
DEFINE_TEST(TestCreateDestroyInstance) {
    INIT_TEST(TestCreateDestroyInstance)

    try {
        XrResult expected_result = XR_SUCCESS;
        char valid_layer_to_enable[] = "XR_APILAYER_LUNARG_api_dump";
        char invalid_layer_to_enable[] = "XR_APILAYER_LUNARG_invalid_layer_test";
        char invalid_extension_to_enable[] = "XR_KHR_fake_ext1";
        const char* const valid_layer_name_array[1] = {valid_layer_to_enable};
        const char* const invalid_layer_name_array[1] = {invalid_layer_to_enable};
        const char* valid_extension_name_array[1];
        const char* const invalid_extension_name_array[1] = {invalid_extension_to_enable};
        XrApplicationInfo application_info = {};
        strcpy(application_info.applicationName, "Loader Test");
        application_info.applicationVersion = 688;
        strcpy(application_info.engineName, "Infinite Improbability Drive");
        application_info.engineVersion = 42;
        application_info.apiVersion = XR_CURRENT_API_VERSION;

        std::string current_path;
        std::string sample_impl_runtime_path;
        std::string layer_path;
        if (!FileSysUtilsGetCurrentPath(current_path) ||
            !FileSysUtilsCombinePaths(current_path, "../../impl/openxr_sample_impl.json", sample_impl_runtime_path)) {
            std::cout << "FAILED to set runtime path!" << std::endl;
            throw - 1;
        }
        if (!FileSysUtilsCombinePaths(current_path, "../../api_layers", layer_path)) {
            std::cout << "FAILED to set layer path!" << std::endl;
            throw - 1;
        }
        LoaderTestSetEnvironmentVariable("XR_RUNTIME_JSON", sample_impl_runtime_path);
        LoaderTestSetEnvironmentVariable("XR_API_LAYER_PATH", layer_path);

        // Look to see if the runtime supports any extensions, if it does, use the first one in this test.
        bool missing_extensions = true;
        std::vector<XrExtensionProperties> properties;
        uint32_t in_extension_value = 0;
        uint32_t out_extension_value = 0;
        if (XR_SUCCESS == xrEnumerateInstanceExtensionProperties(nullptr, in_extension_value, &out_extension_value, nullptr) &&
            0 < out_extension_value) {
            in_extension_value = out_extension_value;
            properties.resize(out_extension_value);
            for (uint32_t prop = 0; prop < out_extension_value; ++prop) {
                properties[prop].type = XR_TYPE_EXTENSION_PROPERTIES;
                properties[prop].next = nullptr;
            }
            if (XR_SUCCESS ==
                xrEnumerateInstanceExtensionProperties(nullptr, in_extension_value, &out_extension_value, properties.data())) {
                valid_extension_name_array[0] = properties[0].extensionName;
                missing_extensions = false;
                for (uint32_t ext = 0; ext < properties.size(); ++ext) {
                    if (!strcmp(properties[ext].extensionName, XR_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
                        g_debug_utils_exists = true;
                    }
#ifdef XR_USE_GRAPHICS_API_OPENGL
                    if (!strcmp(properties[ext].extensionName, XR_KHR_OPENGL_ENABLE_EXTENSION_NAME) &&
                        g_graphics_api_to_use == GRAPHICS_API_UNKONWN) {
                        g_graphics_api_to_use = GRAPHICS_API_OPENGL;
                    }
#endif  // XR_USE_GRAPHICS_API_OPENGL
#ifdef XR_USE_GRAPHICS_API_VULKAN
                    if (!strcmp(properties[ext].extensionName, XR_KHR_VULKAN_ENABLE_EXTENSION_NAME)) {
                        g_graphics_api_to_use = GRAPHICS_API_VULKAN;
                    }
#endif  // XR_USE_GRAPHICS_API_VULKAN
#ifdef XR_USE_GRAPHICS_API_D3D11
                    if (!strcmp(properties[ext].extensionName, XR_KHR_D3D11_ENABLE_EXTENSION_NAME) &&
                        g_graphics_api_to_use == GRAPHICS_API_UNKONWN) {
                        g_graphics_api_to_use = GRAPHICS_API_D3D;
                    }
#endif  // XR_USE_GRAPHICS_API_D3D11
                }
            }
        }

        for (uint32_t test_num = 0; test_num < 6; ++test_num) {
            XrInstance instance = XR_NULL_HANDLE;
            std::string current_test_string;
            XrInstanceCreateInfo instance_create_info = {};
            instance_create_info.type = XR_TYPE_INSTANCE_CREATE_INFO;
            instance_create_info.applicationInfo = application_info;

            switch (test_num) {
                // Test 0 - Basic, plain-vanilla xrCreateInstance
                case 0:
                    current_test_string = "Simple xrCreateInstance";
                    expected_result = XR_SUCCESS;
                    break;
                // Test 1 - xrCreateInstance with a valid layer
                case 1:
                    current_test_string = "xrCreateInstance with a single valid layer";
                    expected_result = XR_SUCCESS;
                    instance_create_info.enabledApiLayerCount = 1;
                    instance_create_info.enabledApiLayerNames = valid_layer_name_array;
                    break;
                // Test 2 - xrCreateInstance with an invalid layer
                case 2:
                    current_test_string = "xrCreateInstance with a single invalid layer";
                    expected_result = XR_ERROR_API_LAYER_NOT_PRESENT;
                    instance_create_info.enabledApiLayerCount = 1;
                    instance_create_info.enabledApiLayerNames = invalid_layer_name_array;
                    break;
                // Test 3 - xrCreateInstance with a valid extension
                case 3:
                    if (missing_extensions) {
                        continue;
                    }
                    current_test_string = "xrCreateInstance with a single valid extension";
                    expected_result = XR_SUCCESS;
                    instance_create_info.enabledExtensionCount = 1;
                    instance_create_info.enabledExtensionNames = valid_extension_name_array;
                    break;
                // Test 4 - xrCreateInstance with an invalid extension
                case 4:
                    if (missing_extensions) {
                        continue;
                    }
                    current_test_string = "xrCreateInstance with a single invalid extension";
                    expected_result = XR_ERROR_EXTENSION_NOT_PRESENT;
                    instance_create_info.enabledExtensionCount = 1;
                    instance_create_info.enabledExtensionNames = invalid_extension_name_array;
                    break;
                // Test 5 - xrCreateInstance with everything
                case 5:
                    if (missing_extensions) {
                        continue;
                    }
                    current_test_string = "xrCreateInstance with app info, valid layer, and a valid extension";
                    expected_result = XR_SUCCESS;
                    instance_create_info.enabledApiLayerCount = 1;
                    instance_create_info.enabledApiLayerNames = valid_layer_name_array;
                    instance_create_info.enabledExtensionCount = 1;
                    instance_create_info.enabledExtensionNames = valid_extension_name_array;
                    break;
            }

            // Try create instance and look for the correct return
            std::string cur_message = current_test_string;
            cur_message += " - xrCreateInstance";
            TEST_EQUAL(xrCreateInstance(&instance_create_info, &instance), expected_result, cur_message)

            // If the expected return was a success, clean up the created instance
            cur_message = current_test_string;
            cur_message += " - xrDestroyInstance";
            TEST_EQUAL(xrDestroyInstance(instance), XR_SUCCESS, cur_message)
        }

        // Make sure delete with a NULL handle works
        TEST_EQUAL(xrDestroyInstance(XR_NULL_HANDLE), XR_SUCCESS, "xrDestroyInstance(XR_NULL_HANDLE)")
    } catch (...) {
        TEST_FAIL("Exception triggered during test, automatic failure")
    }

    // Cleanup
    CleanupEnvironmentVariables();

    // Output results for this test
    TEST_REPORT(TestCreateDestroyInstance)
}

// Test at least one XrInstance function not directly implemented in the loader's manual code section.
// This is to make sure that the automatic instance functions work.
DEFINE_TEST(TestGetSystem) {
    INIT_TEST(TestGetSystem)

    try {
        std::string current_path;
        std::string sample_impl_runtime_path;
        std::string layer_path;
        if (!FileSysUtilsGetCurrentPath(current_path) ||
            !FileSysUtilsCombinePaths(current_path, "../../impl/openxr_sample_impl.json", sample_impl_runtime_path)) {
            std::cout << "FAILED to set runtime path!" << std::endl;
            throw - 1;
        }
        if (!FileSysUtilsCombinePaths(current_path, "../../api_layers", layer_path)) {
            std::cout << "FAILED to set layer path!" << std::endl;
            throw - 1;
        }
        LoaderTestSetEnvironmentVariable("XR_RUNTIME_JSON", sample_impl_runtime_path);
        LoaderTestSetEnvironmentVariable("XR_API_LAYER_PATH", layer_path);

        XrInstance instance = XR_NULL_HANDLE;
        XrApplicationInfo application_info = {};
        strcpy(application_info.applicationName, "Loader Test");
        application_info.applicationVersion = 688;
        strcpy(application_info.engineName, "Infinite Improbability Drive");
        application_info.engineVersion = 42;
        application_info.apiVersion = XR_CURRENT_API_VERSION;
        XrInstanceCreateInfo instance_create_info = {};
        instance_create_info.type = XR_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.applicationInfo = application_info;

        TEST_EQUAL(xrCreateInstance(&instance_create_info, &instance), XR_SUCCESS, "Creating instance")

        XrSystemGetInfo system_get_info = {};
        system_get_info.type = XR_TYPE_SYSTEM_GET_INFO;
        system_get_info.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

        XrSystemId systemId;
        TEST_EQUAL(xrGetSystem(instance, &system_get_info, &systemId), XR_SUCCESS, "xrGetSystem");

        TEST_EQUAL(xrDestroyInstance(instance), XR_SUCCESS, "Destroying instance")
    } catch (...) {
        TEST_FAIL("Exception triggered during test, automatic failure")
    }

    // Cleanup
    CleanupEnvironmentVariables();

    // Output results for this test
    TEST_REPORT(TestGetSystem)
}

// Test at least one non-XrInstance function to make sure that the automatic non-instance functions work.
DEFINE_TEST(TestCreateDestroySession) {
    INIT_TEST(TestCreateDestroySession)

    try {
        std::string current_path;
        std::string sample_impl_runtime_path;
        std::string layer_path;

        if (!FileSysUtilsGetCurrentPath(current_path) ||
            !FileSysUtilsCombinePaths(current_path, "../../impl/openxr_sample_impl.json", sample_impl_runtime_path)) {
            std::cout << "FAILED to set runtime path!" << std::endl;
            throw - 1;
        }
        if (!FileSysUtilsCombinePaths(current_path, "../../api_layers", layer_path)) {
            std::cout << "FAILED to set layer path!" << std::endl;
            throw - 1;
        }
        LoaderTestSetEnvironmentVariable("XR_RUNTIME_JSON", sample_impl_runtime_path);
        LoaderTestSetEnvironmentVariable("XR_API_LAYER_PATH", layer_path);

        XrInstance instance = XR_NULL_HANDLE;
        XrApplicationInfo application_info = {};
        strcpy(application_info.applicationName, "Loader Test");
        application_info.applicationVersion = 688;
        strcpy(application_info.engineName, "Infinite Improbability Drive");
        application_info.engineVersion = 42;
        application_info.apiVersion = XR_CURRENT_API_VERSION;
        XrInstanceCreateInfo instance_create_info = {};
        instance_create_info.type = XR_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.applicationInfo = application_info;
        std::string enabled_extension_name;
        const char* enabled_extension_array[2];

#ifdef XR_USE_GRAPHICS_API_VULKAN
        if (g_graphics_api_to_use == GRAPHICS_API_VULKAN) {
            instance_create_info.enabledExtensionCount = 1;
            enabled_extension_name = XR_KHR_VULKAN_ENABLE_EXTENSION_NAME;
            enabled_extension_array[0] = enabled_extension_name.c_str();
            instance_create_info.enabledExtensionNames = enabled_extension_array;
        }
#endif  // XR_USE_GRAPHICS_API_VULKAN
#ifdef XR_USE_GRAPHICS_API_OPENGL
        if (g_graphics_api_to_use == GRAPHICS_API_OPENGL) {
            instance_create_info.enabledExtensionCount = 1;
            enabled_extension_name = XR_KHR_OPENGL_ENABLE_EXTENSION_NAME;
            enabled_extension_array[0] = enabled_extension_name.c_str();
            instance_create_info.enabledExtensionNames = enabled_extension_array;
        }
#endif  // XR_USE_GRAPHICS_API_OPENGL
#ifdef XR_USE_GRAPHICS_API_D3D11
        if (g_graphics_api_to_use == GRAPHICS_API_D3D) {
            instance_create_info.enabledExtensionCount = 1;
            enabled_extension_name = XR_KHR_D3D11_ENABLE_EXTENSION_NAME;
            enabled_extension_array[0] = enabled_extension_name.c_str();
            instance_create_info.enabledExtensionNames = enabled_extension_array;
        }
#endif  // XR_USE_GRAPHICS_API_D3D11

        TEST_EQUAL(xrCreateInstance(&instance_create_info, &instance), XR_SUCCESS, "Creating instance")

        XrSystemGetInfo system_get_info;
        memset(&system_get_info, 0, sizeof(system_get_info));
        system_get_info.type = XR_TYPE_SYSTEM_GET_INFO;
        system_get_info.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

        XrSystemId systemId;
        TEST_EQUAL(xrGetSystem(instance, &system_get_info, &systemId), XR_SUCCESS, "xrGetSystem");

        if (systemId) {
            void* graphics_binding = nullptr;

#ifdef XR_USE_GRAPHICS_API_VULKAN
            XrGraphicsBindingVulkanKHR vulkan_graphics_binding = {};
            if (g_graphics_api_to_use == GRAPHICS_API_VULKAN) {
                PFN_xrGetVulkanInstanceExtensionsKHR pfn_get_vulkan_instance_extensions_khr;
                PFN_xrGetVulkanDeviceExtensionsKHR pfn_get_vulkan_device_extensions_khr;
                PFN_xrGetVulkanGraphicsDeviceKHR pfn_get_vulkan_graphics_device_khr;
                PFN_xrGetVulkanGraphicsRequirementsKHR pfn_get_vulkan_graphics_requirements_khr;
                TEST_EQUAL(xrGetInstanceProcAddr(instance, "xrGetVulkanInstanceExtensionsKHR",
                                                 reinterpret_cast<PFN_xrVoidFunction*>(&pfn_get_vulkan_instance_extensions_khr)),
                           XR_SUCCESS, "TestCreateDestroySession get xrGetVulkanInstanceExtensionsKHR function pointer");
                TEST_NOT_EQUAL(pfn_get_vulkan_instance_extensions_khr, nullptr,
                               "TestCreateDestroySession invalid xrGetVulkanInstanceExtensionsKHR function pointer");
                TEST_EQUAL(xrGetInstanceProcAddr(instance, "xrGetVulkanDeviceExtensionsKHR",
                                                 reinterpret_cast<PFN_xrVoidFunction*>(&pfn_get_vulkan_device_extensions_khr)),
                           XR_SUCCESS, "TestCreateDestroySession get xrGetVulkanDeviceExtensionsKHR function pointer");
                TEST_NOT_EQUAL(pfn_get_vulkan_device_extensions_khr, nullptr,
                               "TestCreateDestroySession invalid xrGetVulkanDeviceExtensionsKHR function pointer");
                TEST_EQUAL(xrGetInstanceProcAddr(instance, "xrGetVulkanGraphicsDeviceKHR",
                                                 reinterpret_cast<PFN_xrVoidFunction*>(&pfn_get_vulkan_graphics_device_khr)),
                           XR_SUCCESS, "TestCreateDestroySession get xrGetVulkanGraphicsDeviceKHR function pointer");
                TEST_NOT_EQUAL(pfn_get_vulkan_graphics_device_khr, nullptr,
                               "TestCreateDestroySession invalid xrGetVulkanGraphicsDeviceKHR function pointer");
                TEST_EQUAL(xrGetInstanceProcAddr(instance, "xrGetVulkanGraphicsRequirementsKHR",
                                                 reinterpret_cast<PFN_xrVoidFunction*>(&pfn_get_vulkan_graphics_requirements_khr)),
                           XR_SUCCESS, "TestCreateDestroySession get xrGetVulkanGraphicsRequirementsKHR function pointer");
                TEST_NOT_EQUAL(pfn_get_vulkan_graphics_requirements_khr, nullptr,
                               "TestCreateDestroySession invalid xrGetVulkanGraphicsRequirementsKHR function pointer");

                XrGraphicsRequirementsVulkanKHR vulkan_graphics_requirements = {};
                vulkan_graphics_requirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR;
                vulkan_graphics_requirements.next = nullptr;
                TEST_EQUAL(pfn_get_vulkan_graphics_requirements_khr(instance, systemId, &vulkan_graphics_requirements), XR_SUCCESS,
                           "TestCreateDestroySession calling xrGetVulkanGraphicsRequirementsKHR");

                // TODO: Just need something other than NULL here for now (for validation).  Eventually need
                //       to correctly put in a valid pointer to an Display
                vulkan_graphics_binding.type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR;
                vulkan_graphics_binding.instance = VK_NULL_HANDLE;
                vulkan_graphics_binding.physicalDevice = VK_NULL_HANDLE;
                vulkan_graphics_binding.device = VK_NULL_HANDLE;
                vulkan_graphics_binding.queueFamilyIndex = 0;
                vulkan_graphics_binding.queueIndex = 0;
                graphics_binding = reinterpret_cast<void*>(&vulkan_graphics_binding);
            }
#endif  // XR_USE_GRAPHICS_API_VULKAN
#ifdef XR_USE_GRAPHICS_API_OPENGL
#ifdef _WIN32
            XrGraphicsBindingOpenGLWin32KHR win32_gl_graphics_binding = {};
#elif defined(OS_LINUX_XLIB)
            XrGraphicsBindingOpenGLXlibKHR glx_gl_graphics_binding = {};
#elif defined(OS_LINUX_XCB) || defined(OS_LINUX_XCB_GLX)
            XrGraphicsBindingOpenGLXcbKHR xcb_gl_graphics_binding = {};
#elif defined(OS_LINUX_WAYLAND)
            XrGraphicsBindingOpenGLWaylandKHR wayland_gl_graphics_binding = {};
#endif
            if (g_graphics_api_to_use == GRAPHICS_API_OPENGL) {
                PFN_xrGetOpenGLGraphicsRequirementsKHR pfn_get_opengl_graphics_requirements_khr;
                TEST_EQUAL(xrGetInstanceProcAddr(instance, "xrGetOpenGLGraphicsRequirementsKHR",
                                                 reinterpret_cast<PFN_xrVoidFunction*>(&pfn_get_opengl_graphics_requirements_khr)),
                           XR_SUCCESS, "TestCreateDestroySession get xrGetOpenGLGraphicsRequirementsKHR function pointer");
                TEST_NOT_EQUAL(pfn_get_opengl_graphics_requirements_khr, nullptr,
                               "TestCreateDestroySession invalid xrGetOpenGLGraphicsRequirementsKHR function pointer");
                XrGraphicsRequirementsOpenGLKHR opengl_graphics_requirements = {};
                opengl_graphics_requirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR;
                opengl_graphics_requirements.next = nullptr;
                TEST_EQUAL(pfn_get_opengl_graphics_requirements_khr(instance, systemId, &opengl_graphics_requirements), XR_SUCCESS,
                           "TestCreateDestroySession calling xrGetOpenGLGraphicsRequirementsKHR");
#ifdef _WIN32
                win32_gl_graphics_binding.type = XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR;
                graphics_binding = reinterpret_cast<void*>(&win32_gl_graphics_binding);
#elif defined(OS_LINUX_XLIB)
                glx_gl_graphics_binding.type = XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR;
                // TODO: Just need something other than NULL here for now (for validation).  Eventually need
                //       to correctly put in a valid pointer to an Display
                glx_gl_graphics_binding.xDisplay = reinterpret_cast<Display*>(0xFFFFFFFF);
                graphics_binding = reinterpret_cast<void*>(&glx_gl_graphics_binding);
#elif defined(OS_LINUX_XCB) || defined(OS_LINUX_XCB_GLX)
                xcb_gl_graphics_binding.type = XR_TYPE_GRAPHICS_BINDING_OPENGL_XCB_KHR;
                // TODO: Just need something other than NULL here for now (for validation).  Eventually need
                //       to correctly put in a valid pointer to an xcb_connection_t
                xcb_gl_graphics_binding.connection = reinterpret_cast<xcb_connection_t*>(0xFFFFFFFF);
                graphics_binding = reinterpret_cast<void*>(&xcb_gl_graphics_binding);
#elif defined(OS_LINUX_WAYLAND)
                wayland_gl_graphics_binding.type = XR_TYPE_GRAPHICS_BINDING_OPENGL_WAYLAND_KHR;
                // TODO: Just need something other than NULL here for now (for validation).  Eventually need
                //       to correctly put in a valid pointer to an wl_display
                wayland_gl_graphics_binding.display = reinterpret_cast<wl_display*>(0xFFFFFFFF);
                graphics_binding = reinterpret_cast<void*>(&wayland_gl_graphics_binding);
#endif
            }
#endif  // XR_USE_GRAPHICS_API_OPENGL
#ifdef XR_USE_GRAPHICS_API_D3D11
            XrGraphicsBindingD3D11KHR d3d11_graphics_binding = {};
            if (g_graphics_api_to_use == GRAPHICS_API_D3D) {
                PFN_xrGetD3D11GraphicsRequirementsKHR pfn_get_d3d11_graphics_requirements_khr;
                TEST_EQUAL(xrGetInstanceProcAddr(instance, "xrGetD3D11GraphicsRequirementsKHR",
                                                 reinterpret_cast<PFN_xrVoidFunction*>(&pfn_get_d3d11_graphics_requirements_khr)),
                           XR_SUCCESS, "TestCreateDestroySession get xrGetD3D11GraphicsRequirementsKHR function pointer");
                TEST_NOT_EQUAL(pfn_get_d3d11_graphics_requirements_khr, nullptr,
                               "TestCreateDestroySession invalid xrGetD3D11GraphicsRequirementsKHR function pointer");
                XrGraphicsRequirementsD3D11KHR d3d11_graphics_requirements = {};
                d3d11_graphics_requirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR;
                d3d11_graphics_requirements.next = nullptr;
                TEST_EQUAL(pfn_get_d3d11_graphics_requirements_khr(instance, systemId, &d3d11_graphics_requirements), XR_SUCCESS,
                           "TestCreateDestroySession calling xrGetD3D11GraphicsRequirementsKHR");

                d3d11_graphics_binding.type = XR_TYPE_GRAPHICS_BINDING_D3D11_KHR;
                // TODO: Just need something other than NULL here for now (for validation).  Eventually need
                //       to correctly put in a valid pointer to an Display
                d3d11_graphics_binding.device = nullptr;
                graphics_binding = reinterpret_cast<void*>(&d3d11_graphics_binding);
            }
#endif  // XR_USE_GRAPHICS_API_D3D11

            // Create a session for us to begin
            XrSession session;
            XrSessionCreateInfo session_create_info = {};
            session_create_info.type = XR_TYPE_SESSION_CREATE_INFO;
            session_create_info.systemId = systemId;
            session_create_info.next = graphics_binding;
            TEST_EQUAL(xrCreateSession(instance, &session_create_info, &session), XR_SUCCESS, "xrCreateSession")
            TEST_EQUAL(xrDestroySession(session), XR_SUCCESS, "Destroying session")
        }
        TEST_EQUAL(xrDestroyInstance(instance), XR_SUCCESS, "Destroying instance")
    } catch (...) {
        TEST_FAIL("Exception triggered during test, automatic failure")
    }

    // Cleanup
    CleanupEnvironmentVariables();

    // Output results for this test
    TEST_REPORT(TestCreateDestroySession)
}

const char test_function_name[] = "MyTestFunctionName";
static char message_id[64];
static uint64_t object_handle;
static char object_name[64];
static uint32_t num_objects = 0;
const char test_message[] = "This is a test message - 1 . 2! 3";
static void* expected_user_value = NULL;
static bool captured_error = false;
static bool captured_warning = false;
static bool captured_info = false;
static bool captured_verbose = false;
static bool captured_general_message = false;
static bool captured_validation_message = false;
static bool captured_performance_message = false;
static bool user_data_correct = false;
static bool encountered_bad_type = false;
static bool encountered_bad_severity = false;
static bool function_matches = false;
static bool message_matches = false;
static bool message_id_matches = false;
static bool num_objects_matches = false;
static bool object_contents_match = false;
const char g_first_individual_label_name[] = "First individual label";
const char g_second_individual_label_name[] = "Second individual label";
const char g_third_individual_label_name[] = "Third individual label";
const char g_first_label_region_name[] = "First Label Region";
const char g_second_label_region_name[] = "Second Label Region";
uint32_t g_in_region_num = 0;
uint32_t g_individual_label_num = 0;
static bool g_expecting_labels = false;
static bool g_captured_labels = false;
static bool g_captured_only_expected_labels = false;

XrBool32 XRAPI_PTR TestDebugUtilsCallback(XrDebugUtilsMessageSeverityFlagsEXT messageSeverity,
                                          XrDebugUtilsMessageTypeFlagsEXT messageType,
                                          const XrDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData) {
    if (expected_user_value == userData) {
        user_data_correct = true;
    } else {
        user_data_correct = false;
    }

    if (!strcmp(callbackData->functionName, test_function_name)) {
        function_matches = true;
    }

    if (!strcmp(callbackData->messageId, message_id)) {
        message_id_matches = true;
    }

    if (!strcmp(callbackData->message, test_message)) {
        message_matches = true;
    }

    switch (messageType) {
        case XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            captured_general_message = true;
            break;
        case XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            captured_validation_message = true;
            break;
        case XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            captured_performance_message = true;
            break;
        default:
            encountered_bad_type = true;
            break;
    }

    switch (messageSeverity) {
        case XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            captured_error = true;
            break;
        case XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            captured_warning = true;
            break;
        case XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            captured_info = true;
            break;
        case XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            captured_verbose = true;
            break;
        default:
            encountered_bad_severity = true;
            break;
    }

    if (callbackData->objectCount == num_objects) {
        num_objects_matches = true;
        if (callbackData->objectCount == 3) {
            if (callbackData->objects[0].type == XR_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT &&
                callbackData->objects[0].next == nullptr && callbackData->objects[0].objectType == XR_OBJECT_TYPE_INSTANCE &&
                callbackData->objects[0].objectHandle == static_cast<uint64_t>(0xFEED0001) &&
                callbackData->objects[0].objectName == nullptr &&
                callbackData->objects[1].type == XR_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT &&
                callbackData->objects[1].next == nullptr && callbackData->objects[1].objectType == XR_OBJECT_TYPE_SESSION &&
                callbackData->objects[1].objectHandle == static_cast<uint64_t>(0xFEED0002) &&
                callbackData->objects[1].objectName == nullptr &&
                callbackData->objects[2].type == XR_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT &&
                callbackData->objects[2].next == nullptr && callbackData->objects[2].objectType == XR_OBJECT_TYPE_ACTION_SET &&
                callbackData->objects[2].objectHandle == static_cast<uint64_t>(0xFEED0003) &&
                callbackData->objects[2].objectName == nullptr) {
                object_contents_match = true;
            }
        } else if (callbackData->objectCount == 1) {
            // This should have an actual object and name
            if (callbackData->objects[0].type == XR_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT &&
                callbackData->objects[0].next == nullptr && callbackData->objects[0].objectType == XR_OBJECT_TYPE_INSTANCE &&
                callbackData->objects[0].objectHandle == object_handle &&
                !strcmp(callbackData->objects[0].objectName, object_name)) {
                object_contents_match = true;
            }
        }
    }

    if (callbackData->sessionLabelCount > 0) {
        g_captured_labels = true;
        g_captured_only_expected_labels = false;
        if (g_expecting_labels) {
            if (g_in_region_num == 0) {
                // Should not have any label region in list
                bool found_invalid = false;
                for (uint32_t label_index = 0; label_index < callbackData->sessionLabelCount; ++label_index) {
                    if (!strcmp(callbackData->sessionLabels[label_index].labelName, g_second_label_region_name) ||
                        !strcmp(callbackData->sessionLabels[label_index].labelName, g_first_label_region_name)) {
                        found_invalid = true;
                    }
                }
                if (!found_invalid) {
                    g_captured_only_expected_labels = true;
                }
            } else if (g_in_region_num == 2) {
                // Should have g_second_label_region_name in list
                // Should have g_first_label_region_name in list
                if (callbackData->sessionLabelCount >= 2 &&
                    (!strcmp(callbackData->sessionLabels[0].labelName, g_second_label_region_name) &&
                     !strcmp(callbackData->sessionLabels[1].labelName, g_first_label_region_name))) {
                    g_captured_only_expected_labels = true;
                }
            } else if (g_in_region_num == 1) {
                // Should not have g_second_label_region_name in list
                // Should have g_first_label_region_name in list
                if (callbackData->sessionLabelCount >= 2 &&
                    (strcmp(callbackData->sessionLabels[0].labelName, g_second_label_region_name) &&
                     !strcmp(callbackData->sessionLabels[1].labelName, g_first_label_region_name))) {
                    g_captured_only_expected_labels = true;
                } else if (callbackData->sessionLabelCount >= 1 &&
                           !strcmp(callbackData->sessionLabels[0].labelName, g_first_label_region_name)) {
                    g_captured_only_expected_labels = true;
                }
            }

            if (g_individual_label_num == 0) {
                // Should not have any individual label in list
                bool found_invalid = false;
                for (uint32_t label_index = 0; label_index < callbackData->sessionLabelCount; ++label_index) {
                    if (!strcmp(callbackData->sessionLabels[label_index].labelName, g_first_individual_label_name) ||
                        !strcmp(callbackData->sessionLabels[label_index].labelName, g_second_individual_label_name) ||
                        !strcmp(callbackData->sessionLabels[label_index].labelName, g_third_individual_label_name)) {
                        found_invalid = true;
                    }
                }
                if (found_invalid) {
                    g_captured_only_expected_labels = false;
                }
            }
            if (g_individual_label_num == 1) {
                // Should have g_first_individual_label_name in list
                // Should not have g_second_individual_label_name in list
                // Should not have g_third_individual_label_name in list
                bool found_invalid = false;
                bool found_valid = false;
                for (uint32_t label_index = 0; label_index < callbackData->sessionLabelCount; ++label_index) {
                    if (!strcmp(callbackData->sessionLabels[label_index].labelName, g_second_individual_label_name) ||
                        !strcmp(callbackData->sessionLabels[label_index].labelName, g_third_individual_label_name)) {
                        found_invalid = true;
                    }
                    if (!strcmp(callbackData->sessionLabels[label_index].labelName, g_first_individual_label_name)) {
                        found_valid = true;
                    }
                }
                if (found_invalid || !found_valid) {
                    g_captured_only_expected_labels = false;
                }
            } else if (g_individual_label_num == 2) {
                // Should not have g_first_individual_label_name in list
                // Should have g_second_individual_label_name in list
                // Should not have g_third_individual_label_name in list
                bool found_invalid = false;
                bool found_valid = false;
                for (uint32_t label_index = 0; label_index < callbackData->sessionLabelCount; ++label_index) {
                    if (!strcmp(callbackData->sessionLabels[label_index].labelName, g_first_individual_label_name) ||
                        !strcmp(callbackData->sessionLabels[label_index].labelName, g_third_individual_label_name)) {
                        found_invalid = true;
                    }
                    if (!strcmp(callbackData->sessionLabels[label_index].labelName, g_second_individual_label_name)) {
                        found_valid = true;
                    }
                }
                if (found_invalid || !found_valid) {
                    g_captured_only_expected_labels = false;
                }
            } else if (g_individual_label_num == 3) {
                // Should not have g_first_individual_label_name in list
                // Should not have g_second_individual_label_name in list
                // Should have g_third_individual_label_name in list
                bool found_invalid = false;
                bool found_valid = false;
                for (uint32_t label_index = 0; label_index < callbackData->sessionLabelCount; ++label_index) {
                    if (!strcmp(callbackData->sessionLabels[label_index].labelName, g_first_individual_label_name) ||
                        !strcmp(callbackData->sessionLabels[label_index].labelName, g_second_individual_label_name)) {
                        found_invalid = true;
                    }
                    if (!strcmp(callbackData->sessionLabels[label_index].labelName, g_third_individual_label_name)) {
                        found_valid = true;
                    }
                }
                if (found_invalid || !found_valid) {
                    g_captured_only_expected_labels = false;
                }
            }
        }
    }

    return XR_FALSE;
}

// Test the EXT_debug_utils extension.  Just to make sure it properly triggers at least one callback item
DEFINE_TEST(TestDebugUtils) {
    INIT_TEST(TestDebugUtils)

    try {
        std::string current_path;
        std::string sample_impl_runtime_path;
        std::string layer_path;

        if (!FileSysUtilsGetCurrentPath(current_path) ||
            !FileSysUtilsCombinePaths(current_path, "../../impl/openxr_sample_impl.json", sample_impl_runtime_path)) {
            std::cout << "FAILED to set runtime path!" << std::endl;
            throw - 1;
        }
        if (!FileSysUtilsCombinePaths(current_path, "../../api_layers", layer_path)) {
            std::cout << "FAILED to set layer path!" << std::endl;
            throw - 1;
        }
        LoaderTestSetEnvironmentVariable("XR_RUNTIME_JSON", sample_impl_runtime_path);
        LoaderTestSetEnvironmentVariable("XR_API_LAYER_PATH", layer_path);

        if (g_debug_utils_exists) {
            XrInstance instance = XR_NULL_HANDLE;
            XrInstanceCreateInfo instance_create_info = {};
            char debug_utils_extension_name[] = XR_EXT_DEBUG_UTILS_EXTENSION_NAME;
            std::string enabled_graphics_extension_name;
            const char* enabled_extension_array[2];
            instance_create_info.type = XR_TYPE_INSTANCE_CREATE_INFO;
            instance_create_info.enabledExtensionCount = 2;
            instance_create_info.enabledExtensionNames = enabled_extension_array;

            enabled_extension_array[0] = debug_utils_extension_name;
#ifdef XR_USE_GRAPHICS_API_VULKAN
            if (g_graphics_api_to_use == GRAPHICS_API_VULKAN) {
                enabled_graphics_extension_name = XR_KHR_VULKAN_ENABLE_EXTENSION_NAME;
                enabled_extension_array[1] = enabled_graphics_extension_name.c_str();
                instance_create_info.enabledExtensionNames = enabled_extension_array;
            }
#endif  // XR_USE_GRAPHICS_API_VULKAN
#ifdef XR_USE_GRAPHICS_API_OPENGL
            if (g_graphics_api_to_use == GRAPHICS_API_OPENGL) {
                enabled_graphics_extension_name = XR_KHR_OPENGL_ENABLE_EXTENSION_NAME;
                enabled_extension_array[1] = enabled_graphics_extension_name.c_str();
                instance_create_info.enabledExtensionNames = enabled_extension_array;
            }
#endif  // XR_USE_GRAPHICS_API_OPENGL
#ifdef XR_USE_GRAPHICS_API_D3D11
            if (g_graphics_api_to_use == GRAPHICS_API_D3D) {
                enabled_graphics_extension_name = XR_KHR_D3D11_ENABLE_EXTENSION_NAME;
                enabled_extension_array[1] = enabled_graphics_extension_name.c_str();
                instance_create_info.enabledExtensionNames = enabled_extension_array;
            }
#endif  // XR_USE_GRAPHICS_API_D3D11

            // Create an instance with the appropriate data for the debug utils messenger
            XrDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info = {};
            debug_utils_messenger_create_info.type = XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debug_utils_messenger_create_info.messageSeverities =
                XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
            debug_utils_messenger_create_info.messageTypes = XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                                             XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                                             XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debug_utils_messenger_create_info.userCallback = TestDebugUtilsCallback;

            // Create an instacne with the general information setup for a message
            XrDebugUtilsMessengerCallbackDataEXT debug_utils_callback_data = {};
            debug_utils_callback_data.type = XR_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT;
            debug_utils_callback_data.messageId = &message_id[0];
            debug_utils_callback_data.functionName = &test_function_name[0];
            debug_utils_callback_data.message = &test_message[0];
            debug_utils_callback_data.objectCount = 0;
            debug_utils_callback_data.objects = nullptr;
            debug_utils_callback_data.sessionLabelCount = 0;
            debug_utils_callback_data.sessionLabels = nullptr;

            // Path 1 - Create/Destroy with xrCreateInstance/xrDestroyInstance

            // Reset the capture values
            intptr_t def_int_value = 0xBADDECAF;
            expected_user_value = reinterpret_cast<void*>(def_int_value);
            captured_error = false;
            captured_warning = false;
            captured_info = false;
            captured_verbose = false;
            user_data_correct = false;
            captured_general_message = false;
            captured_validation_message = false;
            captured_performance_message = false;
            function_matches = false;
            message_matches = false;
            message_id_matches = false;
            num_objects_matches = false;
            num_objects = 0;
            debug_utils_messenger_create_info.userData = expected_user_value;

            instance_create_info.next = reinterpret_cast<const void*>(&debug_utils_messenger_create_info);

            TEST_EQUAL(xrCreateInstance(&instance_create_info, &instance), XR_SUCCESS,
                       "Creating instance with next messenger creation")

            // Get a function pointer to the submit function to test
            PFN_xrSubmitDebugUtilsMessageEXT pfn_submit_debug_utils_message_ext;
            TEST_EQUAL(xrGetInstanceProcAddr(instance, "xrSubmitDebugUtilsMessageEXT",
                                             reinterpret_cast<PFN_xrVoidFunction*>(&pfn_submit_debug_utils_message_ext)),
                       XR_SUCCESS, "TestDebugUtils get xrSubmitDebugUtilsMessageEXT function pointer");
            TEST_NOT_EQUAL(pfn_submit_debug_utils_message_ext, nullptr,
                           "TestDebugUtils invalid xrSubmitDebugUtilsMessageEXT function pointer");

            // Test the various items
            strcpy(message_id, "General Error");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, &debug_utils_callback_data);
            TEST_EQUAL(captured_error, true, "TestDebugUtils - General Error : Captured Error")
            TEST_EQUAL(captured_general_message, true, "TestDebugUtils - General Error : Captured General")
            TEST_EQUAL(user_data_correct, true, "TestDebugUtils - General Error : User Data Correct")
            TEST_EQUAL(function_matches, true, "TestDebugUtils - General Error : Function Matches")
            TEST_EQUAL(message_matches, true, "TestDebugUtils - General Error : Message Matches")
            TEST_EQUAL(message_id_matches, true, "TestDebugUtils - General Error : Message ID Matches")
            strcpy(message_id, "Validation Warning");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT, &debug_utils_callback_data);
            TEST_EQUAL(captured_warning, true, "TestDebugUtils - Validation Warning : Captured Warning")
            TEST_EQUAL(captured_validation_message, true, "TestDebugUtils - Validation Warning : Captured Validation")
            TEST_EQUAL(user_data_correct, true, "TestDebugUtils - Validation Warning : User Data Correct")
            TEST_EQUAL(function_matches, true, "TestDebugUtils - Validation Warning : Function Matches")
            TEST_EQUAL(message_matches, true, "TestDebugUtils - Validation Warning : Message Matches")
            TEST_EQUAL(message_id_matches, true, "TestDebugUtils - Validation Warning : Message ID Matches")
            strcpy(message_id, "Performance Info");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, &debug_utils_callback_data);
            TEST_EQUAL(captured_info, true, "TestDebugUtils - Performance Info : Captured Info")
            TEST_EQUAL(captured_performance_message, true, "TestDebugUtils - Performance Info : Captured Performance")
            TEST_EQUAL(user_data_correct, true, "TestDebugUtils - Performance Info : User Data Correct")
            TEST_EQUAL(function_matches, true, "TestDebugUtils - Performance Info : Function Matches")
            TEST_EQUAL(message_matches, true, "TestDebugUtils - Performance Info : Message Matches")
            TEST_EQUAL(message_id_matches, true, "TestDebugUtils - Performance Info : Message ID Matches")
            strcpy(message_id, "General Verbose");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, &debug_utils_callback_data);
            TEST_EQUAL(captured_verbose, true, "TestDebugUtils - General Verbose : Captured Verbose")
            TEST_EQUAL(user_data_correct, true, "TestDebugUtils - General Verbose : User Data Correct")
            TEST_EQUAL(function_matches, true, "TestDebugUtils - General Verbose : Function Matches")
            TEST_EQUAL(message_matches, true, "TestDebugUtils - General Verbose : Message Matches")
            TEST_EQUAL(message_id_matches, true, "TestDebugUtils - General Verbose : Message ID Matches")

            TEST_EQUAL(xrDestroyInstance(instance), XR_SUCCESS, "Destroying instance with next messenger creation")

            // Path 2 - Create/Destroy with explicit call (xrCreateDebugUtilsMessengerEXT/xrDestroyDebugUtilsMessengerEXT)

            // Reset the capture values
            def_int_value = 0xDECAFBAD;
            expected_user_value = reinterpret_cast<void*>(def_int_value);
            captured_error = false;
            captured_warning = false;
            captured_info = false;
            captured_verbose = false;
            user_data_correct = false;
            captured_general_message = false;
            captured_validation_message = false;
            captured_performance_message = false;
            function_matches = false;
            message_matches = false;
            message_id_matches = false;
            num_objects_matches = false;
            num_objects = 0;
            debug_utils_messenger_create_info.userData = expected_user_value;
            instance_create_info.next = nullptr;

            TEST_EQUAL(xrCreateInstance(&instance_create_info, &instance), XR_SUCCESS,
                       "Creating instance for separate create/destroy messenger")

            // Get a function pointer to the various debug utils functions to test
            PFN_xrCreateDebugUtilsMessengerEXT pfn_create_debug_utils_messager_ext;
            PFN_xrDestroyDebugUtilsMessengerEXT pfn_destroy_debug_utils_messager_ext;
            TEST_EQUAL(xrGetInstanceProcAddr(instance, "xrCreateDebugUtilsMessengerEXT",
                                             reinterpret_cast<PFN_xrVoidFunction*>(&pfn_create_debug_utils_messager_ext)),
                       XR_SUCCESS, "TestDebugUtils get xrCreateDebugUtilsMessengerEXT function pointer");
            TEST_NOT_EQUAL(pfn_create_debug_utils_messager_ext, nullptr,
                           "TestDebugUtils invalid xrCreateDebugUtilsMessengerEXT function pointer");
            TEST_EQUAL(xrGetInstanceProcAddr(instance, "xrDestroyDebugUtilsMessengerEXT",
                                             reinterpret_cast<PFN_xrVoidFunction*>(&pfn_destroy_debug_utils_messager_ext)),
                       XR_SUCCESS, "TestDebugUtils get xrDestroyDebugUtilsMessengerEXT function pointer");
            TEST_NOT_EQUAL(pfn_destroy_debug_utils_messager_ext, nullptr,
                           "TestDebugUtils invalid xrDestroyDebugUtilsMessengerEXT function pointer");
            TEST_EQUAL(xrGetInstanceProcAddr(instance, "xrSubmitDebugUtilsMessageEXT",
                                             reinterpret_cast<PFN_xrVoidFunction*>(&pfn_submit_debug_utils_message_ext)),
                       XR_SUCCESS, "TestDebugUtils get xrSubmitDebugUtilsMessageEXT function pointer");
            TEST_NOT_EQUAL(pfn_submit_debug_utils_message_ext, nullptr,
                           "TestDebugUtils invalid xrSubmitDebugUtilsMessageEXT function pointer");

            // Create the debug utils messenger
            XrDebugUtilsMessengerEXT debug_utils_messenger = XR_NULL_HANDLE;
            TEST_EQUAL(pfn_create_debug_utils_messager_ext(instance, &debug_utils_messenger_create_info, &debug_utils_messenger),
                       XR_SUCCESS, "Creating the debug utils messenger")

            // Test the various items
            strcpy(message_id, "General Error");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, &debug_utils_callback_data);
            TEST_EQUAL(captured_error, true, "TestDebugUtils - General Error : Captured Error")
            TEST_EQUAL(captured_general_message, true, "TestDebugUtils - General Error : Captured General")
            TEST_EQUAL(user_data_correct, true, "TestDebugUtils - General Error : User Data Correct")
            TEST_EQUAL(function_matches, true, "TestDebugUtils - General Error : Function Matches")
            TEST_EQUAL(message_matches, true, "TestDebugUtils - General Error : Message Matches")
            TEST_EQUAL(message_id_matches, true, "TestDebugUtils - General Error : Message ID Matches")
            TEST_EQUAL(num_objects_matches, true, "TestDebugUtils - General Error : Number of Objects Matches")
            strcpy(message_id, "Validation Warning");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT, &debug_utils_callback_data);
            TEST_EQUAL(captured_warning, true, "TestDebugUtils - Validation Warning : Captured Warning")
            TEST_EQUAL(captured_validation_message, true, "TestDebugUtils - Validation Warning : Captured Validation")
            TEST_EQUAL(user_data_correct, true, "TestDebugUtils - Validation Warning : User Data Correct")
            TEST_EQUAL(function_matches, true, "TestDebugUtils - Validation Warning : Function Matches")
            TEST_EQUAL(message_matches, true, "TestDebugUtils - Validation Warning : Message Matches")
            TEST_EQUAL(message_id_matches, true, "TestDebugUtils - Validation Warning : Message ID Matches")
            TEST_EQUAL(num_objects_matches, true, "TestDebugUtils - Validation Warning : Number of Objects Matches")
            strcpy(message_id, "Performance Info");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, &debug_utils_callback_data);
            TEST_EQUAL(captured_info, true, "TestDebugUtils - Performance Info : Captured Info")
            TEST_EQUAL(captured_performance_message, true, "TestDebugUtils - Performance Info : Captured Performance")
            TEST_EQUAL(user_data_correct, true, "TestDebugUtils - Performance Info : User Data Correct")
            TEST_EQUAL(function_matches, true, "TestDebugUtils - Performance Info : Function Matches")
            TEST_EQUAL(message_matches, true, "TestDebugUtils - Performance Info : Message Matches")
            TEST_EQUAL(message_id_matches, true, "TestDebugUtils - Performance Info : Message ID Matches")
            TEST_EQUAL(num_objects_matches, true, "TestDebugUtils - Performance Info : Number of Objects Matches")
            strcpy(message_id, "General Verbose");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, &debug_utils_callback_data);
            TEST_EQUAL(captured_verbose, true, "TestDebugUtils - General Verbose : Captured Verbose")
            TEST_EQUAL(user_data_correct, true, "TestDebugUtils - General Verbose : User Data Correct")
            TEST_EQUAL(function_matches, true, "TestDebugUtils - General Verbose : Function Matches")
            TEST_EQUAL(message_matches, true, "TestDebugUtils - General Verbose : Message Matches")
            TEST_EQUAL(message_id_matches, true, "TestDebugUtils - General Verbose : Message ID Matches")
            TEST_EQUAL(num_objects_matches, true, "TestDebugUtils - General Verbose : Number of Objects Matches")

            // Destroy what we created
            TEST_EQUAL(pfn_destroy_debug_utils_messager_ext(debug_utils_messenger), XR_SUCCESS,
                       "Destroying the debug utils messenger")

            TEST_EQUAL(xrDestroyInstance(instance), XR_SUCCESS, "Destroying instance for separate create/destroy messenger")

            // Path 3 - Make sure appropriate messages only received when registered.

            // Reset the capture values
            def_int_value = 0xDECAFBAD;
            expected_user_value = reinterpret_cast<void*>(def_int_value);
            captured_error = false;
            captured_warning = false;
            captured_info = false;
            captured_verbose = false;
            user_data_correct = false;
            captured_general_message = false;
            captured_validation_message = false;
            captured_performance_message = false;
            function_matches = false;
            message_matches = false;
            message_id_matches = false;
            num_objects_matches = false;
            num_objects = 0;
            debug_utils_messenger_create_info.userData = expected_user_value;
            instance_create_info.next = nullptr;

            TEST_EQUAL(xrCreateInstance(&instance_create_info, &instance), XR_SUCCESS,
                       "Creating instance for individual message type/severity testing")

            // Get a function pointer to the various debug utils functions to test
            TEST_EQUAL(xrGetInstanceProcAddr(instance, "xrCreateDebugUtilsMessengerEXT",
                                             reinterpret_cast<PFN_xrVoidFunction*>(&pfn_create_debug_utils_messager_ext)),
                       XR_SUCCESS, "TestDebugUtils get xrCreateDebugUtilsMessengerEXT function pointer");
            TEST_NOT_EQUAL(pfn_create_debug_utils_messager_ext, nullptr,
                           "TestDebugUtils invalid xrCreateDebugUtilsMessengerEXT function pointer");
            TEST_EQUAL(xrGetInstanceProcAddr(instance, "xrDestroyDebugUtilsMessengerEXT",
                                             reinterpret_cast<PFN_xrVoidFunction*>(&pfn_destroy_debug_utils_messager_ext)),
                       XR_SUCCESS, "TestDebugUtils get xrDestroyDebugUtilsMessengerEXT function pointer");
            TEST_NOT_EQUAL(pfn_destroy_debug_utils_messager_ext, nullptr,
                           "TestDebugUtils invalid xrDestroyDebugUtilsMessengerEXT function pointer");
            TEST_EQUAL(xrGetInstanceProcAddr(instance, "xrSubmitDebugUtilsMessageEXT",
                                             reinterpret_cast<PFN_xrVoidFunction*>(&pfn_submit_debug_utils_message_ext)),
                       XR_SUCCESS, "TestDebugUtils get xrSubmitDebugUtilsMessageEXT function pointer");
            TEST_NOT_EQUAL(pfn_submit_debug_utils_message_ext, nullptr,
                           "TestDebugUtils invalid xrSubmitDebugUtilsMessageEXT function pointer");

            // Create the debug utils messenger, but only to receive general error messages
            debug_utils_messenger = XR_NULL_HANDLE;
            debug_utils_messenger_create_info.messageSeverities = XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debug_utils_messenger_create_info.messageTypes = XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;

            TEST_EQUAL(pfn_create_debug_utils_messager_ext(instance, &debug_utils_messenger_create_info, &debug_utils_messenger),
                       XR_SUCCESS, "Creating the debug utils messenger for general error testing")

            strcpy(message_id, "General Error");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, &debug_utils_callback_data);
            strcpy(message_id, "Validation Warning");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT, &debug_utils_callback_data);
            strcpy(message_id, "Performance Info");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, &debug_utils_callback_data);
            strcpy(message_id, "General Verbose");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, &debug_utils_callback_data);
            TEST_EQUAL(captured_error, true, "TestDebugUtils - Only General Error : Captured Error")
            TEST_EQUAL(captured_warning, false, "TestDebugUtils - Only General Error : Captured Warning")
            TEST_EQUAL(captured_info, false, "TestDebugUtils - Only General Error : Captured Info")
            TEST_EQUAL(captured_verbose, false, "TestDebugUtils - Only General Error : Captured Verbose")
            TEST_EQUAL(captured_general_message, true, "TestDebugUtils - Only General Error : Captured General")
            TEST_EQUAL(captured_validation_message, false, "TestDebugUtils - Only General Error : Captured Validation")
            TEST_EQUAL(captured_performance_message, false, "TestDebugUtils - Only General Error : Captured Performance")
            TEST_EQUAL(user_data_correct, true, "TestDebugUtils - Only General Error : User Data Correct")
            TEST_EQUAL(function_matches, true, "TestDebugUtils - Only General Error : Function Matches")
            TEST_EQUAL(message_matches, true, "TestDebugUtils - Only General Error : Message Matches")
            TEST_EQUAL(message_id_matches, true, "TestDebugUtils - Only General Error : Message ID Matches")
            TEST_EQUAL(num_objects_matches, true, "TestDebugUtils - Only General Error : Number of Objects Matches")

            // Destroy what we created
            TEST_EQUAL(pfn_destroy_debug_utils_messager_ext(debug_utils_messenger), XR_SUCCESS,
                       "Destroying the debug utils messenger for general error testing")

            // Reset the items we tested above.
            captured_error = false;
            captured_general_message = false;

            // Create the debug utils messenger, but only to receive validation warning messages
            debug_utils_messenger = XR_NULL_HANDLE;
            debug_utils_messenger_create_info.messageSeverities = XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
            debug_utils_messenger_create_info.messageTypes = XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

            TEST_EQUAL(pfn_create_debug_utils_messager_ext(instance, &debug_utils_messenger_create_info, &debug_utils_messenger),
                       XR_SUCCESS, "Creating the debug utils messenger for validation warning testing")

            strcpy(message_id, "General Error");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, &debug_utils_callback_data);
            strcpy(message_id, "Validation Warning");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT, &debug_utils_callback_data);
            strcpy(message_id, "Performance Info");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, &debug_utils_callback_data);
            strcpy(message_id, "General Verbose");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, &debug_utils_callback_data);
            TEST_EQUAL(captured_error, false, "TestDebugUtils - Only Validation Warning : Captured Error")
            TEST_EQUAL(captured_warning, true, "TestDebugUtils - Only Validation Warning : Captured Warning")
            TEST_EQUAL(captured_info, false, "TestDebugUtils - Only Validation Warning : Captured Info")
            TEST_EQUAL(captured_verbose, false, "TestDebugUtils - Only Validation Warning : Captured Verbose")
            TEST_EQUAL(captured_general_message, false, "TestDebugUtils - Only Validation Warning : Captured General")
            TEST_EQUAL(captured_validation_message, true, "TestDebugUtils - Only Validation Warning : Captured Validation")
            TEST_EQUAL(captured_performance_message, false, "TestDebugUtils - Only Validation Warning : Captured Performance")
            TEST_EQUAL(user_data_correct, true, "TestDebugUtils - Only Validation Warning : User Data Correct")
            TEST_EQUAL(function_matches, true, "TestDebugUtils - Only Validation Warning : Function Matches")
            TEST_EQUAL(message_matches, true, "TestDebugUtils - Only Validation Warning : Message Matches")
            TEST_EQUAL(message_id_matches, true, "TestDebugUtils - Only Validation Warning : Message ID Matches")
            TEST_EQUAL(num_objects_matches, true, "TestDebugUtils - Only Validation Warning : Number of Objects Matches")

            // Destroy what we created
            TEST_EQUAL(pfn_destroy_debug_utils_messager_ext(debug_utils_messenger), XR_SUCCESS,
                       "Destroying the debug utils messenger for validation warning testing")

            // Reset the items we tested above.
            captured_warning = false;
            captured_validation_message = false;

            // Create the debug utils messenger, but only to receive validation warning messages
            debug_utils_messenger = XR_NULL_HANDLE;
            debug_utils_messenger_create_info.messageSeverities = XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
            debug_utils_messenger_create_info.messageTypes = XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

            TEST_EQUAL(pfn_create_debug_utils_messager_ext(instance, &debug_utils_messenger_create_info, &debug_utils_messenger),
                       XR_SUCCESS, "Creating the debug utils messenger for performance verbose testing")

            strcpy(message_id, "General Error");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, &debug_utils_callback_data);
            strcpy(message_id, "Validation Warning");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT, &debug_utils_callback_data);
            strcpy(message_id, "Performance Info");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, &debug_utils_callback_data);
            strcpy(message_id, "General Verbose");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, &debug_utils_callback_data);
            TEST_EQUAL(captured_error, false, "TestDebugUtils - Only Performance Verbose : Captured Error")
            TEST_EQUAL(captured_warning, false, "TestDebugUtils - Only Performance Verbose : Captured Warning")
            TEST_EQUAL(captured_info, false, "TestDebugUtils - Only Performance Verbose : Captured Info")
            TEST_EQUAL(captured_verbose, false, "TestDebugUtils - Only Performance Verbose : Captured Verbose")
            TEST_EQUAL(captured_general_message, false, "TestDebugUtils - Only Performance Verbose : Captured General")
            TEST_EQUAL(captured_validation_message, false, "TestDebugUtils - Only Performance Verbose : Captured Validation")
            TEST_EQUAL(captured_verbose, false, "TestDebugUtils - Only Performance Verbose : Captured Verbose")
            TEST_EQUAL(captured_performance_message, false, "TestDebugUtils - Only Performance Verbose : Captured Performance")
            TEST_EQUAL(user_data_correct, true, "TestDebugUtils - Only Performance Verbose : User Data Correct")
            TEST_EQUAL(function_matches, true, "TestDebugUtils - Only Performance Verbose : Function Matches")
            TEST_EQUAL(message_matches, true, "TestDebugUtils - Only Performance Verbose : Message Matches")
            TEST_EQUAL(message_id_matches, true, "TestDebugUtils - Only Performance Verbose : Message ID Matches")
            TEST_EQUAL(num_objects_matches, true, "TestDebugUtils - Only Performance Verbose : Number of Objects Matches")
            strcpy(message_id, "Performance Verbose");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, &debug_utils_callback_data);
            TEST_EQUAL(captured_verbose, true, "TestDebugUtils - Only Performance Verbose : Captured Verbose")
            TEST_EQUAL(captured_performance_message, true, "TestDebugUtils - Only Performance Verbose : Captured Performance")

            // Destroy what we created
            TEST_EQUAL(pfn_destroy_debug_utils_messager_ext(debug_utils_messenger), XR_SUCCESS,
                       "Destroying the debug utils messenger for performance verbose testing")

            // Reset the items we tested above.
            captured_verbose = false;
            captured_performance_message = false;

            // Create the debug utils messenger, but only to receive validation warning messages
            debug_utils_messenger = XR_NULL_HANDLE;
            debug_utils_messenger_create_info.messageSeverities = XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
            debug_utils_messenger_create_info.messageTypes = XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

            TEST_EQUAL(pfn_create_debug_utils_messager_ext(instance, &debug_utils_messenger_create_info, &debug_utils_messenger),
                       XR_SUCCESS, "Creating the debug utils messenger for validation info testing")

            strcpy(message_id, "General Error");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, &debug_utils_callback_data);
            strcpy(message_id, "Validation Warning");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT, &debug_utils_callback_data);
            strcpy(message_id, "Performance Info");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, &debug_utils_callback_data);
            strcpy(message_id, "General Verbose");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, &debug_utils_callback_data);
            TEST_EQUAL(captured_error, false, "TestDebugUtils - Only Validation Info : Captured Error")
            TEST_EQUAL(captured_warning, false, "TestDebugUtils - Only Validation Info : Captured Warning")
            TEST_EQUAL(captured_info, false, "TestDebugUtils - Only Validation Info : Captured Info")
            TEST_EQUAL(captured_verbose, false, "TestDebugUtils - Only Validation Info : Captured Verbose")
            TEST_EQUAL(captured_general_message, false, "TestDebugUtils - Only Validation Info : Captured General")
            TEST_EQUAL(captured_validation_message, false, "TestDebugUtils - Only Validation Info : Captured Validation")
            TEST_EQUAL(captured_performance_message, false, "TestDebugUtils - Only Validation Info : Captured Performance")
            TEST_EQUAL(user_data_correct, true, "TestDebugUtils - Only Validation Info : User Data Correct")
            TEST_EQUAL(function_matches, true, "TestDebugUtils - Only Validation Info : Function Matches")
            TEST_EQUAL(message_matches, true, "TestDebugUtils - Only Validation Info : Message Matches")
            TEST_EQUAL(message_id_matches, true, "TestDebugUtils - Only Validation Info : Message ID Matches")
            TEST_EQUAL(num_objects_matches, true, "TestDebugUtils - Only Validation Info : Number of Objects Matches")
            strcpy(message_id, "Validation Info");
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT, &debug_utils_callback_data);
            TEST_EQUAL(captured_info, true, "TestDebugUtils - Only Validation Info : Captured Info")
            TEST_EQUAL(captured_validation_message, true, "TestDebugUtils - Only Validation Info : Captured Validation")

            // Destroy what we created
            TEST_EQUAL(pfn_destroy_debug_utils_messager_ext(debug_utils_messenger), XR_SUCCESS,
                       "Destroying the debug utils messenger for validation info testing")

            // Reset the items we tested above.
            captured_info = false;
            captured_validation_message = false;

            // Path 4 - Test objects

            // Create the debug utils messenger, but only to receive validation warning messages
            debug_utils_messenger = XR_NULL_HANDLE;
            debug_utils_messenger_create_info.messageSeverities = XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
            debug_utils_messenger_create_info.messageTypes = XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

            TEST_EQUAL(pfn_create_debug_utils_messager_ext(instance, &debug_utils_messenger_create_info, &debug_utils_messenger),
                       XR_SUCCESS, "Creating the debug utils messenger for number of objects testing")

            num_objects_matches = false;
            object_contents_match = false;

            num_objects = 3;
            debug_utils_callback_data.objectCount = static_cast<uint8_t>(num_objects);
            XrDebugUtilsObjectNameInfoEXT objects[3];
            objects[0].type = XR_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            objects[0].next = nullptr;
            objects[0].objectType = XR_OBJECT_TYPE_INSTANCE;
            objects[0].objectHandle = static_cast<uint64_t>(0xFEED0001);
            objects[0].objectName = nullptr;
            objects[1].type = XR_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            objects[1].next = nullptr;
            objects[1].objectType = XR_OBJECT_TYPE_SESSION;
            objects[1].objectHandle = static_cast<uint64_t>(0xFEED0002);
            objects[1].objectName = nullptr;
            objects[2].type = XR_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            objects[2].next = nullptr;
            objects[2].objectType = XR_OBJECT_TYPE_ACTION_SET;
            objects[2].objectHandle = static_cast<uint64_t>(0xFEED0003);
            objects[2].objectName = nullptr;
            debug_utils_callback_data.objects = &objects[0];
            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, &debug_utils_callback_data);
            TEST_EQUAL(num_objects_matches, true, "TestDebugUtils - Number of Objects Matches")
            TEST_EQUAL(object_contents_match, true, "TestDebugUtils - Object Content Matches")
            num_objects_matches = false;
            object_contents_match = false;

            // Path 5 - Test object names
            PFN_xrSetDebugUtilsObjectNameEXT pfn_set_debug_utils_object_name_ext;
            TEST_EQUAL(xrGetInstanceProcAddr(instance, "xrSetDebugUtilsObjectNameEXT",
                                             reinterpret_cast<PFN_xrVoidFunction*>(&pfn_set_debug_utils_object_name_ext)),
                       XR_SUCCESS, "TestDebugUtils get xrSetDebugUtilsObjectNameEXT function pointer");
            TEST_NOT_EQUAL(pfn_set_debug_utils_object_name_ext, nullptr,
                           "TestDebugUtils invalid xrSetDebugUtilsObjectNameEXT function pointer");
            strcpy(object_name, "My Instance Obj");
            object_handle = reinterpret_cast<uint64_t&>(instance);
            objects[0].type = XR_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            objects[0].next = nullptr;
            objects[0].objectType = XR_OBJECT_TYPE_INSTANCE;
            objects[0].objectHandle = object_handle;
            objects[0].objectName = &object_name[0];
            TEST_EQUAL(pfn_set_debug_utils_object_name_ext(instance, &objects[0]), XR_SUCCESS, "Setting object name")
            num_objects = 1;
            debug_utils_callback_data.objectCount = static_cast<uint8_t>(num_objects);

            pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
                                               XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, &debug_utils_callback_data);
            TEST_EQUAL(num_objects_matches, true, "TestDebugUtils - Name Set: Objects Matches")
            TEST_EQUAL(object_contents_match, true, "TestDebugUtils - Name Set: Object Content Matches")
            num_objects_matches = false;
            object_contents_match = false;

            XrSystemGetInfo system_get_info;
            memset(&system_get_info, 0, sizeof(system_get_info));
            system_get_info.type = XR_TYPE_SYSTEM_GET_INFO;
            system_get_info.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
            XrSystemId systemId;
            TEST_EQUAL(xrGetSystem(instance, &system_get_info, &systemId), XR_SUCCESS, "xrGetSystem");

            if (systemId) {
                void* graphics_binding = nullptr;

#ifdef XR_USE_GRAPHICS_API_VULKAN
                XrGraphicsBindingVulkanKHR vulkan_graphics_binding = {};
                if (g_graphics_api_to_use == GRAPHICS_API_VULKAN) {
                    PFN_xrGetVulkanInstanceExtensionsKHR pfn_get_vulkan_instance_extensions_khr;
                    PFN_xrGetVulkanDeviceExtensionsKHR pfn_get_vulkan_device_extensions_khr;
                    PFN_xrGetVulkanGraphicsDeviceKHR pfn_get_vulkan_graphics_device_khr;
                    PFN_xrGetVulkanGraphicsRequirementsKHR pfn_get_vulkan_graphics_requirements_khr;
                    TEST_EQUAL(
                        xrGetInstanceProcAddr(instance, "xrGetVulkanInstanceExtensionsKHR",
                                              reinterpret_cast<PFN_xrVoidFunction*>(&pfn_get_vulkan_instance_extensions_khr)),
                        XR_SUCCESS, "TestDebugUtils get xrGetVulkanInstanceExtensionsKHR function pointer");
                    TEST_NOT_EQUAL(pfn_get_vulkan_instance_extensions_khr, nullptr,
                                   "TestDebugUtils invalid xrGetVulkanInstanceExtensionsKHR function pointer");
                    TEST_EQUAL(xrGetInstanceProcAddr(instance, "xrGetVulkanDeviceExtensionsKHR",
                                                     reinterpret_cast<PFN_xrVoidFunction*>(&pfn_get_vulkan_device_extensions_khr)),
                               XR_SUCCESS, "TestDebugUtils get xrGetVulkanDeviceExtensionsKHR function pointer");
                    TEST_NOT_EQUAL(pfn_get_vulkan_device_extensions_khr, nullptr,
                                   "TestDebugUtils invalid xrGetVulkanDeviceExtensionsKHR function pointer");
                    TEST_EQUAL(xrGetInstanceProcAddr(instance, "xrGetVulkanGraphicsDeviceKHR",
                                                     reinterpret_cast<PFN_xrVoidFunction*>(&pfn_get_vulkan_graphics_device_khr)),
                               XR_SUCCESS, "TestDebugUtils get xrGetVulkanGraphicsDeviceKHR function pointer");
                    TEST_NOT_EQUAL(pfn_get_vulkan_graphics_device_khr, nullptr,
                                   "TestDebugUtils invalid xrGetVulkanGraphicsDeviceKHR function pointer");
                    TEST_EQUAL(
                        xrGetInstanceProcAddr(instance, "xrGetVulkanGraphicsRequirementsKHR",
                                              reinterpret_cast<PFN_xrVoidFunction*>(&pfn_get_vulkan_graphics_requirements_khr)),
                        XR_SUCCESS, "TestDebugUtils get xrGetVulkanGraphicsRequirementsKHR function pointer");
                    TEST_NOT_EQUAL(pfn_get_vulkan_graphics_requirements_khr, nullptr,
                                   "TestDebugUtils invalid xrGetVulkanGraphicsRequirementsKHR function pointer");

                    XrGraphicsRequirementsVulkanKHR vulkan_graphics_requirements = {};
                    TEST_EQUAL(pfn_get_vulkan_graphics_requirements_khr(instance, systemId, &vulkan_graphics_requirements),
                               XR_SUCCESS, "TestDebugUtils calling xrGetVulkanGraphicsRequirementsKHR");

                    vulkan_graphics_binding.type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR;
                    // TODO: Just need something other than NULL here for now (for validation).  Eventually need
                    //       to correctly put in a valid pointer to an Display
                    vulkan_graphics_binding.instance = VK_NULL_HANDLE;
                    vulkan_graphics_binding.physicalDevice = VK_NULL_HANDLE;
                    vulkan_graphics_binding.device = VK_NULL_HANDLE;
                    vulkan_graphics_binding.queueFamilyIndex = 0;
                    vulkan_graphics_binding.queueIndex = 0;
                    graphics_binding = reinterpret_cast<void*>(&vulkan_graphics_binding);
                }
#endif  // XR_USE_GRAPHICS_API_VULKAN
#ifdef XR_USE_GRAPHICS_API_OPENGL
#ifdef _WIN32
                XrGraphicsBindingOpenGLWin32KHR win32_gl_graphics_binding = {};
#elif defined(OS_LINUX_XLIB)
                XrGraphicsBindingOpenGLXlibKHR glx_gl_graphics_binding = {};
#elif defined(OS_LINUX_XCB) || defined(OS_LINUX_XCB_GLX)
                XrGraphicsBindingOpenGLXcbKHR xcb_gl_graphics_binding = {};
#elif defined(OS_LINUX_WAYLAND)
                XrGraphicsBindingOpenGLWaylandKHR wayland_gl_graphics_binding = {};
#endif
                if (g_graphics_api_to_use == GRAPHICS_API_OPENGL) {
                    PFN_xrGetOpenGLGraphicsRequirementsKHR pfn_get_opengl_graphics_requirements_khr;
                    TEST_EQUAL(
                        xrGetInstanceProcAddr(instance, "xrGetOpenGLGraphicsRequirementsKHR",
                                              reinterpret_cast<PFN_xrVoidFunction*>(&pfn_get_opengl_graphics_requirements_khr)),
                        XR_SUCCESS, "TestDebugUtils get xrGetOpenGLGraphicsRequirementsKHR function pointer");
                    TEST_NOT_EQUAL(pfn_get_opengl_graphics_requirements_khr, nullptr,
                                   "TestDebugUtils invalid xrGetOpenGLGraphicsRequirementsKHR function pointer");
                    XrGraphicsRequirementsOpenGLKHR opengl_graphics_requirements = {};
                    TEST_EQUAL(pfn_get_opengl_graphics_requirements_khr(instance, systemId, &opengl_graphics_requirements),
                               XR_SUCCESS, "TestDebugUtils calling xrGetOpenGLGraphicsRequirementsKHR");
#ifdef _WIN32
                    win32_gl_graphics_binding.type = XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR;
                    graphics_binding = reinterpret_cast<void*>(&win32_gl_graphics_binding);
#elif defined(OS_LINUX_XLIB)
                    glx_gl_graphics_binding.type = XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR;
                    // TODO: Just need something other than NULL here for now (for validation).  Eventually need
                    //       to correctly put in a valid pointer to an Display
                    glx_gl_graphics_binding.xDisplay = reinterpret_cast<Display*>(0xFFFFFFFF);
                    graphics_binding = reinterpret_cast<void*>(&glx_gl_graphics_binding);
#elif defined(OS_LINUX_XCB) || defined(OS_LINUX_XCB_GLX)
                    xcb_gl_graphics_binding.type = XR_TYPE_GRAPHICS_BINDING_OPENGL_XCB_KHR;
                    // TODO: Just need something other than NULL here for now (for validation).  Eventually need
                    //       to correctly put in a valid pointer to an xcb_connection_t
                    xcb_gl_graphics_binding.connection = reinterpret_cast<xcb_connection_t*>(0xFFFFFFFF);
                    graphics_binding = reinterpret_cast<void*>(&xcb_gl_graphics_binding);
#elif defined(OS_LINUX_WAYLAND)
                    wayland_gl_graphics_binding.type = XR_TYPE_GRAPHICS_BINDING_OPENGL_WAYLAND_KHR;
                    // TODO: Just need something other than NULL here for now (for validation).  Eventually need
                    //       to correctly put in a valid pointer to an wl_display
                    wayland_gl_graphics_binding.display = reinterpret_cast<wl_display*>(0xFFFFFFFF);
                    graphics_binding = reinterpret_cast<void*>(&wayland_gl_graphics_binding);
#endif
                }
#endif  // XR_USE_GRAPHICS_API_OPENGL
#ifdef XR_USE_GRAPHICS_API_D3D11
                XrGraphicsBindingD3D11KHR d3d11_graphics_binding = {};
                if (g_graphics_api_to_use == GRAPHICS_API_D3D) {
                    PFN_xrGetD3D11GraphicsRequirementsKHR pfn_get_d3d11_graphics_requirements_khr;
                    TEST_EQUAL(
                        xrGetInstanceProcAddr(instance, "xrGetD3D11GraphicsRequirementsKHR",
                                              reinterpret_cast<PFN_xrVoidFunction*>(&pfn_get_d3d11_graphics_requirements_khr)),
                        XR_SUCCESS, "TestDebugUtils get xrGetD3D11GraphicsRequirementsKHR function pointer");
                    TEST_NOT_EQUAL(pfn_get_d3d11_graphics_requirements_khr, nullptr,
                                   "TestDebugUtils invalid xrGetD3D11GraphicsRequirementsKHR function pointer");
                    XrGraphicsRequirementsD3D11KHR d3d11_graphics_requirements = {};
                    TEST_EQUAL(pfn_get_d3d11_graphics_requirements_khr(instance, systemId, &d3d11_graphics_requirements),
                               XR_SUCCESS, "TestDebugUtils calling xrGetD3D11GraphicsRequirementsKHR");

                    d3d11_graphics_binding.type = XR_TYPE_GRAPHICS_BINDING_D3D11_KHR;
                    // TODO: Just need something other than NULL here for now (for validation).  Eventually need
                    //       to correctly put in a valid pointer to an Display
                    d3d11_graphics_binding.device = nullptr;
                    graphics_binding = reinterpret_cast<void*>(&d3d11_graphics_binding);
                }
#endif  // XR_USE_GRAPHICS_API_D3D11

                // Get a function pointer to the various debug utils functions to test
                PFN_xrSessionBeginDebugUtilsLabelRegionEXT pfn_begin_debug_utils_label_region_ext;
                PFN_xrSessionEndDebugUtilsLabelRegionEXT pfn_end_debug_utils_label_region_ext;
                PFN_xrSessionInsertDebugUtilsLabelEXT pfn_insert_debug_utils_label_ext;
                TEST_EQUAL(xrGetInstanceProcAddr(instance, "xrSessionBeginDebugUtilsLabelRegionEXT",
                                                 reinterpret_cast<PFN_xrVoidFunction*>(&pfn_begin_debug_utils_label_region_ext)),
                           XR_SUCCESS, "TestDebugUtils get xrSessionBeginDebugUtilsLabelRegionEXT function pointer");
                TEST_NOT_EQUAL(pfn_begin_debug_utils_label_region_ext, nullptr,
                               "TestDebugUtils invalid xrSessionBeginDebugUtilsLabelRegionEXT function pointer");
                TEST_EQUAL(xrGetInstanceProcAddr(instance, "xrSessionEndDebugUtilsLabelRegionEXT",
                                                 reinterpret_cast<PFN_xrVoidFunction*>(&pfn_end_debug_utils_label_region_ext)),
                           XR_SUCCESS, "TestDebugUtils get xrSessionEndDebugUtilsLabelRegionEXT function pointer");
                TEST_NOT_EQUAL(pfn_end_debug_utils_label_region_ext, nullptr,
                               "TestDebugUtils invalid xrSessionEndDebugUtilsLabelRegionEXT function pointer");
                TEST_EQUAL(xrGetInstanceProcAddr(instance, "xrSessionInsertDebugUtilsLabelEXT",
                                                 reinterpret_cast<PFN_xrVoidFunction*>(&pfn_insert_debug_utils_label_ext)),
                           XR_SUCCESS, "TestDebugUtils get xrSessionInsertDebugUtilsLabelEXT function pointer");
                TEST_NOT_EQUAL(pfn_insert_debug_utils_label_ext, nullptr,
                               "TestDebugUtils invalid xrSessionInsertDebugUtilsLabelEXT function pointer");

                g_expecting_labels = true;

                // Create a label struct for initial testing
                XrDebugUtilsLabelEXT first_label = {};
                first_label.type = XR_TYPE_DEBUG_UTILS_LABEL_EXT;
                first_label.labelName = g_first_individual_label_name;

                // Create a session for us to begin
                XrSession session;
                XrSessionCreateInfo session_create_info = {};
                session_create_info.type = XR_TYPE_SESSION_CREATE_INFO;
                session_create_info.systemId = systemId;
                session_create_info.next = graphics_binding;
                TEST_EQUAL(xrCreateSession(instance, &session_create_info, &session), XR_SUCCESS, "xrCreateSession")

                // Set it up to put in the session and instance to any debug utils messages
                num_objects = 2;
                debug_utils_callback_data.objectCount = static_cast<uint8_t>(num_objects);
                objects[0].type = XR_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                objects[0].next = nullptr;
                objects[0].objectType = XR_OBJECT_TYPE_INSTANCE;
                objects[0].objectHandle = reinterpret_cast<uint64_t&>(instance);
                objects[0].objectName = nullptr;
                objects[1].type = XR_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                objects[1].next = nullptr;
                objects[1].objectType = XR_OBJECT_TYPE_SESSION;
                objects[1].objectHandle = reinterpret_cast<uint64_t&>(session);
                objects[1].objectName = nullptr;
                debug_utils_callback_data.objects = &objects[0];

                // Try invalid session on each of the label functions
                TEST_EQUAL(pfn_begin_debug_utils_label_region_ext(XR_NULL_HANDLE, &first_label), XR_ERROR_HANDLE_INVALID,
                           "TestDebugUtils calling xrSessionBeginDebugUtilsLabelRegionEXT with invalid session");
                TEST_EQUAL(pfn_end_debug_utils_label_region_ext(XR_NULL_HANDLE), XR_ERROR_HANDLE_INVALID,
                           "TestDebugUtils calling xrSessionEndDebugUtilsLabelRegionEXT with invalid session");
                TEST_EQUAL(pfn_insert_debug_utils_label_ext(XR_NULL_HANDLE, &first_label), XR_ERROR_HANDLE_INVALID,
                           "TestDebugUtils calling xrSessionInsertDebugUtilsLabelEXT with invalid session");

                // Try with nullptr for the label
                TEST_EQUAL(pfn_begin_debug_utils_label_region_ext(session, nullptr), XR_ERROR_VALIDATION_FAILURE,
                           "TestDebugUtils calling xrSessionBeginDebugUtilsLabelRegionEXT with NULL label");
                TEST_EQUAL(pfn_insert_debug_utils_label_ext(session, nullptr), XR_ERROR_VALIDATION_FAILURE,
                           "TestDebugUtils calling xrSessionInsertDebugUtilsLabelEXT with NULL label");

                // Start an individual label
                TEST_EQUAL(pfn_insert_debug_utils_label_ext(session, &first_label), XR_SUCCESS,
                           "TestDebugUtils calling xrSessionInsertDebugUtilsLabelEXT");

                // Trigger a message and make sure we see "First individual label"
                g_in_region_num = 0;
                g_individual_label_num = 1;
                g_captured_labels = false;
                g_captured_only_expected_labels = false;
                strcpy(message_id, "First Individual Label");
                pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
                                                   XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, &debug_utils_callback_data);
                TEST_EQUAL(g_captured_labels, true, "TestDebugUtils - First Individual Label : Captured Labels")
                TEST_EQUAL(g_captured_only_expected_labels, true,
                           "TestDebugUtils - First Individual Label : Captured Correct Labels")

                // Begin a label region
                first_label.labelName = g_first_label_region_name;
                TEST_EQUAL(pfn_begin_debug_utils_label_region_ext(session, &first_label), XR_SUCCESS,
                           "TestDebugUtils calling xrSessionBeginDebugUtilsLabelRegionEXT");

                // Trigger a message and make sure we see "Label Region" and not "First individual label"
                g_in_region_num = 1;
                g_individual_label_num = 0;
                g_captured_labels = false;
                g_captured_only_expected_labels = false;
                strcpy(message_id, "First Label Region");
                pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
                                                   XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, &debug_utils_callback_data);
                TEST_EQUAL(g_captured_labels, true, "TestDebugUtils - First Label Region : Captured Labels")
                TEST_EQUAL(g_captured_only_expected_labels, true, "TestDebugUtils - First Label Region : Captured Correct Labels")

                // Begin the session now.
                XrSessionBeginInfo session_begin_info = {};
                session_begin_info.type = XR_TYPE_SESSION_BEGIN_INFO;
                TEST_EQUAL(xrBeginSession(session, &session_begin_info), XR_SUCCESS, "xrBeginSession")

                XrDebugUtilsLabelEXT individual_label = {};
                individual_label.type = XR_TYPE_DEBUG_UTILS_LABEL_EXT;
                individual_label.labelName = g_second_individual_label_name;
                TEST_EQUAL(pfn_insert_debug_utils_label_ext(session, &individual_label), XR_SUCCESS,
                           "TestDebugUtils calling xrSessionInsertDebugUtilsLabelEXT");

                // TODO: Trigger a message and make sure we see "Second individual" and "First Label Region" and not "First
                // individual label"
                g_in_region_num = 1;
                g_individual_label_num = 2;
                g_captured_labels = false;
                g_captured_only_expected_labels = false;
                strcpy(message_id, "Second Individual and First Region");
                pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
                                                   XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, &debug_utils_callback_data);
                TEST_EQUAL(g_captured_labels, true, "TestDebugUtils - Second Individual and First Region : Captured Labels")
                TEST_EQUAL(g_captured_only_expected_labels, true,
                           "TestDebugUtils - Second Individual and First Region : Captured Correct Labels")

                individual_label.labelName = g_third_individual_label_name;
                TEST_EQUAL(pfn_insert_debug_utils_label_ext(session, &individual_label), XR_SUCCESS,
                           "TestDebugUtils calling xrSessionInsertDebugUtilsLabelEXT");

                // TODO: Trigger a message and make sure we see "Third individual" and "First Label Region" and not "First
                // individual label" or "Second individual label"
                g_in_region_num = 1;
                g_individual_label_num = 3;
                g_captured_labels = false;
                g_captured_only_expected_labels = false;
                strcpy(message_id, "Third Individual and First Region");
                pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
                                                   XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, &debug_utils_callback_data);
                TEST_EQUAL(g_captured_labels, true, "TestDebugUtils - Third Individual and First Region : Captured Labels")
                TEST_EQUAL(g_captured_only_expected_labels, true,
                           "TestDebugUtils - Third Individual and First Region : Captured Correct Labels")

                // Begin a label region
                XrDebugUtilsLabelEXT second_label_region = {};
                second_label_region.type = XR_TYPE_DEBUG_UTILS_LABEL_EXT;
                second_label_region.labelName = g_second_label_region_name;
                TEST_EQUAL(pfn_begin_debug_utils_label_region_ext(session, &second_label_region), XR_SUCCESS,
                           "TestDebugUtils calling xrSessionBeginDebugUtilsLabelRegionEXT");

                // TODO: Trigger a message and make sure we see "Second Label Region" and "First Label Region"
                g_in_region_num = 2;
                g_individual_label_num = 0;
                g_captured_labels = false;
                g_captured_only_expected_labels = false;
                strcpy(message_id, "Second and First Label Regions");
                pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
                                                   XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, &debug_utils_callback_data);
                TEST_EQUAL(g_captured_labels, true, "TestDebugUtils - Second and First Label Regions : Captured Labels")
                TEST_EQUAL(g_captured_only_expected_labels, true,
                           "TestDebugUtils - Second and First Label Regions : Captured Correct Labels")

                // End the last label region
                TEST_EQUAL(pfn_end_debug_utils_label_region_ext(session), XR_SUCCESS,
                           "TestDebugUtils calling xrSessionEndDebugUtilsLabelRegionEXT");

                // TODO: Trigger a message and make sure we see "First Label Region"
                g_in_region_num = 1;
                g_individual_label_num = 0;
                g_captured_labels = false;
                g_captured_only_expected_labels = false;
                strcpy(message_id, "First Label Region 2");
                pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
                                                   XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, &debug_utils_callback_data);
                TEST_EQUAL(g_captured_labels, true, "TestDebugUtils - First Label Region 2 : Captured Labels")
                TEST_EQUAL(g_captured_only_expected_labels, true, "TestDebugUtils - First Label Region 2 : Captured Correct Labels")

                // Now clean-up
                TEST_EQUAL(xrEndSession(session), XR_SUCCESS, "Ending session")

                // End the last label region
                TEST_EQUAL(pfn_end_debug_utils_label_region_ext(session), XR_SUCCESS,
                           "TestDebugUtils calling xrSessionEndDebugUtilsLabelRegionEXT");

                // TODO: Trigger a message and make sure we see no labels
                g_expecting_labels = false;
                g_in_region_num = 0;
                g_individual_label_num = 0;
                g_captured_labels = false;
                g_captured_only_expected_labels = false;
                strcpy(message_id, "No Labels");
                pfn_submit_debug_utils_message_ext(instance, XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
                                                   XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, &debug_utils_callback_data);
                TEST_EQUAL(g_captured_labels, false, "TestDebugUtils - No Labels : Captured Labels")

                TEST_EQUAL(xrDestroySession(session), XR_SUCCESS, "Destroying session")
            }

            // Destroy what we created
            TEST_EQUAL(pfn_destroy_debug_utils_messager_ext(debug_utils_messenger), XR_SUCCESS,
                       "Destroying the debug utils messenger for number of objects testing")

            TEST_EQUAL(xrDestroyInstance(instance), XR_SUCCESS, "Destroying instance for individual message type/severity testing")
        }
    } catch (...) {
        TEST_FAIL("Exception triggered during test, automatic failure")
    }

    // Cleanup
    CleanupEnvironmentVariables();

    // Output results for this test
    TEST_REPORT(TestDebugUtils)
}

int main(int argc, char* argv[]) {
    uint32_t total_tests = 0;
    uint32_t total_passed = 0;
    uint32_t total_skipped = 0;
    uint32_t total_failed = 0;

    // Unused for now.
    (void)argc;
    (void)argv;

#if FILTER_OUT_LOADER_ERRORS == 1
    // Re-direct std::cerr to a string since we're intentionally causing errors and we don't
    // want it polluting the output stream.
    std::stringstream buffer;
    std::streambuf* original_cerr = nullptr;
    original_cerr = std::cerr.rdbuf(buffer.rdbuf());
#endif

    std::cout << "Starting loader_test" << std::endl << "--------------------" << std::endl;

    TestEnumLayers(total_tests, total_passed, total_skipped, total_failed);
    TestEnumInstanceExtensions(total_tests, total_passed, total_skipped, total_failed);
    TestCreateDestroyInstance(total_tests, total_passed, total_skipped, total_failed);
    TestGetSystem(total_tests, total_passed, total_skipped, total_failed);
    TestCreateDestroySession(total_tests, total_passed, total_skipped, total_failed);
    TestDebugUtils(total_tests, total_passed, total_skipped, total_failed);

#if FILTER_OUT_LOADER_ERRORS == 1
    // Restore std::cerr to the original buffer
    std::cerr.rdbuf(original_cerr);
#endif

    // Cleanup
    CleanupEnvironmentVariables();

    std::cout << "    Results:" << std::endl << "    ------------------------------" << std::endl;
    std::cout << "        Total Tests:    " << std::to_string(total_tests) << std::endl;
    std::cout << "        Tests Passed:   " << std::to_string(total_passed) << std::endl;
    std::cout << "        Tests Skipped:  " << std::to_string(total_skipped) << std::endl;
    std::cout << "        Tests Failed:   " << std::to_string(total_failed) << std::endl;
    std::cout << "        Overall Result: ";
    if (total_failed > 0) {
        std::cout << "Failed" << std::endl;
    } else {
        std::cout << "Passed" << std::endl;
    }
    return 0;
}
