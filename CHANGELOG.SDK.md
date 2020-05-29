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

## OpenXR SDK 1.0.9 (2020-05-29)

- Registry
  - Add an author ID, and reserve a vendor extension for Huawei.
    ([OpenXR-Docs/#46](https://github.com/KhronosGroup/OpenXR-Docs/pull/46))
  - Reserve vendor extensions for future LunarG overlay and input focus
    functionality.
    ([internal MR 1720](https://gitlab.khronos.org/openxr/openxr/merge_requests/1720))
  - Reserve vendor extensions for Microsoft.
    ([internal MR 1723](https://gitlab.khronos.org/openxr/openxr/merge_requests/1723))
  - Add `XR_EXT_hand_tracking` multi-vendor extension.
    ([internal MR 1554](https://gitlab.khronos.org/openxr/openxr/merge_requests/1554),
    [internal issue 1266](https://gitlab.khronos.org/openxr/openxr/issues/1266),
    [internal issue 1267](https://gitlab.khronos.org/openxr/openxr/issues/1267),
    [internal issue 1268](https://gitlab.khronos.org/openxr/openxr/issues/1268),
    [internal issue 1269](https://gitlab.khronos.org/openxr/openxr/issues/1269))
  - Add `XR_HUAWEI_controller_interaction` vendor extension.
    ([OpenXR-Docs/#47](https://github.com/KhronosGroup/OpenXR-Docs/pull/47))
  - Add `XR_MNDX_egl_enable` provisional vendor extension.
    ([OpenXR-Docs/#48](https://github.com/KhronosGroup/OpenXR-Docs/pull/48))
  - Add `XR_MSFT_spatial_graph_bridge` vendor extension.
    ([internal MR 1730](https://gitlab.khronos.org/openxr/openxr/merge_requests/1730))
  - Add `XR_MSFT_secondary_view_configuration` and `XR_MSFT_first_person_observer`
    vendor extensions.
    ([internal MR 1731](https://gitlab.khronos.org/openxr/openxr/merge_requests/1731))
  - Add `XR_MSFT_hand_mesh_tracking` vendor extension.
    ([internal MR 1736](https://gitlab.khronos.org/openxr/openxr/merge_requests/1736))
  - Fix missing space in XML definition of `XrSpatialAnchorCreateInfoMSFT`.
    ([internal MR 1742](https://gitlab.khronos.org/openxr/openxr/merge_requests/1742),
    [internal issue 1351](https://gitlab.khronos.org/openxr/openxr/issues/1351),
    [OpenXR-SDK-Source/#187](https://github.com/KhronosGroup/OpenXR-SDK-Source/issues/187))
  - Update a number of contacts for author/vendor tags.
    ([internal MR 1788](https://gitlab.khronos.org/openxr/openxr/merge_requests/1788),
    [internal issue 1326](https://gitlab.khronos.org/openxr/openxr/issues/1326))
- SDK
  - Replaced usage of the `_DEBUG` macro with `NDEBUG`.
    ([internal MR 1756](https://gitlab.khronos.org/openxr/openxr/merge_requests/1756))
  - Allow disabling of `std::filesystem` usage via CMake, and detect if it's
    available and what its requirements are.
    ([OpenXR-SDK-Source/#192](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/192),
    [OpenXR-SDK-Source/#188](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/188))
  - CI: Modifications to Azure DevOps build pipeline. Now builds UWP loader DLLs in
    addition to Win32 loader DLLs. No longer builds static loader libraries due to
    linkability concerns. Re-arranged release artifact zip to distinguish
    architecture from 32-bit or 64-bit.
  - Loader: Replace global static initializers with functions that return static
    locals. With this change, code that includes OpenXR doesn't have to page in
    this code and initialize these during startup.
    ([OpenXR-SDK-Source/#173](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/173))
  - Loader: Unload runtime when `xrCreateInstance` fails.
    ([internal MR 1778](https://gitlab.khronos.org/openxr/openxr/merge_requests/1778))
  - Loader: Add "info"-level debug messages listing all the places that we look for
    the OpenXR active runtime manifest.
    ([OpenXR-SDK-Source/#190](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/190))
  - Validation Layer: Fix crash in dereferencing a nullptr optional array handle
    when the `count > 0`.
    ([internal MR 1709](https://gitlab.khronos.org/openxr/openxr/merge_requests/1709),
    [OpenXR-SDK-Source/#161](https://github.com/KhronosGroup/OpenXR-SDK-Source/issues/161),
    [internal issue 1322](https://gitlab.khronos.org/openxr/openxr/issues/1322))
  - Validation Layer: Fix static analysis error and possible loss of validation
    error.
    ([internal MR 1715](https://gitlab.khronos.org/openxr/openxr/merge_requests/1715),
    [OpenXR-SDK-Source/#160](https://github.com/KhronosGroup/OpenXR-SDK-Source/issues/160),
    [internal issue 1321](https://gitlab.khronos.org/openxr/openxr/issues/1321))
  - Validation Layer: Simplify some generated code, and minor performance
    improvements.
    ([OpenXR-SDK-Source/#176](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/176))
  - API Dump Layer: Fix crash in dereferencing a `nullptr` while constructing a
    `std::string`.
    ([internal MR 1712](https://gitlab.khronos.org/openxr/openxr/merge_requests/1712),
    [OpenXR-SDK-Source/#162](https://github.com/KhronosGroup/OpenXR-SDK-Source/issues/162),
    [internal issue 1323](https://gitlab.khronos.org/openxr/openxr/issues/1323))
  - hello_xr: Fix releasing a swapchain image with the incorrect image layout.
    ([internal MR 1755](https://gitlab.khronos.org/openxr/openxr/merge_requests/1755))
  - hello_xr: Prefer `VK_LAYER_KHRONOS_validation` over
    `VK_LAYER_LUNARG_standard_validation` when available.
    ([internal MR 1755](https://gitlab.khronos.org/openxr/openxr/merge_requests/1755))
  - hello_xr: Optimizations to D3D12 plugin to avoid GPU pipeline stall.
    ([internal MR 1770](https://gitlab.khronos.org/openxr/openxr/merge_requests/1770))
    ([OpenXR-SDK-Source/#175](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/175))
  - hello_xr: Fix build with Vulkan headers 1.2.136.
    ([OpenXR-SDK-Source/#181](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/181),
    [OpenXR-SDK-Source/#180](https://github.com/KhronosGroup/OpenXR-SDK-Source/issues/180),
    [internal issue 1347](https://gitlab.khronos.org/openxr/openxr/issues/1347))
  - hello_xr: Fix build with Visual Studio 16.6.
    ([OpenXR-SDK-Source/#186](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/186),
    [OpenXR-SDK-Source/#184](https://github.com/KhronosGroup/OpenXR-SDK-Source/issues/184))

## OpenXR SDK 1.0.8 (2020-03-27)

Patch release for the 1.0 series.

- Registry
  - `XR_EXTX_overlay`: upgrade overlay bit names to match the convention, and
    increase extension version number. This is a **source-incompatible change** to
    a provisional multi-vendor extension.
    ([internal MR 1697](https://gitlab.khronos.org/openxr/openxr/merge_requests/1697),
    [internal issue 1318](https://gitlab.khronos.org/openxr/openxr/issues/1318),
    [internal issue 42](https://gitlab.khronos.org/openxr/openxr/issues/42),
    [internal MR 171](https://gitlab.khronos.org/openxr/openxr/merge_requests/171))
  - Introduce `XR_EXT_eye_gaze_interaction` extension for eye gaze interaction
    profile.
    ([internal MR 1556](https://gitlab.khronos.org/openxr/openxr/merge_requests/1556))
  - Add SPDX license identifier tag to registry schema.
    ([internal MR 1686](https://gitlab.khronos.org/openxr/openxr/merge_requests/1686))
  - Add missing error codes to `xrCreateActionSet`, `xrCreateAction`, and
    `xrGetInputSourceLocalizedName`.
    ([internal MR 1698](https://gitlab.khronos.org/openxr/openxr/merge_requests/1698))
- SDK
  - Add SPDX license identifier tags to nearly all (code) files, including
    generated files.
    ([internal MR 1686](https://gitlab.khronos.org/openxr/openxr/merge_requests/1686))
  - Fix build system behavior with MSVC building in Release mode: only attempt
    to copy PDB files if they exist.
    ([internal MR 1701](https://gitlab.khronos.org/openxr/openxr/merge_requests/1701))

## OpenXR SDK 1.0.7 (2020-03-20)

Patch release for the 1.0 series.

Note: Changelogs are now being assembled with the help of the
[Proclamation](https://pypi.org/project/proclamation/) tool, so the format has
changed somewhat.

- Registry
  - Introduce `XR_MSFT_hand_interaction` extension for hand interaction profile.
    ([internal MR 1601](https://gitlab.khronos.org/openxr/openxr/merge_requests/1601))
  - Introduce `XR_EPIC_view_configuration_fov` extension for system field-of-view
    queries.
    ([internal MR 1170](https://gitlab.khronos.org/openxr/openxr/merge_requests/1170))
  - Indicate that `xrBeginFrame` returns `XR_ERROR_CALL_ORDER_INVALID` when not
    paired with a corresponding `xrWaitFrame` call.
    ([internal MR 1673](https://gitlab.khronos.org/openxr/openxr/merge_requests/1673))
  - Update the version number of `XR_KHR_D3D12_enable` extension.
    ([internal MR 1681](https://gitlab.khronos.org/openxr/openxr/merge_requests/1681))
  - Introduce `XR_EXTX_overlay` extension for Overlay sessions (which can
    provide overlay composition layers).
    ([internal MR 1665](https://gitlab.khronos.org/openxr/openxr/merge_requests/1665))
- SDK
  - loader: Add linker export map/version script to avoid exporting implementation
    symbols from C++ on non-MSVC platforms.
    ([internal MR 1641](https://gitlab.khronos.org/openxr/openxr/merge_requests/1641),
    [OpenXR-SDK-Source/#159](https://github.com/KhronosGroup/OpenXR-SDK-Source/issues/159))
  - Add tracking and destruction of debug messengers in the loader.
    ([internal MR 1668](https://gitlab.khronos.org/openxr/openxr/merge_requests/1668),
    [OpenXR-SDK-Source/#29](https://github.com/KhronosGroup/OpenXR-SDK-Source/issues/29),
    [internal issue 1284](https://gitlab.khronos.org/openxr/openxr/issues/1284))
  - Fix issue in `hello_xr` breaking the build in certain limited conditions.
    ([OpenXR-SDK-Source/#170](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/170))
  - Add initial (partial) Android support for `hello_xr`.
    ([internal MR 1680](https://gitlab.khronos.org/openxr/openxr/merge_requests/1680))
  - Fix a mismatched type signature, breaking compiles of hello_xr in at least some
    Linux environments.
    ([OpenXR-SDK-Source/#164](https://github.com/KhronosGroup/OpenXR-SDK-Source/issues/164),
    [internal MR 166](https://gitlab.khronos.org/openxr/openxr/merge_requests/166))
  - Explicitly link in `advapi32` for many of the APIs the loader uses on Windows,
    needed when building for ARM/ARM64 (non-UWP only).
    ([internal MR 1664](https://gitlab.khronos.org/openxr/openxr/merge_requests/1664))
  - Remove "Dev Build" string from loader resources and fix version. ([internal MR
    1664](https://gitlab.khronos.org/openxr/openxr/merge_requests/1664))
  - Add manual pages for `openxr_runtime_list` and `hello_xr` (based on their
    `--help`), and install in the standard location on non-Windows platforms.
    ([OpenXR-SDK-Source/#169](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/169))
  - Silence some noisy warnings in hello_xr and the layers.
    ([OpenXR-SDK-Source/#165](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/165))

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
