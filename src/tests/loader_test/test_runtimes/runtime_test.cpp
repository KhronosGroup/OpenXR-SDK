// Copyright (c) 2017 The Khronos Group Inc.
// Copyright (c) 2017 Valve Corporation
// Copyright (c) 2017 LunarG, Inc.
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

#include <cstring>
#include <iostream>

#include "xr_dependencies.h"
#include <openxr/openxr.h>

#include "loader_interfaces.h"

#if defined(__GNUC__) && __GNUC__ >= 4
#define RUNTIME_EXPORT __attribute__((visibility("default")))
#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x590)
#define RUNTIME_EXPORT __attribute__((visibility("default")))
#else
#define RUNTIME_EXPORT
#endif

extern "C" {

XrResult RuntimeTestXrCreateInstance(const XrInstanceCreateInfo *info, XrInstance *instance) { return XR_SUCCESS; }

XrResult RuntimeTestXrDestroyInstance(XrInstance instance) { return XR_SUCCESS; }

XrResult RuntimeTestXrEnumerateInstanceExtensionProperties(const char *layerName, uint32_t propertyCapacityInput,
                                                           uint32_t *propertyCountOutput, XrExtensionProperties *properties) {
    if (nullptr != layerName) {
        return XR_ERROR_API_LAYER_NOT_PRESENT;
    }
    // Return 2 fake extensions, just to test
    *propertyCountOutput = 2;
    if (0 != propertyCapacityInput) {
        strcpy(properties[0].extensionName, "XR_KHR_fake_ext1");
        properties[0].specVersion = 57;
        strcpy(properties[1].extensionName, "XR_KHR_fake_ext2");
        properties[1].specVersion = 3;
    }
    return XR_SUCCESS;
}

XrResult RuntimeTestXrGetInstanceProcAddr(XrInstance instance, const char *name, PFN_xrVoidFunction *function) {
    if (0 == strcmp(name, "xrGetInstanceProcAddr")) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(RuntimeTestXrGetInstanceProcAddr);
    } else if (0 == strcmp(name, "xrEnumerateInstanceExtensionProperties")) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(RuntimeTestXrEnumerateInstanceExtensionProperties);
    } else if (0 == strcmp(name, "xrCreateInstance")) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(RuntimeTestXrCreateInstance);
    } else if (0 == strcmp(name, "xrDestroyInstance")) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(RuntimeTestXrDestroyInstance);
    } else {
        *function = nullptr;
    }

    return *function ? XR_SUCCESS : XR_ERROR_FUNCTION_UNSUPPORTED;
}

// Function used to negotiate an interface betewen the loader and a runtime.
RUNTIME_EXPORT XrResult xrNegotiateLoaderRuntimeInterface(const XrNegotiateLoaderInfo *loaderInfo,
                                                          XrNegotiateRuntimeRequest *runtimeRequest) {
    if (nullptr == loaderInfo || nullptr == runtimeRequest || loaderInfo->structType != XR_LOADER_INTERFACE_STRUCT_LOADER_INFO ||
        loaderInfo->structVersion != XR_LOADER_INFO_STRUCT_VERSION || loaderInfo->structSize != sizeof(XrNegotiateLoaderInfo) ||
        runtimeRequest->structType != XR_LOADER_INTERFACE_STRUCT_RUNTIME_REQUEST ||
        runtimeRequest->structVersion != XR_RUNTIME_INFO_STRUCT_VERSION ||
        runtimeRequest->structSize != sizeof(XrNegotiateRuntimeRequest) ||
        loaderInfo->minInterfaceVersion > XR_CURRENT_LOADER_RUNTIME_VERSION ||
        loaderInfo->maxInterfaceVersion < XR_CURRENT_LOADER_RUNTIME_VERSION ||
        loaderInfo->maxInterfaceVersion > XR_CURRENT_LOADER_RUNTIME_VERSION ||
        loaderInfo->minXrVersion < XR_MAKE_VERSION(0, 1, 0) || loaderInfo->minXrVersion >= XR_MAKE_VERSION(1, 1, 0)) {
        return XR_ERROR_INITIALIZATION_FAILED;
    }

    runtimeRequest->runtimeInterfaceVersion = XR_CURRENT_LOADER_RUNTIME_VERSION;
    runtimeRequest->runtimeXrVersion = XR_MAKE_VERSION(0, 1, 0);
    runtimeRequest->getInstanceProcAddr = reinterpret_cast<PFN_xrGetInstanceProcAddr>(RuntimeTestXrGetInstanceProcAddr);

    return XR_SUCCESS;
}

// Always fail
RUNTIME_EXPORT XrResult TestRuntimeAlwaysFailNegotiateLoaderRuntimeInterface(const XrNegotiateLoaderInfo *loaderInfo,
                                                                             XrNegotiateRuntimeRequest *runtimeRequest) {
    return XR_ERROR_INITIALIZATION_FAILED;
}

