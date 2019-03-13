#pragma once

struct IGraphicsAdapter {
    virtual ~IGraphicsAdapter() = default;

    virtual std::vector<std::string> GetInstanceExtensions() const = 0;

    virtual const XrBaseInStructure* GetGraphicsBinding() const = 0;
};

struct IGraphicsApi {
    virtual ~IGraphicsApi() = default;

    virtual std::shared_ptr<IGraphicsAdapter> CreateAdapter(XrInstance instance, XrSystemId systemId) = 0;

    // OpenXR extensions required by this graphics API.
    virtual std::vector<std::string> GetInstanceExtensions() const = 0;
};

std::shared_ptr<IGraphicsApi> CreateGraphicsAPI(const std::shared_ptr<struct Options>& options,
                                                std::shared_ptr<struct IPlatformAdapter> platformAdapter);
