#include "pch.h"
#include "common.h"
#include "options.h"
#include "graphicsplugin.h"

// Graphics API factories are forward declared here.
// #ifdef XR_USE_GRAPHICS_API_OPENGL_ES
//     std::shared_ptr<IGraphicsPlugin> CreateGraphicsPlugin_OpenGLES(const std::shared_ptr<Options>& options,
//                                                              std::shared_ptr<IPlatformPlugin> platformPlugin);
// #endif
#ifdef XR_USE_GRAPHICS_API_OPENGL
std::shared_ptr<IGraphicsPlugin> CreateGraphicsPlugin_OpenGL(const std::shared_ptr<Options>& options,
                                                             std::shared_ptr<IPlatformPlugin> platformPlugin);
#endif
#ifdef XR_USE_GRAPHICS_API_VULKAN
std::shared_ptr<IGraphicsPlugin> CreateGraphicsPlugin_Vulkan(const std::shared_ptr<Options>& options,
                                                             std::shared_ptr<IPlatformPlugin> platformPlugin);
#endif
// #ifdef XR_USE_GRAPHICS_API_D3D10
//     std::shared_ptr<IGraphicsPlugin> CreateGraphicsPlugin_D3D10(const std::shared_ptr<Options>& options,
//                                                           std::shared_ptr<IPlatformPlugin> platformPlugin);
// #endif
#ifdef XR_USE_GRAPHICS_API_D3D11
std::shared_ptr<IGraphicsPlugin> CreateGraphicsPlugin_D3D11(const std::shared_ptr<Options>& options,
                                                            std::shared_ptr<IPlatformPlugin> platformPlugin);
#endif
// #ifdef XR_USE_GRAPHICS_API_D3D12
//     std::shared_ptr<IGraphicsPlugin> CreateGraphicsPlugin_D3D12(const std::shared_ptr<Options>& options,
//                                                           std::shared_ptr<IPlatformPlugin> platformPlugin);
// #endif

namespace {
using GraphicsPluginFactory = std::function<std::shared_ptr<IGraphicsPlugin>(const std::shared_ptr<Options>& options,
                                                                             std::shared_ptr<IPlatformPlugin> platformPlugin)>;

std::map<std::string, GraphicsPluginFactory, IgnoreCaseStringLess> graphicsPluginMap = {
// #ifdef XR_USE_GRAPHICS_API_OPENGL_ES
//     { "OpenGLES", [](const std::shared_ptr<Options>& options, std::shared_ptr<IPlatformPlugin> platformPlugin) {
//         return CreateGraphicsPlugin_OpenGLES(options, platformPlugin); } },
// #endif
#ifdef XR_USE_GRAPHICS_API_OPENGL
    {"OpenGL",
     [](const std::shared_ptr<Options>& options, std::shared_ptr<IPlatformPlugin> platformPlugin) {
         return CreateGraphicsPlugin_OpenGL(options, platformPlugin);
     }},
#endif
#ifdef XR_USE_GRAPHICS_API_VULKAN
    {"Vulkan",
     [](const std::shared_ptr<Options>& options, std::shared_ptr<IPlatformPlugin> platformPlugin) {
         return CreateGraphicsPlugin_Vulkan(options, platformPlugin);
     }},
#endif
// #ifdef XR_USE_GRAPHICS_API_D3D10
//     { "D3D10", [](const std::shared_ptr<Options>& options, std::shared_ptr<IPlatformPlugin> platformPlugin) {
//         return CreateGraphicsPlugin_D3D10(options, platformPlugin); } },
// #endif
#ifdef XR_USE_GRAPHICS_API_D3D11
    {"D3D11", [](const std::shared_ptr<Options>& options,
                 std::shared_ptr<IPlatformPlugin> platformPlugin) { return CreateGraphicsPlugin_D3D11(options, platformPlugin); }},
#endif
    // #ifdef XR_USE_GRAPHICS_API_D3D12
    //     { "D3D12", [](const std::shared_ptr<Options>& options, std::shared_ptr<IPlatformPlugin> platformPlugin) {
    //         return CreateGraphicsPlugin_D3D12(options, platformPlugin); } },
    // #endif
};
}  // namespace

std::shared_ptr<IGraphicsPlugin> CreateGraphicsPlugin(const std::shared_ptr<Options>& options,
                                                      std::shared_ptr<IPlatformPlugin> platformPlugin) {
    if (options->GraphicsPlugin.size() == 0) {
        throw std::invalid_argument("No graphics API specified");
    }

    const auto apiIt = graphicsPluginMap.find(options->GraphicsPlugin);
    if (apiIt == graphicsPluginMap.end()) {
        throw std::invalid_argument(Fmt("Unsupported graphics API '%s'", options->GraphicsPlugin.c_str()));
    }

    return apiIt->second(options, platformPlugin);
}