// Pass, but return NULL for the runtime's xrGetInstanceProcAddr
RUNTIME_EXPORT XrResult TestRuntimeNullGipaNegotiateLoaderRuntimeInterface(const XrNegotiateLoaderInfo *loaderInfo,
                                                                           XrNegotiateRuntimeRequest *runtimeRequest) {
    if (nullptr == loaderInfo || nullptr == runtimeRequest || loaderInfo->structType != XR_LOADER_INTERFACE_STRUCT_LOADER_INFO ||
        loaderInfo->structVersion != XR_LOADER_INFO_STRUCT_VERSION || loaderInfo->structSize != sizeof(XrNegotiateLoaderInfo) ||
        runtimeRequest->structType != XR_LOADER_INTERFACE_STRUCT_RUNTIME_REQUEST ||
        runtimeRequest->structVersion != XR_RUNTIME_INFO_STRUCT_VERSION ||
        runtimeRequest->structSize != sizeof(XrNegotiateRuntimeRequest) ||
        loaderInfo->minInterfaceVersion > XR_CURRENT_LOADER_RUNTIME_VERSION ||
        loaderInfo->maxInterfaceVersion < XR_CURRENT_LOADER_RUNTIME_VERSION ||
        loaderInfo->maxInterfaceVersion > XR_CURRENT_LOADER_RUNTIME_VERSION ||
        loaderInfo->minXrVersion < XR_MAKE_VERSION(0, 1, 0) || loaderInfo->minXrVersion >= XR_MAKE_VERSION(1, 1, 0)) {
        return XR_ERROR_INITIALIZATION_FAILED;
    }

    runtimeRequest->runtimeInterfaceVersion = XR_CURRENT_LOADER_RUNTIME_VERSION;
    runtimeRequest->runtimeXrVersion = XR_MAKE_VERSION(0, 1, 0);
    runtimeRequest->getInstanceProcAddr = nullptr;

    return XR_SUCCESS;
}

// Pass, but return invalid interface version
RUNTIME_EXPORT XrResult TestRuntimeInvalidInterfaceNegotiateLoaderRuntimeInterface(const XrNegotiateLoaderInfo *loaderInfo,
                                                                                   XrNegotiateRuntimeRequest *runtimeRequest) {
    if (nullptr == loaderInfo || nullptr == runtimeRequest || loaderInfo->structType != XR_LOADER_INTERFACE_STRUCT_LOADER_INFO ||
        loaderInfo->structVersion != XR_LOADER_INFO_STRUCT_VERSION || loaderInfo->structSize != sizeof(XrNegotiateLoaderInfo) ||
        runtimeRequest->structType != XR_LOADER_INTERFACE_STRUCT_RUNTIME_REQUEST ||
        runtimeRequest->structVersion != XR_RUNTIME_INFO_STRUCT_VERSION ||
        runtimeRequest->structSize != sizeof(XrNegotiateRuntimeRequest) ||
        loaderInfo->minInterfaceVersion > XR_CURRENT_LOADER_RUNTIME_VERSION ||
        loaderInfo->maxInterfaceVersion < XR_CURRENT_LOADER_RUNTIME_VERSION ||
        loaderInfo->maxInterfaceVersion > XR_CURRENT_LOADER_RUNTIME_VERSION ||
        loaderInfo->minXrVersion < XR_MAKE_VERSION(0, 1, 0) || loaderInfo->minXrVersion >= XR_MAKE_VERSION(1, 1, 0)) {
        return XR_ERROR_INITIALIZATION_FAILED;
    }

    runtimeRequest->runtimeInterfaceVersion = 0;
    runtimeRequest->runtimeXrVersion = XR_MAKE_VERSION(0, 1, 0);
    runtimeRequest->getInstanceProcAddr = reinterpret_cast<PFN_xrGetInstanceProcAddr>(RuntimeTestXrGetInstanceProcAddr);

    return XR_SUCCESS;
}

// Pass, but return invalid API version
RUNTIME_EXPORT XrResult TestRuntimeInvalidApiNegotiateLoaderRuntimeInterface(const XrNegotiateLoaderInfo *loaderInfo,
                                                                             XrNegotiateRuntimeRequest *runtimeRequest) {
    if (nullptr == loaderInfo || nullptr == runtimeRequest || loaderInfo->structType != XR_LOADER_INTERFACE_STRUCT_LOADER_INFO ||
        loaderInfo->structVersion != XR_LOADER_INFO_STRUCT_VERSION || loaderInfo->structSize != sizeof(XrNegotiateLoaderInfo) ||
        runtimeRequest->structType != XR_LOADER_INTERFACE_STRUCT_RUNTIME_REQUEST ||
        runtimeRequest->structVersion != XR_RUNTIME_INFO_STRUCT_VERSION ||
        runtimeRequest->structSize != sizeof(XrNegotiateRuntimeRequest) ||
        loaderInfo->minInterfaceVersion > XR_CURRENT_LOADER_RUNTIME_VERSION ||
        loaderInfo->maxInterfaceVersion < XR_CURRENT_LOADER_RUNTIME_VERSION ||
        loaderInfo->maxInterfaceVersion > XR_CURRENT_LOADER_RUNTIME_VERSION ||
        loaderInfo->minXrVersion < XR_MAKE_VERSION(0, 1, 0) || loaderInfo->minXrVersion >= XR_MAKE_VERSION(1, 1, 0)) {
        return XR_ERROR_INITIALIZATION_FAILED;
    }

    runtimeRequest->runtimeInterfaceVersion = XR_CURRENT_LOADER_RUNTIME_VERSION;
    runtimeRequest->runtimeXrVersion = 0;
    runtimeRequest->getInstanceProcAddr = reinterpret_cast<PFN_xrGetInstanceProcAddr>(RuntimeTestXrGetInstanceProcAddr);

    return XR_SUCCESS;
}

}  // extern "C"
