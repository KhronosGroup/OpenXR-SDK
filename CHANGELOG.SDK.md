# Changelog for OpenXR-SDK-Source and OpenXR-SDK Repo

Update log for the OpenXR-SDK-Source and OpenXR-SDK repo on GitHub. Updates are
in reverse chronological order starting with the latest public release.

Note that only changes relating to the loader and some of the build changes will
affect the OpenXR-SDK repository.

This summarizes the periodic public updates, not individual commits. Updates
on GitHub are generally done as single large patches at the release point,
collecting together the resolution of many Khronos internal issues,
along with any public pull requests that have been accepted.
In this repository in particular, since it is primarily software,
pull requests may be integrated as they are accepted even between periodic updates.

## OpenXR 1.0.6 release (24-January-2020)

Patch release for the 1.0 series.

This release contains, among other things, a substantial simplification and
cleanup of the loader, which should fix a number of issues and also make it
forward compatible with extensions newer than the loader itself. As a part of
this change, the loader itself now only supports a single `XrInstance` active at
a time per process. If you attempt to create a new instance while an existing
one remains (such as in the case of application code leaking an `XrInstance`
handle), the loader will now return `XR_ERROR_LIMIT_REACHED`.

### GitHub Pull Requests

These had been integrated into the public repo incrementally.

- hello_xr
  - Initialize hand_scale to 1.0
    <https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/157>
  - Fix Vulkan CHECK_CBSTATE build under newer MSVC
    <https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/154>
  - Initialize hand_scale to 1.0 to still show controller cubes even if
    grabAction not available on startup.
    <https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/157>
- Loader
  - Single instance loader refactor with forward compatibility
    <https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/146> (and internal
    MRs 1599, 1621)
  - Fix bug in loading API layers that could result in not loading an available
    and enabled layer
    <https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/155>
- Build
  - Clean up linking, build loader and layers with all available
    platform/presentation support, fix pkg-config file, rename `runtime_list`
    test executable to `openxr_runtime_list`
    <https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/149>

### Internal issues

- Registry
  - Fix typo in visibility mesh enum comment.
  - Add `XR_EXT_win32_appcontainer_compatible` extension.
- Scripts
  - Fix comment typos.
  - Sync scripts with Vulkan. (internal MR 1625)
- Loader
  - Allow use of `/` in paths in FileSysUtils on Windows.
- Build
  - Improve messages
- hello_xr
  - Add D3D12 graphics plugin (internal MR 1616)
  - Fix comment typo.

## OpenXR 1.0.5 release (6-December-2019)

Patch release for the 1.0 series.

This release primarily contains extension reservations and small specification
clarifications/fixes.

### GitHub Pull Requests

These had been integrated into the public repo incrementally.

- Loader tests
  - #147 - Small bugfix and output extension

### Internal issues

- Registry
  - Reserve Microsoft extension numbers (Internal MR 1613)

## OpenXR 1.0.4 release (21-November-2019)

Patch release for the 1.0 series.

This release includes some fixes, extensions, and a small build system change:
the build system is now configured to use C++14. No code changes in the loader
or layers have yet taken place that require C++14. **Please file an issue** in
OpenXR-SDK-Source if there is some deployment platform where you would be unable
to use a loader making use of C++14 features.

### GitHub Pull Requests

These had been integrated into the public repo incrementally.

- General, Build, Other
  - #141 - Support system libs better (permit system jsoncpp, etc. for easier
    packaging)
- hello_xr
  - #144 - Fix hello_xr when running under Linux OpenGL X11
- Registry
  - Reserve a Monado EGL extension
    <https://github.com/KhronosGroup/OpenXR-Docs/pull/39>

### Internal issues

- General, Build, Other
  - Switch C++ standard version to C++14 (internal MR 1602)
  - Remove unused/unneeded files (internal MR 1609)
