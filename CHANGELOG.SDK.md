# Changelog for OpenXR-SDK-Source and OpenXR-SDK Repo

<!--
Copyright (c) 2019-2021, The Khronos Group Inc.

SPDX-License-Identifier: CC-BY-4.0
-->

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

## OpenXR SDK 1.0.20 (2021-10-04)

This release includes a proposed cross-vendor OpenXR loader for Android, Android
build system for hello_xr, and a number of new vendor extensions.

- Registry
  - Add new `XR_HTCX_vive_tracker_interaction` provisional vendor extension.
    ([internal MR 1983](https://gitlab.khronos.org/openxr/openxr/merge_requests/1983))
  - Add new `XR_VARJO_marker_tracking` vendor extension.
    ([internal MR 2129](https://gitlab.khronos.org/openxr/openxr/merge_requests/2129))
  - Add new `XR_FB_triangle_mesh` vendor extension.
    ([internal MR 2130](https://gitlab.khronos.org/openxr/openxr/merge_requests/2130))
  - Add new `XR_FB_passthrough` vendor extension.
    ([internal MR 2130](https://gitlab.khronos.org/openxr/openxr/merge_requests/2130))
  - Reserve vendor extensions for Facebook.
    ([internal MR 2131](https://gitlab.khronos.org/openxr/openxr/merge_requests/2131))
  - Reserve a vendor extension for Almalence.
    ([OpenXR-Docs PR 99](https://github.com/KhronosGroup/OpenXR-Docs/pull/99))
  - XR_FB_color_space: Fix XML markup to indicate that
    `XrSystemColorSpacePropertiesFB` is chained to `XrSystemProperties`.
    ([internal MR 2143](https://gitlab.khronos.org/openxr/openxr/merge_requests/2143))
- SDK
  - Loader specification: Describe a cross-vendor loader for use on Android.
    ([internal MR 1949](https://gitlab.khronos.org/openxr/openxr/merge_requests/1949),
    [internal issue 1425](https://gitlab.khronos.org/openxr/openxr/issues/1425))
  - hello_xr: Add Android build system, using new cross-vendor loader, and make
    some improvements/fixes.
    ([internal MR 1949](https://gitlab.khronos.org/openxr/openxr/merge_requests/1949),
    [internal issue 1425](https://gitlab.khronos.org/openxr/openxr/issues/1425))
  - loader: Implement cross-vendor loader for Android, with AAR Prefab packaging.
    ([internal MR 1949](https://gitlab.khronos.org/openxr/openxr/merge_requests/1949),
    [internal issue 1425](https://gitlab.khronos.org/openxr/openxr/issues/1425))

## OpenXR SDK 1.0.19 (2021-08-24)

This release features a number of new or updated vendor extensions, as well as
some minor cleanups and bug fixes in the SDK.

- Registry
  - Add `XR_SESSION_NOT_FOCUSED` as a possible success return code to
    `xrApplyHapticFeedback` and `xrStopHapticFeedback`.
    ([internal MR 2106](https://gitlab.khronos.org/openxr/openxr/merge_requests/2106),
    [internal issue 1270](https://gitlab.khronos.org/openxr/openxr/issues/1270))
  - Add new `XR_FB_hand_tracking_mesh` vendor extension.
    ([internal MR 2089](https://gitlab.khronos.org/openxr/openxr/merge_requests/2089))
  - Add new `XR_FB_hand_tracking_capsules` vendor extension.
    ([internal MR 2089](https://gitlab.khronos.org/openxr/openxr/merge_requests/2089))
  - Add new `XR_FB_hand_tracking_aim` vendor extension.
    ([internal MR 2089](https://gitlab.khronos.org/openxr/openxr/merge_requests/2089))
  - Add version 1 of new `XR_FB_space_warp` vendor extension.
    ([internal MR 2115](https://gitlab.khronos.org/openxr/openxr/merge_requests/2115))
  - Register new Author ID for Almalence.
    ([OpenXR-Docs PR 92](https://github.com/KhronosGroup/OpenXR-Docs/pull/92),
    [OpenXR-Docs PR 93](https://github.com/KhronosGroup/OpenXR-Docs/pull/93))
  - Update to version 2 of `XR_VALVE_analog_threshold`.
    ([internal MR 2113](https://gitlab.khronos.org/openxr/openxr/merge_requests/2113))
- SDK
  - scripts: Some typing annotations and type-related cleanup found by using type-
    aware Python editors.
    ([internal MR 2100](https://gitlab.khronos.org/openxr/openxr/merge_requests/2100))
  - `xr_linear.h`: Fix bug in `XrVector3f_Cross`
    ([internal MR 2111](https://gitlab.khronos.org/openxr/openxr/merge_requests/2111))

## OpenXR SDK 1.0.18 (2021-07-30)

This release mostly adds new extensions. It also includes some fixes to the
included layers, as well as text in the loader documentation describing how
runtimes can register themselves for manual selection. This is not used by the
loader itself and does not require any changes to the loader, but it may be
useful to developer-focused supporting software.

- Registry
  - Add ratified `XR_KHR_swapchain_usage_input_attachment_bit` Khronos extension.
    (Promotion of `XR_MND_swapchain_usage_input_attachment_bit`, which is now
    deprecated.)
    ([internal MR 2045](https://gitlab.khronos.org/openxr/openxr/merge_requests/2045))
  - Add new `XR_FB_foveation`, `XR_FB_foveation_configuration`, and
    `XR_FB_foveation_vulkan` vendor extensions.
    ([internal MR 2050](https://gitlab.khronos.org/openxr/openxr/merge_requests/2050))
  - Add additional extension dependencies to `XR_FB_swapchain_update_state`.
    ([internal MR 2072](https://gitlab.khronos.org/openxr/openxr/merge_requests/2072),
    [internal issue 1572](https://gitlab.khronos.org/openxr/openxr/issues/1572))
  - Add new `XR_FB_composition_layer_secure_content` vendor extension.
    ([internal MR 2075](https://gitlab.khronos.org/openxr/openxr/merge_requests/2075))
  - Add new `XR_FB_composition_layer_alpha_blend` vendor extension.
    ([internal MR 2078](https://gitlab.khronos.org/openxr/openxr/merge_requests/2078))
  - Add new `XR_FB_composition_layer_image_layout` vendor extension.
    ([internal MR 2090](https://gitlab.khronos.org/openxr/openxr/merge_requests/2090))
  - Add new `XR_MSFT_spatial_anchor_persistence` vendor extension.
    ([internal MR 2093](https://gitlab.khronos.org/openxr/openxr/merge_requests/2093))
  - Add some simple [Schematron](https://schematron.com) rules and a script to
    check the XML registry against them.
    ([internal MR 2103](https://gitlab.khronos.org/openxr/openxr/merge_requests/2103))
  - Register author ID and Reserve vendor extensions for Unity.
    ([internal MR 2105](https://gitlab.khronos.org/openxr/openxr/merge_requests/2105))
  - Reserve extension ID range 187-196 for LIV Inc.
    ([internal MR 2102](https://gitlab.khronos.org/openxr/openxr/merge_requests/2102))
- SDK
  - Describe how runtimes may register themselves at installation time for manual
    selection.
    ([internal MR 2081](https://gitlab.khronos.org/openxr/openxr/merge_requests/2081),
    [internal MR 2109](https://gitlab.khronos.org/openxr/openxr/merge_requests/2109),
    [internal issue 1574](https://gitlab.khronos.org/openxr/openxr/issues/1574))
  - Include sRGB in list of supported swapchain texture formats for the HelloXR
    OpenGLES plugin.
    ([internal MR 2066](https://gitlab.khronos.org/openxr/openxr/merge_requests/2066))
  - layers: Refactor generated `xrGetInstanceProcAddr` implementations to avoid
    deeply-nested `if ... else` blocks. (Some compilers have limits we were nearing
    or hitting.)
    ([internal MR 2050](https://gitlab.khronos.org/openxr/openxr/merge_requests/2050))
  - validation layer: Set default logging mode to stdout ("text") instead of none.
    ([OpenXR-SDK-Source PR 262](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/262))
  - validation layer: Fix invalid struct type error message to show the expected
    type instead of the actual type.
    ([OpenXR-SDK-Source PR 263](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/263))

## OpenXR SDK 1.0.17 (2021-06-08)

This release features an important fix to the loader for an invalid-iterator bug
introduced in 1.0.16. All developers shipping the loader are strongly encouraged
to upgrade. It also includes a variety of new vendor extensions.

- Registry
  - Add `XR_MSFT_scene_understanding` vendor extension.
    ([internal MR 2032](https://gitlab.khronos.org/openxr/openxr/merge_requests/2032))
  - Add `XR_MSFT_scene_understanding_serialization` vendor extension.
    ([internal MR 2032](https://gitlab.khronos.org/openxr/openxr/merge_requests/2032))
  - Add `XR_MSFT_composition_layer_reprojection` vendor extension.
    ([internal MR 2033](https://gitlab.khronos.org/openxr/openxr/merge_requests/2033))
  - Add `XR_OCULUS_audio_device_guid` vendor extension.
    ([internal MR 2053](https://gitlab.khronos.org/openxr/openxr/merge_requests/2053))
  - Add version 3 of `XR_FB_swapchain_update_state` vendor extension, which splits
    platform and graphics API specific structs into separate extensions.
    ([internal MR 2059](https://gitlab.khronos.org/openxr/openxr/merge_requests/2059))
  - Apply formatting to registry XML by selectively committing changes made by
    <https://github.com/rpavlik/PrettyRegistryXml>.
    ([internal MR 2070](https://gitlab.khronos.org/openxr/openxr/merge_requests/2070),
    [OpenXR-SDK-Source/#256](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/256))
  - Enforce that all `xrCreate` functions must be able to return
    `XR_ERROR_LIMIT_REACHED` and `XR_ERROR_OUT_OF_MEMORY`, and adjust lists of
    error codes accordingly.
    ([internal MR 2064](https://gitlab.khronos.org/openxr/openxr/merge_requests/2064))
  - Fix a usage of `>` without escaping as an XML entity.
    ([internal MR 2064](https://gitlab.khronos.org/openxr/openxr/merge_requests/2064))
  - Fix all cases of a success code (most often `XR_SESSION_LOSS_PENDING`)
    appearing in the `errorcodes` attribute of a command.
    ([internal MR 2064](https://gitlab.khronos.org/openxr/openxr/merge_requests/2064),
    [internal issue 1566](https://gitlab.khronos.org/openxr/openxr/issues/1566))
  - Improve comments for several enum values.
    ([internal MR 1982](https://gitlab.khronos.org/openxr/openxr/merge_requests/1982))
  - Perform some script clean-up and refactoring, including selective type
    annotation and moving the Conventions abstract base class to `spec_tools`.
    ([internal MR 2064](https://gitlab.khronos.org/openxr/openxr/merge_requests/2064))
  - Sort return codes, with some general, popular codes made to be early. Script
    `sort_codes.py` can be used to maintain this, though it mangles other XML
    formatting, so use it with care. <https://github.com/rpavlik/PrettyRegistryXml>
    can format, and eventually sort return codes (currently sort order does not
    match).
    ([internal MR 2064](https://gitlab.khronos.org/openxr/openxr/merge_requests/2064),
    [OpenXR-SDK-Source/#256](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/256))
- SDK
  - Loader: Fix iteration over explicit layer manifests.
    ([OpenXR-SDK-Source/#256](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/256))
  - validation layer: Don't try to apply `strlen` to `wchar_t`-based output
    buffers.
    ([internal MR 2053](https://gitlab.khronos.org/openxr/openxr/merge_requests/2053))

## OpenXR SDK 1.0.16 (2021-05-11)

This release contains an update to define a new error code,
`XR_ERROR_RUNTIME_UNAVAILABLE`, now returned by the loader at `xrCreateInstance`
and `xrEnumerateInstanceProperties` when it cannot find or load a runtime for
some reason. This should be more clear for developers when encountering it, as
well as helpful when troubleshooting errors hit by users. (The
previously-returned error was typically `XR_ERROR_INSTANCE_LOST`, which is
confusing when returned when trying to create an instance.) This release also
includes a new multi-vendor extension, a new vendor extension, and improved
concurrency handling in the loader, among smaller fixes.

- Registry
  - Add new `XR_ERROR_RUNTIME_UNAVAILABLE` error code, add
    `XR_ERROR_RUNTIME_UNAVAILABLE` as a supported error code to `xrCreateInstance`
    and `xrEnumerateInstanceProperties`, and remove `XR_ERROR_INSTANCE_LOST` as a
    supported error code from `xrCreateInstance`.
    ([internal MR 2024](https://gitlab.khronos.org/openxr/openxr/merge_requests/2024),
    [internal issue 1552](https://gitlab.khronos.org/openxr/openxr/issues/1552),
    [OpenXR-SDK-Source/#177](https://github.com/KhronosGroup/OpenXR-SDK-Source/issues/177))
  - Add `XR_EXT_hand_joint_motion_range` multi-vendor extension.
    ([internal MR 1995](https://gitlab.khronos.org/openxr/openxr/merge_requests/1995))
  - Add `XR_FB_swapchain_update_state` vendor extension.
    ([internal MR 1997](https://gitlab.khronos.org/openxr/openxr/merge_requests/1997))
  - Fix missing `XR_ERROR_INSTANCE_LOST` return codes for extension functions in
    `XR_EXT_performance_settings`, `XR_EXT_debug_utils`,
    `XR_EXT_conformance_automation`, and `XR_EXT_thermal_query`.
    ([internal MR 2023](https://gitlab.khronos.org/openxr/openxr/merge_requests/2023),
    [OpenXR-Docs/#10](https://github.com/KhronosGroup/OpenXR-Docs/issues/10),
    [internal issue 1256](https://gitlab.khronos.org/openxr/openxr/issues/1256))
  - Reserve extension 166 for working group use.
    ([internal MR 2025](https://gitlab.khronos.org/openxr/openxr/merge_requests/2025))
- SDK
  - Loader: Change runtime part to return `XR_ERROR_RUNTIME_UNAVAILABLE` when
    there is an error loading a runtime.
    ([internal MR 2024](https://gitlab.khronos.org/openxr/openxr/merge_requests/2024),
    [internal issue 1552](https://gitlab.khronos.org/openxr/openxr/issues/1552),
    [OpenXR-SDK-Source/#177](https://github.com/KhronosGroup/OpenXR-SDK-Source/issues/177))
  - Loader: Simplify in areas where code paths were dead.
    ([internal MR 2024](https://gitlab.khronos.org/openxr/openxr/merge_requests/2024))
  - Loader: Improved locking around a few areas of the loader that aren't robust
    against usual concurrent calls.
    ([OpenXR-SDK-Source/#252](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/252))
  - validation layer: Fix generated code when a protected extension contains a base
    header type.
    ([internal MR 1997](https://gitlab.khronos.org/openxr/openxr/merge_requests/1997))

## OpenXR SDK 1.0.15 (2021-04-13)

The main SDK change in this release is that the OpenXR headers **no longer
expose extension function prototypes** because extension functions are not
exported by the loader. This should prevent some confusion during development
without affecting code that correctly compiles and links with older SDKs. Code
that was compiled but not linked (for instance, the automated tests of example
source in the specification) and that would not have successfully linked may
have their defects highlighted by this change, however. If you need those
prototypes still available, there is a preprocessor define that can re-enable
them. The function pointer definitions are always available.

In addition to that header change, this release contains three new vendor
extensions plus an assortment of SDK fixes.

- Registry
  - Add `XR_VARJO_foveated_rendering` vendor extension.
    ([internal MR 1981](https://gitlab.khronos.org/openxr/openxr/merge_requests/1981))
  - Add `XR_VARJO_composition_layer_depth_test` vendor extension.
    ([internal MR 1998](https://gitlab.khronos.org/openxr/openxr/merge_requests/1998))
  - Add `XR_VARJO_environment_depth_estimation` vendor extension.
    ([internal MR 1998](https://gitlab.khronos.org/openxr/openxr/merge_requests/1998))
  - Add `uint16_t` to `openxr_platform_defines` (and associated scripts) so it may
    be used easily by extensions.
    ([internal MR 2017](https://gitlab.khronos.org/openxr/openxr/merge_requests/2017))
  - Reserve extension 149 for working group use.
    ([internal MR 1999](https://gitlab.khronos.org/openxr/openxr/merge_requests/1999))
  - Reserve extension numbers 150 to 155 for ULTRALEAP extensions
    ([internal MR 2006](https://gitlab.khronos.org/openxr/openxr/merge_requests/2006))
  - Reserve extension numbers 156-165 for Facebook.
    ([internal MR 2018](https://gitlab.khronos.org/openxr/openxr/merge_requests/2018))
- SDK
  - Hide prototypes for extension functions unless explicitly requested by defining
    `XR_EXTENSION_PROTOTYPES`. These functions are not exported from the loader, so
    having their prototypes available is confusing and leads to link errors, etc.
    ([OpenXR-SDK-Source/#251](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/251),
    [OpenXR-SDK-Source/#174](https://github.com/KhronosGroup/OpenXR-SDK-Source/issues/174),
    [internal issue 1554](https://gitlab.khronos.org/openxr/openxr/issues/1554),
    [internal issue 1338](https://gitlab.khronos.org/openxr/openxr/issues/1338))
  - Also list API layers in list tool.
    ([internal MR 1991](https://gitlab.khronos.org/openxr/openxr/merge_requests/1991))
  - Ensure we expose the OpenXR headers in the build-time interface of the loader,
    as well as the install-time interface, for use with FetchContent.cmake.
    ([OpenXR-SDK-Source/#242](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/242),
    [OpenXR-SDK-Source/#195](https://github.com/KhronosGroup/OpenXR-SDK-Source/issues/195),
    [internal issue 1409](https://gitlab.khronos.org/openxr/openxr/issues/1409))
  - Improve `BUILDING.md`, including adding details on how to specify architecture
    for VS2019.
    ([OpenXR-SDK-Source/#245](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/245),
    [OpenXR-SDK-Source/#253](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/253))
  - Loader: Fix loader failing to load on Windows 7 due to `pathcch` dependency.
    ([OpenXR-SDK-Source/#239](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/239),
    [OpenXR-SDK-Source/#214](https://github.com/KhronosGroup/OpenXR-SDK-Source/issues/214),
    [internal issue 1471](https://gitlab.khronos.org/openxr/openxr/issues/1471),
    [OpenXR-SDK-Source/#236](https://github.com/KhronosGroup/OpenXR-SDK-Source/issues/236),
    [internal issue 1519](https://gitlab.khronos.org/openxr/openxr/issues/1519))
  - Loader: Fix conflicting filename in `openxr_loader.def` causing a linker warning
    when building debug for Windows.
    ([OpenXR-SDK-Source/#246](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/246))
  - Update `cgenerator.py` to generate header comments in `openxr.h` to show when a
    struct extends another struct
    ([internal MR 2005](https://gitlab.khronos.org/openxr/openxr/merge_requests/2005))
  - hello_xr: Check for `shaderStorageImageMultisample` feature in Vulkan plugin
    before using it.
    ([OpenXR-SDK-Source/#234](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/234),
    [OpenXR-SDK-Source/#233](https://github.com/KhronosGroup/OpenXR-SDK-Source/issues/233),
    [internal issue 1518](https://gitlab.khronos.org/openxr/openxr/issues/1518))
  - hello_xr: Make sure `common.h` includes the reflection header that it uses.
    ([OpenXR-SDK-Source/#247](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/247))
  - layers: Revise documentation, re-formatting and updating to refer to real
    functions and URLs.
    ([internal MR 2012](https://gitlab.khronos.org/openxr/openxr/merge_requests/2012))
  - loader: Check the instance handle passed to `xrGetInstanceProcAddr`.
    ([internal MR 1980](https://gitlab.khronos.org/openxr/openxr/merge_requests/1980))
  - loader: Fix building OpenXR-SDK with CMake's multi-config Ninja generator.
    ([OpenXR-SDK-Source/#249](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/249),
    [OpenXR-SDK-Source/#231](https://github.com/KhronosGroup/OpenXR-SDK-Source/issues/231))
  - `openxr_reflection.h`: Make reproducible/deterministic by sorting protection
    defines in the script.
    ([internal MR 1993](https://gitlab.khronos.org/openxr/openxr/merge_requests/1993),
    [internal issue 1424](https://gitlab.khronos.org/openxr/openxr/issues/1424))
  - xr_dependencies (shared utility): Include `unknwn.h` on Windows, even without
    D3D enabled.
    ([OpenXR-SDK-Source/#250](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/250),
    [OpenXR-SDK-Source/#237](https://github.com/KhronosGroup/OpenXR-SDK-Source/issues/237))

## OpenXR SDK 1.0.14 (2021-01-27)

This release contains a collection of fixes and improvements, including one new
vendor extension. Notably, we have relicensed all files that become part of the
loader, so the loader may be "Apache-2.0 OR MIT" for downstream license
compatibility.

- Registry
  - Add new `XR_FB_android_surface_swapchain_create` vendor extension.
    ([internal MR 1939](https://gitlab.khronos.org/openxr/openxr/merge_requests/1939),
    [internal issue 1493](https://gitlab.khronos.org/openxr/openxr/issues/1493),
    [internal MR 1968](https://gitlab.khronos.org/openxr/openxr/merge_requests/1968))
  - Add missing `optional` attributes to `XR_KHR_vulkan_enable2` structs. Fixes
    validation layer.
    ([OpenXR-Docs/#72](https://github.com/KhronosGroup/OpenXR-Docs/pull/72))
  - Correction to `locationFlags` field in `XrHandJointLocationEXT` to be optional.
    ([internal MR 1945](https://gitlab.khronos.org/openxr/openxr/merge_requests/1945))
  - Reserve vendor extensions for Varjo.
    ([internal MR 1935](https://gitlab.khronos.org/openxr/openxr/merge_requests/1935))
  - Reserve vendor extensions for Magic Leap.
    ([internal MR 1967](https://gitlab.khronos.org/openxr/openxr/merge_requests/1967),
    [internal MR 1970](https://gitlab.khronos.org/openxr/openxr/merge_requests/1970))
  - Reserve extension number 143 to 148 for MSFT extensions.
    ([internal MR 1969](https://gitlab.khronos.org/openxr/openxr/merge_requests/1969))
  - Update Magic Leap ID and contact information.
    ([internal MR 1967](https://gitlab.khronos.org/openxr/openxr/merge_requests/1967))
- SDK
  - Add `./` to the start of the library name in API layer manifests on Windows, so
    they are treated as a relative path.
    ([internal MR 1975](https://gitlab.khronos.org/openxr/openxr/merge_requests/1975))
  - Fix searching for prerequisites in generated CMake config files.
    ([internal MR 1963](https://gitlab.khronos.org/openxr/openxr/merge_requests/1963))
  - Start shipping the OpenXR API layers with the release artifacts.
    ([internal MR 1975](https://gitlab.khronos.org/openxr/openxr/merge_requests/1975))
  - cmake: Debug library uses d suffix on Windows. CMake `OPENXR_DEBUG_POSTFIX`
    variable can be set to something else to change it.
    ([OpenXR-SDK-Source/#229](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/229))
  - hello_xr: Remove redundant call to `xrInitializeLoaderKHR`.
    ([internal MR 1933](https://gitlab.khronos.org/openxr/openxr/merge_requests/1933))
  - hello_xr: Return supported sample count as 1 for GLES, GL and D3D11.
    ([internal MR 1962](https://gitlab.khronos.org/openxr/openxr/merge_requests/1962))
  - hello_xr: Use `android.app.NativeActivity` correctly in place of NativeActivity
    subclass.
    ([internal MR 1976](https://gitlab.khronos.org/openxr/openxr/merge_requests/1976))
  - hello_xr: On Vulkan, explicitly add surface extensions for mirror window.
    ([OpenXR-SDK-Source/#230](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/230),
    [internal MR 1934](https://gitlab.khronos.org/openxr/openxr/merge_requests/1934))
  - loader: Relicense all files that become part of the loader, so the loader may
    be "Apache-2.0 OR MIT" for downstream license compatibility.
    ([internal MR 1937](https://gitlab.khronos.org/openxr/openxr/merge_requests/1937),
    [internal issue 1449](https://gitlab.khronos.org/openxr/openxr/issues/1449),
    [OpenXR-SDK-Source/#205](https://github.com/KhronosGroup/OpenXR-SDK-Source/issues/205))
  - loader: Protect against the application overriding loader symbols.
    ([internal MR 1961](https://gitlab.khronos.org/openxr/openxr/merge_requests/1961))
  - loader: Handle JSON files in the search path that are not objects.
    ([internal MR 1979](https://gitlab.khronos.org/openxr/openxr/merge_requests/1979))

## OpenXR SDK 1.0.13 (2020-11-24)

The SDK in this release features some fixes to the loader's layer parsing:
upgrading is recommended. The hello_xr example has also been improved. The
registry for this release features a new ratified Khronos extension which will
serve as the basis of other extensions, as well as a number of new vendor
extensions.

- Registry
  - Add `XR_HTC_vive_cosmos_controller_interaction` vendor extension.
    ([internal MR 1907](https://gitlab.khronos.org/openxr/openxr/merge_requests/1907))
  - Add `XR_FB_display_refresh_rate` vendor extension.
    ([internal MR 1909](https://gitlab.khronos.org/openxr/openxr/merge_requests/1909))
  - Add `XR_MSFT_perception_anchor_interop` vendor extension.
    ([internal MR 1929](https://gitlab.khronos.org/openxr/openxr/merge_requests/1929))
  - Added ratified `KHR_binding_modifications` Khronos extension.
    ([internal MR 1878](https://gitlab.khronos.org/openxr/openxr/merge_requests/1878),
    [internal issue 1413](https://gitlab.khronos.org/openxr/openxr/issues/1413))
  - Reserve vendor extensions for HTC.
    ([internal MR 1907](https://gitlab.khronos.org/openxr/openxr/merge_requests/1907))
  - Reserve vendor extension numbers 109-120 for Facebook extensions.
    ([internal MR 1913](https://gitlab.khronos.org/openxr/openxr/merge_requests/1913))
- SDK
  - Fix build errors under mingw-w64.
    ([OpenXR-SDK-Source/#212](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/212))
  - Include PDB symbols to go along with the openxr_loader.dll Windows artifacts.
    ([OpenXR-SDK-Source/#225](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/225))
  - `XrMatrix4x4f_CreateProjection`: Explicitly define matrix values as floats.
    Prevents potential division by zero.
    ([OpenXR-SDK-Source/#219](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/219))
  - build: Normalize how we detect and utilize threading libraries in the build
    process.
    ([internal MR 1910](https://gitlab.khronos.org/openxr/openxr/merge_requests/1910))
  - build: Search for OpenGL ES and other things needed on Android.
    ([internal MR 1910](https://gitlab.khronos.org/openxr/openxr/merge_requests/1910))
  - build: Normalize how we detect and utilize Vulkan in the build process.
    ([internal MR 1910](https://gitlab.khronos.org/openxr/openxr/merge_requests/1910))
  - build/ci: Have Windows loader artifacts organize themselves by
    architecture/platform, and bundle the CMake config files and a "meta" CMake
    config.
    ([OpenXR-SDK-Source/#224](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/224),
    [OpenXR-SDK-Source/#185](https://github.com/KhronosGroup/OpenXR-SDK-Source/issues/185))
  - documentation: Make API Layer manifest example for "disable_environment" and
    "enable_environment" match the loader behavior
    ([internal MR 1917](https://gitlab.khronos.org/openxr/openxr/merge_requests/1917),
    [OpenXR-SDK-Source/#213](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/213))
  - hello_xr: Don't use subaction paths for quit_session action, it's unnecessary.
    ([internal MR 1898](https://gitlab.khronos.org/openxr/openxr/merge_requests/1898))
  - hello_xr: Add initial build system support for building for Android. (No gradle
    support yet.)
    ([internal MR 1910](https://gitlab.khronos.org/openxr/openxr/merge_requests/1910))
  - hello_xr: Call `xrInitializeLoaderKHR` and dynamically load `openxr_loader` on
    Android.
    ([internal MR 1910](https://gitlab.khronos.org/openxr/openxr/merge_requests/1910))
  - hello_xr: Fix printing of action bindings and make it prettier.
    ([internal MR 1914](https://gitlab.khronos.org/openxr/openxr/merge_requests/1914))
  - hello_xr: Fix break on Oculus Quest.
    ([internal MR 1921](https://gitlab.khronos.org/openxr/openxr/merge_requests/1921))
  - hello_xr: The D3D12 and Vulkan graphics plugins sometimes did not update their
    swapchain image context maps due to rare key collisions.
    ([OpenXR-SDK-Source/#217](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/217))
  - loader: Stub in some preliminary code for Android loader support - not a
    complete port.
    ([internal MR 1910](https://gitlab.khronos.org/openxr/openxr/merge_requests/1910))
  - loader: Add Android logcat logger.
    ([internal MR 1910](https://gitlab.khronos.org/openxr/openxr/merge_requests/1910))
  - loader: Fix parsing of XR_ENABLE_API_LAYERS environment variable
    ([internal MR 1912](https://gitlab.khronos.org/openxr/openxr/merge_requests/1912))
  - loader: Fix issues around `xrInitializeLoaderKHR`.
    ([internal MR 1922](https://gitlab.khronos.org/openxr/openxr/merge_requests/1922))
  - loader: Replace `#if _WIN32` with `#ifdef _WIN32`.
    ([OpenXR-SDK-Source/#215](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/215))

## OpenXR SDK 1.0.12 (2020-09-25)

This release features a number of new ratified KHR extensions, as well as a new
vendor extension.

- Registry
  - Add ratified `XR_KHR_vulkan_enable2` Khronos extension.
    ([internal MR 1627](https://gitlab.khronos.org/openxr/openxr/merge_requests/1627),
    [internal issue 1249](https://gitlab.khronos.org/openxr/openxr/issues/1249),
    [internal issue 1283](https://gitlab.khronos.org/openxr/openxr/issues/1283),
    [internal MR 1863](https://gitlab.khronos.org/openxr/openxr/merge_requests/1863))
  - Add ratified `XR_KHR_loader_init` Khronos extension.
    ([internal MR 1744](https://gitlab.khronos.org/openxr/openxr/merge_requests/1744))
  - Add ratified `XR_KHR_loader_init_android` Khronos extension.
    ([internal MR 1744](https://gitlab.khronos.org/openxr/openxr/merge_requests/1744))
  - Add ratified `XR_KHR_composition_layer_equirect2` Khronos extension.
    ([internal MR 1746](https://gitlab.khronos.org/openxr/openxr/merge_requests/1746))
  - Add ratified `XR_KHR_composition_layer_color_scale_bias` Khronos extension.
    ([internal MR 1762](https://gitlab.khronos.org/openxr/openxr/merge_requests/1762))
  - Add `XR_MSFT_controller_model` extension.
    ([internal MR 1832](https://gitlab.khronos.org/openxr/openxr/merge_requests/1832))
  - Add vendor tag `LIV` for LIV Inc.
    ([internal MR 1896](https://gitlab.khronos.org/openxr/openxr/merge_requests/1896))
  - Fix `structextends` attribute of `XrHandPoseTypeInfoMSFT`.
    ([OpenXR-SDK-Source/#207](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/207))
  - schema: Update to permit aliases for commands and struct types. (Already
    supported by tooling.)
    ([internal MR 1627](https://gitlab.khronos.org/openxr/openxr/merge_requests/1627))
- SDK
  - cmake: fix openxr_loader target export when installing both Release and Debug
    config on Windows.
    ([OpenXR-SDK-Source/#206](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/206))
  - hello_xr: Support the new `XR_KHR_vulkan_enable2` extension.
    ([internal MR 1627](https://gitlab.khronos.org/openxr/openxr/merge_requests/1627))
  - hello_xr: Use the `XR_KHR_loader_init_android` extension on Android.
    ([internal MR 1903](https://gitlab.khronos.org/openxr/openxr/merge_requests/1903))
  - layers: Fix ARM builds by re-adding function attributes.
    ([OpenXR-SDK-Source/#193](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/193))
- Misc
  - Clean up trailing whitespace, byte-order marks, anda ensure trailing newlines.
    ([OpenXR-SDK-Source/#208](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/208))

## OpenXR SDK 1.0.11 (2020-08-14)

This release is mainly for SDK improvements, with only small changes to the
docs. A new error code is provided for `xrCreateSession` for developers
convenience.

- Registry
  - Register `ULTRALEAP` author ID for Ultraleap.
    ([internal MR 1877](https://gitlab.khronos.org/openxr/openxr/merge_requests/1877))
  - Reserve the extension number 98 to 101 for future MSFT extensions.
    ([internal MR 1879](https://gitlab.khronos.org/openxr/openxr/merge_requests/1879))
  - schema: Distinguish `parentstruct` and `structextends` attributes in comments.
    ([internal MR 1881](https://gitlab.khronos.org/openxr/openxr/merge_requests/1881),
    [OpenXR-Docs/#51](https://github.com/KhronosGroup/OpenXR-Docs/issues/51),
    [internal issue 1396](https://gitlab.khronos.org/openxr/openxr/issues/1396))
  - Add a new result code, `XR_ERROR_GRAPHICS_REQUIREMENTS_CALL_MISSING`, for
    runtimes to return if `xrBeginSession` is called before calling one of the
    `xrGetGraphicsRequirements` calls.
    ([internal MR 1882](https://gitlab.khronos.org/openxr/openxr/merge_requests/1882),
    [OpenXR-Docs/#53](https://github.com/KhronosGroup/OpenXR-Docs/issues/53),
    [internal issue 1397](https://gitlab.khronos.org/openxr/openxr/issues/1397))
- SDK
  - Improve language usage in code and comments to be more respectful.
    ([internal MR 1881](https://gitlab.khronos.org/openxr/openxr/merge_requests/1881))
  - Loader: Correct type of "extension_version" in API layer manifest files to
    string, while maintaining backwards compatibility. Remove undocumented and
    unused "device_extensions" and "entrypoints" keys.
    ([internal MR 1867](https://gitlab.khronos.org/openxr/openxr/merge_requests/1867),
    [internal issue 1411](https://gitlab.khronos.org/openxr/openxr/issues/1411))
  - Replace usage of `std::filesystem::canonical` with `PathCchCanonicalize` on
    Windows platform to work around bug on UWP platforms. This also replaces
    `PathCanonicalize` with `PathCchCanonicalize` and adds the appropriate library
    for linking in.
    ([OpenXR-SDK-Source/#198](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/198))
  - Support for building more projects when targeting UWP, and support for all
    architectures when targeting Win32.
    ([OpenXR-SDK-Source/#199](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/199))
  - hello_xr: fix Vulkan image layout transitions.
    ([internal MR 1876](https://gitlab.khronos.org/openxr/openxr/merge_requests/1876))
  - validation: Enable three additional checks (on optional arrays with non-
    optional counts) that were missing because of a script error.
    ([internal MR 1881](https://gitlab.khronos.org/openxr/openxr/merge_requests/1881))

## OpenXR SDK 1.0.10 (2020-07-28)

Note the relicensing of the registry XML file and some include files provided by
or generated by this repository (first item in each changelog section). Each
file's header, or an adjacent file with `.license` appended to the filename, is
the best reference for its license terms. We are currently working on ensuring
all files have an SPDX license identifier tag either in them or in an adjacent
file. This is still in progress but mostly complete.

- Registry
  - Relicense registry XML from MIT-like "Khronos Free Use License for Software and
    Documentation" to, at your option, either the Apache License, Version 2.0,
    found at
    <http://www.apache.org/licenses/LICENSE-2.0>, or the MIT License, found at
    <http://opensource.org/licenses/MIT>, for broader license compatibility with
    downstream projects. (SPDX License Identifier expression "Apache-2.0 OR MIT")
    ([internal MR 1814](https://gitlab.khronos.org/openxr/openxr/merge_requests/1814),
    [OpenXR-Docs/#3](https://github.com/KhronosGroup/OpenXR-Docs/issues/3),
    [internal issue 958](https://gitlab.khronos.org/openxr/openxr/issues/958))
  - Add `XR_MSFT_holographic_window_attachment` vendor extension.
    ([internal MR 1833](https://gitlab.khronos.org/openxr/openxr/merge_requests/1833))
  - Add `XR_EXT_hp_mixed_reality_controller` multi-vendor extension.
    ([internal MR 1834](https://gitlab.khronos.org/openxr/openxr/merge_requests/1834))
  - Add `XR_EXT_samsung_odyssey_controller` multi-vendor extension.
    ([internal MR 1835](https://gitlab.khronos.org/openxr/openxr/merge_requests/1835))
  - Add `XR_VALVE_analog_threshold` vendor extension.
    ([internal MR 1859](https://gitlab.khronos.org/openxr/openxr/merge_requests/1859))
  - Add `XR_MND_swapchain_usage_input_attachment_bit` vendor extension.
    ([internal MR 1865](https://gitlab.khronos.org/openxr/openxr/merge_requests/1865))
  - Reserve extension numbers 71 to 78 for Facebook extensions.
    ([internal MR 1839](https://gitlab.khronos.org/openxr/openxr/merge_requests/1839))
  - Reserve extension numbers 79 to 88 for Valve extensions.
    ([internal MR 1842](https://gitlab.khronos.org/openxr/openxr/merge_requests/1842))
  - Reserve extension numbers 89 to 92 for Khronos extensions.
    ([internal MR 1844](https://gitlab.khronos.org/openxr/openxr/merge_requests/1844))
  - Reserve extension numbers 93 to 94 for `EXT_unbounded_reference_space` and
    `EXT_spatial_anchor`.
    ([internal MR 1854](https://gitlab.khronos.org/openxr/openxr/merge_requests/1854))
  - `XR_EPIC_view_configuration_fov`: Fix `recommendedFov` incorrectly being named
    `recommendedMutableFov`. This is a **source-incompatible change** to a vendor
    extension.
    ([internal MR 1812](https://gitlab.khronos.org/openxr/openxr/merge_requests/1812))
  - schema: Adjust to permit bitmask expansion in extensions, already supported by
    toolchain thanks to Vulkan.
    ([internal MR 1865](https://gitlab.khronos.org/openxr/openxr/merge_requests/1865))
  - scripts: Teach xml-consistency to handle bitmask values defined in extensions.
    ([internal MR 1865](https://gitlab.khronos.org/openxr/openxr/merge_requests/1865))
- SDK
  - Relicense generated headers `openxr.h`, `openxr_platform.h`,
    `openxr_reflection.h`, and static header `openxr_platform_defines.h` from the
    Apache License, version 2.0, to, at your option, either the Apache License,
    Version 2.0, found at
    <http://www.apache.org/licenses/LICENSE-2.0>, or the MIT License, found at
    <http://opensource.org/licenses/MIT>, for broader license compatibility with
    downstream projects. (SPDX License Identifier expression "Apache-2.0 OR MIT")
    ([internal MR 1814](https://gitlab.khronos.org/openxr/openxr/merge_requests/1814),
    [OpenXR-Docs/#3](https://github.com/KhronosGroup/OpenXR-Docs/issues/3),
    [internal issue 958](https://gitlab.khronos.org/openxr/openxr/issues/958))
  - Loader: Fix loading relative runtime libraries on Linux.
    ([internal MR 1817](https://gitlab.khronos.org/openxr/openxr/merge_requests/1817))
  - Loader: Fix error on xrCreateInstance when explicitly trying to enable an
    implicit API layer.
    ([internal MR 1858](https://gitlab.khronos.org/openxr/openxr/merge_requests/1858))
  - Modify Azure DevOps build pipeline to automatically generate a NuGet package.
    ([OpenXR-SDK-Source/#196](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/196))
  - Partially revert build system changes related to detecting Direct3D, to fix
    builds.
    ([internal MR 1802](https://gitlab.khronos.org/openxr/openxr/merge_requests/1802))
  - Portability fixes, including checking for `timespec_get` before enabling
    `XR_USE_TIMESPEC`.
    ([internal MR 1804](https://gitlab.khronos.org/openxr/openxr/merge_requests/1804))
  - cmake: export `OpenXRConfig.cmake` during install. Two targets can be imported
    by another CMake application: `OpenXR::openxr_loader` and `OpenXR::headers`.
    ([OpenXR-SDK-Source/#191](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/191),
    [OpenXR-SDK-Source/#185](https://github.com/KhronosGroup/OpenXR-SDK-Source/issues/185))
  - hello_xr: Fix disparity between swapchain and render pass sample count in
    Vulkan in the case where implementation recommends a value higher than one.
    ([internal MR 1794](https://gitlab.khronos.org/openxr/openxr/merge_requests/1794))
  - hello_xr: Fix build on a minimal Linux install by ensuring we check for all
    dependencies we use. We had missed checking for xcb_glx.
    ([internal MR 1799](https://gitlab.khronos.org/openxr/openxr/merge_requests/1799),
    [internal issue 1360](https://gitlab.khronos.org/openxr/openxr/issues/1360))
  - hello_xr: Fix a Vulkan crash on Windows related to the mirror window.
    ([internal MR 1823](https://gitlab.khronos.org/openxr/openxr/merge_requests/1823))
  - hello_xr: Use more proper linear formats
    ([internal MR 1840](https://gitlab.khronos.org/openxr/openxr/merge_requests/1840))
  - hello_xr: Enable use of glslangValidator to compile shaders if shaderc is not
    available.
    ([internal MR 1857](https://gitlab.khronos.org/openxr/openxr/merge_requests/1857))
  - hello_xr: Fix verbose per-layer information.
    ([internal MR 1866](https://gitlab.khronos.org/openxr/openxr/merge_requests/1866))
  - hello_xr: Add Valve Index Controller bindings. Also use trigger value instead
    of squeeze click for grab action on Vive Wand controller.
    ([OpenXR-SDK-Source/#163](https://github.com/KhronosGroup/OpenXR-SDK-Source/pull/163))
  - openxr_reflection: Add `XR_LIST_STRUCT_` expansion macros for structure types,
    as well as `XR_LIST_STRUCTURE_TYPES` macro associating types with
    `XrStructureType` values.
    ([internal MR 1495](https://gitlab.khronos.org/openxr/openxr/merge_requests/1495))
  - openxr_reflection: Adds `XR_LIST_EXTENSIONS()` macro, which will call your
    supplied macro name with the name and extension number of all known extensions.
    ([internal MR 1864](https://gitlab.khronos.org/openxr/openxr/merge_requests/1864))

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
