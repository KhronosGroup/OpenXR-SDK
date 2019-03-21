# OpenXR &reg; Software Development Kit (SDK) Project

**_NOTE: This repository contains components that will eventually be assembled 
into an OpenXR SDK, but are currently not organized nor packaged as a distributable SDK._**  

This repository contains source code and build scripts for implementations 
of the OpenXR loader, validation layers, and code samples.

The authoritative public repository is located at
https://github.com/KhronosGroup/OpenXR-SDK/.
It hosts the public Issue tracker, and accepts patches (Pull Requests) from the
general public.

## Directory Structure
```
BUILDING.md           Instructions for building the projects
README.md             This file
COPYING.md            Copyright and licensing information
CODE_OF_CONDUCT.md    Code of Conduct
external/             External code for projects in the repo
include/              OpenXR platform include file
specification/        xr.xml file
src/                  Source code for various projects
src/api_layer         Sample code for developing API layers
src/loader            OpenXR loader code
src/tests/            various test code (if looking for sample code start with hello_xr/)
```

Currently the best sample code is in [src/tests/hello_xr/](https://github.com/KhronosGroup/OpenXR-SDK/tree/master/src/tests/hello_xr).  More will be added in the future.  

This structure is for the provisional specification.  Things are 
incomplete at launch but will be added to going forward.


## Building

See [BUILDING.md](https://github.com/KhronosGroup/OpenXR-SDK/blob/master/BUILDING.md)

