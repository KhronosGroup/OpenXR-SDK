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
#define LAYER_EXPORT __attribute__((visibility("default")))
#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x590)
#define LAYER_EXPORT __attribute__((visibility("default")))
#else
#define LAYER_EXPORT
#endif

extern "C" {

XrResult LayerTestXrCreateInstance(const XrInstanceCreateInfo *info, XrInstance *instance) { return XR_SUCCESS; }

XrResult LayerTestXrDestroyInstance(XrInstance instance) { return XR_SUCCESS; }

XrResult LayerTestXrGetInstanceProcAddr(XrInstance instance, const char *name, PFN_xrVoidFunction *function) {
    if (0 == strcmp(name, "xrGetInstanceProcAddr")) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(LayerTestXrGetInstanceProcAddr);
    } else if (0 == strcmp(name, "xrCreateInstance")) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(LayerTestXrCreateInstance);
    } else if (0 == strcmp(name, "xrDestroyInstance")) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(LayerTestXrDestroyInstance);
    } else {
        *function = nullptr;
    }

    return *function ? XR_SUCCESS : XR_ERROR_FUNCTION_UNSUPPORTED;
}

// Function used to negotiate an interface betewen the loader and a layer.  Each library exposing one or
// more layers needs to expose at least this function.
LAYER_EXPORT XrResult xrNegotiateLoaderApiLayerInterface(const XrNegotiateLoaderInfo *loaderInfo, const char *layerName,
                                                         XrNegotiateApiLayerRequest *layerRequest) {
    if (nullptr == loaderInfo || nullptr == layerRequest || loaderInfo->structType != XR_LOADER_INTERFACE_STRUCT_LOADER_INFO ||
        loaderInfo->structVersion != XR_LOADER_INFO_STRUCT_VERSION || loaderInfo->structSize != sizeof(XrNegotiateLoaderInfo) ||
        layerRequest->structType != XR_LOADER_INTERFACE_STRUCT_API_LAYER_REQUEST ||
        layerRequest->structVersion != XR_API_LAYER_INFO_STRUCT_VERSION ||
        layerRequest->structSize != sizeof(XrNegotiateApiLayerRequest) ||
        loaderInfo->minInterfaceVersion > XR_CURRENT_LOADER_API_LAYER_VERSION ||
        loaderInfo->maxInterfaceVersion < XR_CURRENT_LOADER_API_LAYER_VERSION ||
        loaderInfo->maxInterfaceVersion > XR_CURRENT_LOADER_API_LAYER_VERSION ||
        loaderInfo->minXrVersion < XR_MAKE_VERSION(0, 1, 0) || loaderInfo->minXrVersion >= XR_MAKE_VERSION(1, 1, 0)) {
        return XR_ERROR_INITIALIZATION_FAILED;
    }

    layerRequest->layerInterfaceVersion = XR_CURRENT_LOADER_API_LAYER_VERSION;
    layerRequest->layerXrVersion = XR_MAKE_VERSION(0, 1, 0);
    layerRequest->getInstanceProcAddr = reinterpret_cast<PFN_xrGetInstanceProcAddr>(LayerTestXrGetInstanceProcAddr);

    return XR_SUCCESS;
}

// Always fail
LAYER_EXPORT XrResult TestlayerAlwaysFailNegotiateLoaderApiLayerInterface(const XrNegotiateLoaderInfo *loaderInfo,
                                                                          const char *layerName,
                                                                          XrNegotiateApiLayerRequest *layerRequest) {
    return XR_ERROR_INITIALIZATION_FAILED;
}

// Pass, but return NULL for the layer's xrGetInstanceProcAddr
LAYER_EXPORT XrResult TestLayerNullGipaNegotiateLoaderApiLayerInterface(const XrNegotiateLoaderInfo *loaderInfo,
                                                                        const char *layerName,
                                                                        XrNegotiateApiLayerRequest *layerRequest) {
    if (nullptr == loaderInfo || nullptr == layerRequest || loaderInfo->structType != XR_LOADER_INTERFACE_STRUCT_LOADER_INFO ||
        loaderInfo->structVersion != XR_LOADER_INFO_STRUCT_VERSION || loaderInfo->structSize != sizeof(XrNegotiateLoaderInfo) ||
        layerRequest->structType != XR_LOADER_INTERFACE_STRUCT_API_LAYER_REQUEST ||
        layerRequest->structVersion != XR_API_LAYER_INFO_STRUCT_VERSION ||
        layerRequest->structSize != sizeof(XrNegotiateApiLayerRequest) ||
        loaderInfo->minInterfaceVersion > XR_CURRENT_LOADER_API_LAYER_VERSION ||
        loaderInfo->maxInterfaceVersion < XR_CURRENT_LOADER_API_LAYER_VERSION ||
        loaderInfo->maxInterfaceVersion > XR_CURRENT_LOADER_API_LAYER_VERSION ||
        loaderInfo->minXrVersion < XR_MAKE_VERSION(0, 1, 0) || loaderInfo->minXrVersion >= XR_MAKE_VERSION(1, 1, 0)) {
        return XR_ERROR_INITIALIZATION_FAILED;
    }

    layerRequest->layerInterfaceVersion = XR_CURRENT_LOADER_API_LAYER_VERSION;
    layerRequest->layerXrVersion = XR_MAKE_VERSION(0, 1, 0);
    layerRequest->getInstanceProcAddr = nullptr;

    return XR_SUCCESS;
}

