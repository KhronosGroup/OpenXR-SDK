# OpenXR API Layers

## Overview

API Layers can be written to intercept or hook OpenXR commands for various
debug and validation purposes.
One or more XR commands can be defined in your API layer.
Commands that are not defined in an API layer will be passed directly onto
the next enabled API layer, or runtime, which does implement that
command.
This directory contains several functional API layers which can be used by
an OpenXR application.
For more information on API Layers, refer to section 2.5 on the OpenXR
Specification.

<br/>

## Using API Layers

1. Build the entire tree's source from the base folder in the OpenXR tree.
2. Set the XR\_API\_LAYER\_PATH environment variable to specify where the API
layers reside.  This should be
```(openxr_base)/(build_folder)/src/api_layers``` if you build the source.
3. Specify which API layers to activate by using xrCreateInstance or the
environmental variable XR\_ENABLE\_API\_LAYERS.

For example:
```
export XR_ENABLE_API_LAYERS=XR_APILAYER_LUNARG_api_dump
```

<br/>

## API Layers In Tree

The following API layers' source appears in this tree and can be used
as needed:
* [API Dump](README_api_dump.md)
* [Core Validation](README_core_validation.md)
