#pragma once

struct Cube {
    XrPosef Pose;
    XrVector3f Scale;
};

// Wraps a graphics API so the main openxr program can be graphics API-independent.
struct IGraphicsPlugin {
    virtual ~IGraphicsPlugin() = default;

    // OpenXR extensions required by this graphics API.
    virtual std::vector<std::string> GetInstanceExtensions() const = 0;

    // Create an instance of this graphics api for the provided instance and systemId.
    virtual void InitializeDevice(XrInstance instance, XrSystemId systemId) = 0;

    // Select the preferred swapchain format from the list of available formats.
    virtual int64_t SelectColorSwapchainFormat(const std::vector<int64_t>& runtimeFormats) const = 0;

    // Get the graphics binding header for session creation.
    virtual const XrBaseInStructure* GetGraphicsBinding() const = 0;

    // Allocate space for the swapchain image structures. These are different for each graphics API. The returned
    // pointers are valid for the lifetime of the graphics plugin.
    virtual std::vector<XrSwapchainImageBaseHeader*> AllocateSwapchainImageStructs(
        uint32_t capacity, const XrSwapchainCreateInfo& swapchainCreateInfo) = 0;

    // Render to a swapchain image for a projection view.
    virtual void RenderView(const XrCompositionLayerProjectionView& layerView, const XrSwapchainImageBaseHeader* swapchainImage,
                            int64_t swapchainFormat, const std::vector<Cube>& cubes) = 0;
};

// Create a graphics plugin for the graphics API specified in the options.
std::shared_ptr<IGraphicsPlugin> CreateGraphicsPlugin(const std::shared_ptr<struct Options>& options,
                                                      std::shared_ptr<struct IPlatformPlugin> platformPlugin);