- Loader
  - Fix typo in parameter/member names (internal MR 1607, internal issue 1233)
  - Fix deprecated usage of JsonCpp (internal MR 1604, internal issue 1212)
- hello_xr
  - Resolve misleading use of `xrLocateViews` before `xrWaitFrame` in helloXR
    and spec (internal MR 1584, internal issue 1227, public issue
    <https://github.com/KhronosGroup/OpenXR-SDK-Source/issues/134>)
- Registry
  - Add `XR_EXT_conformance_automation` extension, for use **only** by
    conformance testing (internal MR 1577, 1608)

## OpenXR 1.0.3 release (7-October-2019)

Patch release for the 1.0 series.

Note that this release includes changes to adjust the symbol exports from
dynamic library versions of the loader to align with the specification. Only
**core** symbols are currently exported. All extension symbols must be retrieved
using `xrGetInstanceProcAddr`.

### GitHub Pull Requests

These had been integrated into the public repo incrementally.

- General, Build, Other
  - #139 - Write output atomically at the end of generator scripts
  - #119 - Loader test updates.
  - #116 - Static analysis cleanups.
- Loader
  - #140 - Permit broader valid usage re: layers
  - #133 - Remove shwapi dependency
  - #132 - Fix directory searching for layers
  - #130 - Fix exporting of symbols on Windows.
  - #129 - Remove debug ext only when added by loader - fixes usage of debug ext
    on runtimes that do not provide it themselves.
  - #125 - Include a `OutputDebugString` logger for Win32
- Layers
  - #138 - Don't validate output enum buffer values
  - #137 - Fix incorrect filenames in the generated API layer JSON

### Internal issues

- General, Build, Other
  - Fix warnings in MSVC static code analysis mode (internal MR 1574)
  - Validation layer improvements and fixes (internal MR 1568)
  - Update vendored jsoncpp to 1.9.1 (internal MR 1523)
- Loader
  - Add ability to quiet the loader's default output (internal MR 1576)
  - Fix conformance of loader in
    `xrEnumerateApiLayerProperties`/`xrEnumerateInstanceExtensionProperties`
- hello_xr
  - Simplify action usage in hello_xr (internal MR 1553)
- Registry
  - Add `XR_EXT_view_configuration_depth_range` extension (internal MR 1502,
    internal issue 1201)
  - Reserve a Monado extension (internal MR 1541)

## OpenXR 1.0.2 release (27-August-2019)

Patch release for the 1.0 series.

Note that the loader on Windows has a **security fix**: All developers incorporating
the OpenXR loader should update immediately.

### GitHub Pull Requests

These had been integrated into the public repo incrementally.

- General, Build, Other
  - #112 - Update active runtime search documentation
  - #106 - List app changes
  - #114 - Support for building WindowsStore loader and layers, and simplified
    filename
  - #96 - Misc cleanup: build simplification, install hello_xr,
    allow building as subproject, fix null deref in validation layer.
- Loader
  - #102 - Default to catching exceptions, since not being able to catch
    (and having a non-throwing standard library) is less common
  - #109 - Factor out some debug-utils related code from the loader,
    and migrate validation layer to that shared code.
  - #108 - Update json_stream initialization to improve compatibility
  - #118 - Fix logic error in Linux active runtime search
  - #115, #117 - Simplification and refactoring.
- Layers
  - #111 - Some fixes to Validation Layer (as found applying to the UE4 OpenXR
    plugin)
  - #110 - Fix cleaning up session labels in validation layer
- From OpenXR-Docs:
  - #26 - Proposal for unbounded space and spatial anchor extensions (vendor
    extensions)

### Internal issues

- General, Build, Other
  - Allow project to be included in a parent project. (Internal MR 1512)
- hello_xr
  - Fix OpenGL version number to be XrVersion. (Internal MR 1515)
  - Make D3D11 debug device handling more friendly. (Internal MR 1504)
