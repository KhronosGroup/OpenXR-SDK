# Changelog for OpenXR-SDK-Source and OpenXR-SDK Repo

Update log for the OpenXR-SDK-Source and OpenXR-SDK repo on GitHub.
Updates are in reverse chronological order starting with the latest public release.

Note that only changes relating to the loader and some of the build changes will affect the OpenXR-SDK repository.

This summarizes the periodic public updates, not individual commits. Updates
on GitHub are generally done as single large patches at the release point,
collecting together the resolution of many Khronos internal issues,
along with any public pull requests that have been accepted.
In this repository in particular, since it is primarily software,
pull requests may be integrated as they are accepted even between periodic updates.

## OpenXR 1.0.0 release (29-July-2019)

Incorporates spec changes from OpenXR 1.0,
all public pull requests incorporated in the 0.90 series,
and additional fixes and improvements not previously published.

## Change log for OpenXR 0.90 provisional spec updates post-0.90.1

### GitHub Pull Requests

These had been integrated into the public repo incrementally.

- General, Build, Other
  - #40 - Update BUILDING.md with some Linux pre-requisites
  - #43 - Make manifest file more compatible
  - #44 - Remove pkg-config dependency from xlib backend
  - #46 - Support building with "embedded" Python
  - #48 - Install layers and pkg-config file on Linux
  - #66 - Install the layers libraries on Linux
  - #71 - Validation layer: fix logic error
- hello_xr
  - #49 - Fix hello_xr to properly use two call idiom
- Loader
  - #38 - Remove dead file-locking code
  - #51 - Idiomatic Linux active_runtime.json search logic
  - #55, #58, #68 - Purge std::map bracket operations that might do inadvertent insertions
  - #56 - Make `filesystem_util.cc` `#define UNICODE`-compatible
  - #57 - Make it possible to bypass macro that checks which `filesystem` to use
  - #60 - Fix build error with shlwapi function
  - #62 - Don't limit contents of `XristanceCreateInfo` next chain
  - #65 - Fix minor substr error
  - #69 - Simplify loader
  - #70, #76 - Make loader exception free
  - #72 - filesystem: fix theoretical bug on Linux
  - #73 - Loader proper UNICODE support
  - #75 - Clang tidy
  - #80 - Switchable exceptions
  - #82 - Add folder properties to all CMake targets.

## Change log for OpenXR 0.90.1 provisional spec update (8-May-2019)

No API changes, and only minimal consistency changes to the spec/registry.
Mostly an update for tooling, layers, loader, and sample code.
Header version has been bumped to 43, but no symbols that should have actually been in use have changed.

### GitHub Pull Requests

These had been integrated into the public repo incrementally.

- General, Build, Other
  - #8, #11, #12 - Improve BUILDING and README
  - #9 - Make Vulkan SDK dependency optional
  - #17 - Add install target to CMake files
  - #17 - API dump layer, build: timespec extension fixes
  - #19 - build: fix CMAKE_PRESENTATION_BACKEND default on linux
  - #34 - list: Fix list test output
- validation layer
  - #18, #22, #23 - Fix build and execution
  - #24 - Fix crash and refactor
- hello_xr
  - #13 - Do not query GL context API version before creating context
  - #26 - Fix a warning
- Loader
  - #3 - Don't cross 32/64 registry silos
  - #14 - Initialize XrExtensionProperties array parameter for rt_xrEnumerateInstanceExtensionProperties
  - #20 - Fix Linux manifest file search
  - #30 - Add default implementations of API functions to dispatch chains
  - #32 - Avoid crash when evaluating layer disable environment vars
  - #35 - Add 'unknown' strings to loader's *ToString fallback functions
  - #36 - Allow null instance in xrGetInstanceProcAddr() for certain entry points
  - #39 - Default to static loader only on Windows

### Internal Issues

- General, Build, Other
  - Unify (for the most part) the OpenXR and Vulkan generator scripts. (internal MR 1166)
  - List instance extensions in the "list" test. (internal MR 1169)
  - Avoid dllexport for all apps compiled with `openxr_platform_defines.h` (internal MR 1187)
  - Don't offer `BUILD_SPECIFICATION` unless the spec makefile is there. (internal MR 1179)
  - Add simple input example to hello_xr. (internal MR 1178)
  - Add a clang-format script for ease of development.
- API Registry and Headers
  - Remove impossible and undocumented error codes. (internal MR 1185 and 1189)
  - Mark layers in `XrFrameEndInfo` as optional. (internal MR 1151, internal issue 899)
  - Remove unused windows types from `openxr_platform.h` (internal MR 1197)
  - Make `openxr_platform.h` include `openxr.h` on which it depends. (internal MR 1140, internal issue 918)
  - Remove unused, undocumented defines. (internal MR 1238, internal issue 1012)
- Loader
  - Fix loader regkey search logic so 64bit application loads 64bit regkey value. (internal MR 1180)
  - Modify loader to be friendly to UWP (Universal Windows Platform) build target. (internal MR 1198)

## OpenXR 0.90.0 - Initial public provisional release at GDC