// Pass, but return invalid interface version
LAYER_EXPORT XrResult TestLayerInvalidInterfaceNegotiateLoaderApiLayerInterface(const XrNegotiateLoaderInfo *loaderInfo,
                                                                                const char *layerName,
                                                                                XrNegotiateApiLayerRequest *layerRequest) {
    if (nullptr == loaderInfo || nullptr == layerRequest || loaderInfo->structType != XR_LOADER_INTERFACE_STRUCT_LOADER_INFO ||
        loaderInfo->structVersion != XR_LOADER_INFO_STRUCT_VERSION || loaderInfo->structSize != sizeof(XrNegotiateLoaderInfo) ||
        layerRequest->structType != XR_LOADER_INTERFACE_STRUCT_API_LAYER_REQUEST ||
        layerRequest->structVersion != XR_API_LAYER_INFO_STRUCT_VERSION ||
        layerRequest->structSize != sizeof(XrNegotiateApiLayerRequest) ||
        loaderInfo->minInterfaceVersion > XR_CURRENT_LOADER_API_LAYER_VERSION ||
        loaderInfo->maxInterfaceVersion < XR_CURRENT_LOADER_API_LAYER_VERSION ||
        loaderInfo->maxInterfaceVersion > XR_CURRENT_LOADER_API_LAYER_VERSION ||
        loaderInfo->minXrVersion < XR_MAKE_VERSION(0, 1, 0) || loaderInfo->minXrVersion >= XR_MAKE_VERSION(1, 1, 0)) {
        return XR_ERROR_INITIALIZATION_FAILED;
    }

    layerRequest->layerInterfaceVersion = 0;
    layerRequest->layerXrVersion = XR_MAKE_VERSION(0, 1, 0);
    layerRequest->getInstanceProcAddr = reinterpret_cast<PFN_xrGetInstanceProcAddr>(LayerTestXrGetInstanceProcAddr);

    return XR_SUCCESS;
}

// Pass, but return invalid API version
LAYER_EXPORT XrResult TestLayerInvalidApiNegotiateLoaderApiLayerInterface(const XrNegotiateLoaderInfo *loaderInfo,
                                                                          const char *layerName,
                                                                          XrNegotiateApiLayerRequest *layerRequest) {
    if (nullptr == loaderInfo || nullptr == layerRequest || loaderInfo->structType != XR_LOADER_INTERFACE_STRUCT_LOADER_INFO ||
        loaderInfo->structVersion != XR_LOADER_INFO_STRUCT_VERSION || loaderInfo->structSize != sizeof(XrNegotiateLoaderInfo) ||
        layerRequest->structType != XR_LOADER_INTERFACE_STRUCT_API_LAYER_REQUEST ||
        layerRequest->structVersion != XR_API_LAYER_INFO_STRUCT_VERSION ||
        layerRequest->structSize != sizeof(XrNegotiateApiLayerRequest) ||
        loaderInfo->minInterfaceVersion > XR_CURRENT_LOADER_API_LAYER_VERSION ||
        loaderInfo->maxInterfaceVersion < XR_CURRENT_LOADER_API_LAYER_VERSION ||
        loaderInfo->maxInterfaceVersion > XR_CURRENT_LOADER_API_LAYER_VERSION ||
        loaderInfo->minXrVersion < XR_MAKE_VERSION(0, 1, 0) || loaderInfo->minXrVersion >= XR_MAKE_VERSION(1, 1, 0)) {
        return XR_ERROR_INITIALIZATION_FAILED;
    }

    layerRequest->layerInterfaceVersion = XR_CURRENT_LOADER_API_LAYER_VERSION;
    layerRequest->layerXrVersion = 0;
    layerRequest->getInstanceProcAddr = reinterpret_cast<PFN_xrGetInstanceProcAddr>(LayerTestXrGetInstanceProcAddr);

    return XR_SUCCESS;
}

}  // extern "C"
