#include "pch.h"
#include "common.h"
#include "platformplugin.h"

#ifdef XR_USE_PLATFORM_WIN32
namespace {
struct Win32PlatformPlugin : public IPlatformPlugin {
    Win32PlatformPlugin(const std::shared_ptr<Options>&) { CHECK_HRCMD(CoInitializeEx(nullptr, COINIT_MULTITHREADED)); }

    virtual ~Win32PlatformPlugin() { CoUninitialize(); }

    std::vector<std::string> GetInstanceExtensions() const override { return {}; }

    XrBaseInStructure* GetInstanceCreateExtension() const override { return nullptr; }
};
}  // namespace

std::shared_ptr<IPlatformPlugin> CreatePlatformPlugin_Win32(const std::shared_ptr<Options>& options) {
    return std::make_shared<Win32PlatformPlugin>(options);
}
#endif
