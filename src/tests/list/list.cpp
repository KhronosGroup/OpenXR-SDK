// Copyright (c) 2017 Collabora, Ltd.
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
// Author: Jakob Bornecrantz <jakob.bornecrantz@collabora.com>
//

// We do not need any graphics.
#define _CRT_SECURE_NO_WARNINGS 1
#include "xr_dependencies.h"
#include <openxr/openxr.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

// Struct that does book keeping of what a
// OpenXr application need to keep track of.
struct Program {
   public:
    XrInstance instance;

    uint32_t numPhysDevs;
    XrPath* physDevs;

   public:
    Program() {
        numPhysDevs = 0;
        physDevs = nullptr;
    }
    ~Program() {
        if (physDevs != NULL) {
            free(physDevs);
            numPhysDevs = 0;
            physDevs = NULL;
        }

        if (instance != XR_NULL_HANDLE) {
            xrDestroyInstance(instance);
            instance = XR_NULL_HANDLE;
        }
    }
};

// This below function is written in as close to "C style" as
// possible, so users of C (and other languages) are not too
// bogged down with C++ism. The cleanup code is hidden in
// the struct so the example is lighter but still correct.
int main() {
    Program program = {};

    // Start with creating a instance.
    XrInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.applicationInfo = {};
    strncpy(instanceCreateInfo.applicationInfo.applicationName, "List", XR_MAX_APPLICATION_NAME_SIZE);
    instanceCreateInfo.applicationInfo.applicationVersion = 1;
    strncpy(instanceCreateInfo.applicationInfo.engineName, "List Engine", XR_MAX_ENGINE_NAME_SIZE);
    instanceCreateInfo.applicationInfo.engineVersion = 1;
    instanceCreateInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;

    if (xrCreateInstance(&instanceCreateInfo, &program.instance) != XR_SUCCESS) {
        fprintf(stderr, "Failed to create XR instance.\n");
        return 1;
    }

    // Get the default system for the HMD form factor.
    XrSystemGetInfo systemGetInfo = {};
    systemGetInfo.type = XR_TYPE_SYSTEM_GET_INFO;
    systemGetInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

    XrSystemId systemId;
    if (xrGetSystem(program.instance, &systemGetInfo, &systemId) != XR_SUCCESS) {
        fprintf(stderr, "Failed to get system for HMD form factor.\n");
        return 1;
    }

    // Print information about the system.
    XrSystemProperties systemProperties = {};
    systemProperties.type = XR_TYPE_SYSTEM_PROPERTIES;

    xrGetSystemProperties(program.instance, systemId, &systemProperties);

    printf("Evaluating system\n");
    printf("\t           name: '%s'\n", systemProperties.systemName);
    printf("\t       vendorId: 0x%" PRIx32 "\n", systemProperties.vendorId);
    printf("\t       systemId: 0x%" PRIx64 "\n", systemProperties.systemId);
    printf("\t     systemName: %s\n", systemProperties.systemName);

    // The program struct will do cleanup for us.
    return 0;
}