- Registry
  - Fix error in extension-added function. (Internal MR 1510)
  - Add Oculus Android extension. (Internal MR 1518)
  - Reserve additional extension number for Oculus. (Internal MR 1517)
- Loader
  - **Security fix**: Do not use HKEY_CURRENT_USER or environment variables when
    the process is running higher than medium-integrity on Windows.
    (Internal issue 1205, internal MR 1511)
  - Small updates to the loader documentation.

### New extension

- `XR_OCULUS_android_session_state_enable`

## OpenXR 1.0.1 release (2-August-2019)

Patch release for the 1.0 series.

### GitHub Pull Requests

These had been integrated into the public repo incrementally.

- General, Build, Other
  - #87 - Fix makefiles
  - #88 - Remove unneeded generation (corresponds to issue #74, internal issue
    1139, internal MR 1491)
  - #101 - Fix install of header and loader.
- Loader
  - #91 - Fix a loader bug which prevented Layers from not implementing all XR
    functions
  - #95 - Guard config includes/defines (relates to #81, #92)
  - #97 - Remove a constant static std::vector, use a std::array instead.
- Layers
  - #84 - Fix Linux warning for apidump
- From OpenXR-Docs:
  - #26 - Proposal for unbounded space and spatial anchor extensions (vendor
    extensions)

### Internal issues

- General, Build, Other
  - Makefile cleanups (internal MR 1469, 1489)
  - Add release scripts (internal MR 1496)
- Registry
  - Reserve Oculus extension numbers (internal MR 1493)
  - Add Monado headless (vendor extension) (internal MR 1482)
- Loader
  - Remove unnecessary `#ifdef _WIN32` in loader. (internal MR 1487)

### New extensions

- `XR_MND_headless`
- `XR_MSFT_spatial_anchor`
- `XR_MSFT_unbounded_reference_space`

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
  - #55, #58, #68 - Purge std::map bracket operations that might do inadvertent
    insertions
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
Mostly an update for tooling, layers, loader, and sample code. Header version
has been bumped to 43, but no symbols that should have actually been in use have
changed.

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
  - #14 - Initialize XrExtensionProperties array parameter for
    rt_xrEnumerateInstanceExtensionProperties
  - #20 - Fix Linux manifest file search
  - #30 - Add default implementations of API functions to dispatch chains
  - #32 - Avoid crash when evaluating layer disable environment vars
  - #35 - Add 'unknown' strings to loader's *ToString fallback functions
  - #36 - Allow null instance in xrGetInstanceProcAddr() for certain entry
    points
  - #39 - Default to static loader only on Windows

### Internal Issues

- General, Build, Other
  - Unify (for the most part) the OpenXR and Vulkan generator scripts. (internal
    MR 1166)
  - List instance extensions in the "list" test. (internal MR 1169)
  - Avoid dllexport for all apps compiled with `openxr_platform_defines.h`
    (internal MR 1187)
  - Don't offer `BUILD_SPECIFICATION` unless the spec makefile is there.
    (internal MR 1179)
  - Add simple input example to hello_xr. (internal MR 1178)
  - Add a clang-format script for ease of development.
- API Registry and Headers
  - Remove impossible and undocumented error codes. (internal MR 1185 and 1189)
  - Mark layers in `XrFrameEndInfo` as optional. (internal MR 1151, internal
    issue 899)
  - Remove unused windows types from `openxr_platform.h` (internal MR 1197)
  - Make `openxr_platform.h` include `openxr.h` on which it depends. (internal
    MR 1140, internal issue 918)
  - Remove unused, undocumented defines. (internal MR 1238, internal issue 1012)
- Loader
  - Fix loader regkey search logic so 64bit application loads 64bit regkey
    value. (internal MR 1180)
  - Modify loader to be friendly to UWP (Universal Windows Platform) build
    target. (internal MR 1198)

## OpenXR 0.90.0 - Initial public provisional release at GDC
