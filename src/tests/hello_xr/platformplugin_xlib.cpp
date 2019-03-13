#include "pch.h"
#include "common.h"
#include "platformplugin.h"

#ifdef XR_USE_PLATFORM_XLIB
namespace {
struct XlibPlatformPlugin : public IPlatformPlugin {
    XlibPlatformPlugin(const std::shared_ptr<Options>&) {}

    virtual ~XlibPlatformPlugin() {}

    std::vector<std::string> GetInstanceExtensions() const override { return {}; }

    XrBaseInStructure* GetInstanceCreateExtension() const override { return nullptr; }
};
}  // namespace

std::shared_ptr<IPlatformPlugin> CreatePlatformPlugin_Xlib(const std::shared_ptr<Options>& options) {
    return std::make_shared<XlibPlatformPlugin>(options);
}
#endif
