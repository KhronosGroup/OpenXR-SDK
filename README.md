# OpenXR &reg; Software Development Kit (SDK) Project

[NOTE: This repository contains components that may eventually be assembled 
into an SDK, but are currently not being packaged into a distributable SDK.]  

This repository contains source code and build scripts for implementations 
of the OpenXR loader, validation layers, and code samples.

The authoritative public repository is located at
https://github.com/KhronosGroup/OpenXR-SDK/.
It hosts public Issue tracker, and accepts patches (Pull Requests) from the
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

Currently the best sample code is in src/tests/hello_xr/.  More will be added in the future.  

This structure is for the provisional specification.  Things are 
incomplete at launch but will be added to going forward.


## Building

See BUILDING.md

