#include "pch.h"
#include "common.h"
#include "geometry.h"
#include "graphicsplugin.h"

#ifdef XR_USE_GRAPHICS_API_VULKAN

#include <common/xr_linear.h>
#include <array>

#ifdef USE_ONLINE_VULKAN_SHADERC
#include <shaderc/shaderc.hpp>
#endif

#if defined(VK_USE_PLATFORM_WIN32_KHR)
// Define USE_MIRROR_WINDOW to open a dummy window for e.g. RenderDoc
#define USE_MIRROR_WINDOW
#endif

namespace {

static std::string vkResultString(VkResult res) {
    switch (res) {
        case VK_SUCCESS:
            return "SUCCESS";
        case VK_NOT_READY:
            return "NOT_READY";
        case VK_TIMEOUT:
            return "TIMEOUT";
        case VK_EVENT_SET:
            return "EVENT_SET";
        case VK_EVENT_RESET:
            return "EVENT_RESET";
        case VK_INCOMPLETE:
            return "INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return "ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return "ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED:
            return "ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST:
            return "ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED:
            return "ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT:
            return "ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            return "ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT:
            return "ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            return "ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS:
            return "ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            return "ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_SURFACE_LOST_KHR:
            return "ERROR_SURFACE_LOST_KHR";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            return "ERROR_NATIVE_WINDOW_IN_USE_KHR";
        case VK_SUBOPTIMAL_KHR:
            return "SUBOPTIMAL_KHR";
        case VK_ERROR_OUT_OF_DATE_KHR:
            return "ERROR_OUT_OF_DATE_KHR";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
            return "ERROR_INCOMPATIBLE_DISPLAY_KHR";
        case VK_ERROR_VALIDATION_FAILED_EXT:
            return "ERROR_VALIDATION_FAILED_EXT";
        case VK_ERROR_INVALID_SHADER_NV:
            return "ERROR_INVALID_SHADER_NV";
        default:
            return std::to_string(res);
    }
}

[[noreturn]] inline void ThrowVkResult(VkResult res, const char* originator = nullptr, const char* sourceLocation = nullptr) {
    Throw(Fmt("VkResult failure [%s]", vkResultString(res).c_str()), originator, sourceLocation);
}

inline VkResult CheckVkResult(VkResult res, const char* originator = nullptr, const char* sourceLocation = nullptr) {
    if ((res) < VK_SUCCESS) {
        ThrowVkResult(res, originator, sourceLocation);
    }

    return res;
}

// XXX These really shouldn't have trailing ';'s
#define THROW_VK(res, cmd) ThrowVkResult(res, #cmd, FILE_AND_LINE);
#define CHECK_VKCMD(cmd) CheckVkResult(cmd, #cmd, FILE_AND_LINE);
#define CHECK_VKRESULT(res, cmdStr) CheckVkResult(res, cmdStr, FILE_AND_LINE);

#ifdef USE_ONLINE_VULKAN_SHADERC
constexpr char VertexShaderGlsl[] =
    R"_(
    #version 430
    #extension GL_ARB_separate_shader_objects : enable

    layout (std140, push_constant) uniform buf
    {
        mat4 mvp;
    } ubuf;

    layout (location = 0) in vec3 Position;
    layout (location = 1) in vec3 Color;

    layout (location = 0) out vec4 oColor;
    out gl_PerVertex
    {
        vec4 gl_Position;
    };

    void main()
    {
        oColor.rgba  = Color.rgba;
        gl_Position = ubuf.mvp * Position;
    }
)_";

constexpr char FragmentShaderGlsl[] =
    R"_(
    #version 430
    #extension GL_ARB_separate_shader_objects : enable

    layout (location = 0) in vec4 oColor;

    layout (location = 0) out vec4 FragColor;

    void main()
    {
        FragColor = oColor;
    }
)_";
#endif  // USE_ONLINE_VULKAN_SHADERC

struct MemoryAllocator {
    void Init(VkPhysicalDevice physicalDevice, VkDevice device) {
        m_vkDevice = device;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &m_memProps);
    }

    static const VkFlags defaultFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    void Allocate(VkMemoryRequirements const& memReqs, VkDeviceMemory* mem, VkFlags flags = defaultFlags,
                  const void* pNext = nullptr) const {
        // Search memtypes to find first index with those properties
        for (uint32_t i = 0; i < m_memProps.memoryTypeCount; ++i) {
            if (memReqs.memoryTypeBits & (1 << i)) {
                // Type is available, does it match user properties?
                if ((m_memProps.memoryTypes[i].propertyFlags & flags) == flags) {
                    VkMemoryAllocateInfo memAlloc{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, pNext};
                    memAlloc.allocationSize = memReqs.size;
                    memAlloc.memoryTypeIndex = i;
                    CHECK_VKCMD(vkAllocateMemory(m_vkDevice, &memAlloc, nullptr, mem));
                    return;
                }
            }
        }
        THROW("Memory format not supported");
    }

   private:
    VkDevice m_vkDevice{VK_NULL_HANDLE};
    VkPhysicalDeviceMemoryProperties m_memProps{};
};

// CmdBuffer - manage VkCommandBuffer state
struct CmdBuffer {
#define LIST_CMDBUFFER_STATES(_) \
    _(Undefined)                 \
    _(Initialized)               \
    _(Recording)                 \
    _(Executable)                \
    _(Executing)
    enum class CmdBufferState {
#define MK_ENUM(name) name,
        LIST_CMDBUFFER_STATES(MK_ENUM)
#undef MK_ENUM
    };
    CmdBufferState state{CmdBufferState::Undefined};
    VkCommandPool pool{VK_NULL_HANDLE};
    VkCommandBuffer buf{VK_NULL_HANDLE};
    VkFence execFence{VK_NULL_HANDLE};

    CmdBuffer() {}

    CmdBuffer(const CmdBuffer& that) = delete;

    ~CmdBuffer() {
        SetState(CmdBufferState::Undefined);
        if (m_vkDevice) {
            if (buf) vkFreeCommandBuffers(m_vkDevice, pool, 1, &buf);
            if (pool) vkDestroyCommandPool(m_vkDevice, pool, nullptr);
            if (execFence) vkDestroyFence(m_vkDevice, execFence, nullptr);
        }
        buf = nullptr;
        pool = VK_NULL_HANDLE;
        execFence = VK_NULL_HANDLE;
    }

    std::string StateString(CmdBufferState s) {
        switch (s) {
#define MK_CASE(name)          \
    case CmdBufferState::name: \
        return #name;
            LIST_CMDBUFFER_STATES(MK_CASE)
#undef MK_CASE
        }
        return "(Unknown)";
    }

#define CHECK_CBSTATE(s)                                                                                                           \
    do                                                                                                                             \
        if (state != (s)) {                                                                                                        \
            (std::string(__FILE__) + "(" + std::to_string(__LINE__) + "): Expecting state " #s " from " + __FUNCTION__ + ", in " + \
             StateString(state));                                                                                                  \
            return false;                                                                                                          \
        }                                                                                                                          \
    while (0)

    bool Init(VkDevice device, uint32_t queueFamilyIndex) {
        CHECK_CBSTATE(CmdBufferState::Undefined);

        m_vkDevice = device;

        // Create a command pool to allocate our command buffer from
        VkCommandPoolCreateInfo cmdPoolInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        cmdPoolInfo.queueFamilyIndex = queueFamilyIndex;
        CHECK_VKCMD(vkCreateCommandPool(m_vkDevice, &cmdPoolInfo, nullptr, &pool));

        // Create the command buffer from the command pool
        VkCommandBufferAllocateInfo cmd{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        cmd.commandPool = pool;
        cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmd.commandBufferCount = 1;
        CHECK_VKCMD(vkAllocateCommandBuffers(m_vkDevice, &cmd, &buf));

        VkFenceCreateInfo fenceInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        CHECK_VKCMD(vkCreateFence(m_vkDevice, &fenceInfo, nullptr, &execFence));

        SetState(CmdBufferState::Initialized);
        return true;
    }

    bool Begin() {
        CHECK_CBSTATE(CmdBufferState::Initialized);
        VkCommandBufferBeginInfo cmdBeginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        CHECK_VKCMD(vkBeginCommandBuffer(buf, &cmdBeginInfo));
        SetState(CmdBufferState::Recording);
        return true;
    }

    bool End() {
        CHECK_CBSTATE(CmdBufferState::Recording);
        CHECK_VKCMD(vkEndCommandBuffer(buf));
        SetState(CmdBufferState::Executable);
        return true;
    }

    bool Exec(VkQueue queue) {
        CHECK_CBSTATE(CmdBufferState::Executable);

        VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &buf;
        CHECK_VKCMD(vkQueueSubmit(queue, 1, &submitInfo, execFence));

        SetState(CmdBufferState::Executing);
        return true;
    }

    bool Wait() {
        // Waiting on a not-in-flight command buffer is a no-op
        if (state == CmdBufferState::Initialized) return true;

        CHECK_CBSTATE(CmdBufferState::Executing);

        const uint32_t timeoutNs = 1 * 1000 * 1000 * 1000;
        for (int i = 0; i < 5; ++i) {
            auto res = vkWaitForFences(m_vkDevice, 1, &execFence, VK_TRUE, timeoutNs);
            if (res == VK_SUCCESS) {
                // Buffer can be executed multiple times...
                SetState(CmdBufferState::Executable);
                return true;
            }
            Log::Write(Log::Level::Info, "Waiting for CmdBuffer fence timed out, retrying...");
        }

        return false;
    }

    bool Reset() {
        if (state != CmdBufferState::Initialized) {
            CHECK_CBSTATE(CmdBufferState::Executable);

            CHECK_VKCMD(vkResetFences(m_vkDevice, 1, &execFence));
            CHECK_VKCMD(vkResetCommandBuffer(buf, 0));

            SetState(CmdBufferState::Initialized);
        }

        return true;
    }

   private:
    VkDevice m_vkDevice{VK_NULL_HANDLE};

    void SetState(CmdBufferState newState) { state = newState; }

#undef CHECK_CBSTATE
#undef LIST_CMDBUFFER_STATES
};

// ShaderProgram to hold a pair of vertex & fragment shaders
struct ShaderProgram {
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderInfo{
        {{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO}, {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO}}};

    ShaderProgram() {}

    ~ShaderProgram() {
        if (m_vkDevice) {
            for (auto& si : shaderInfo) {
                if (si.module) vkDestroyShaderModule(m_vkDevice, shaderInfo[0].module, nullptr);
                si.module = VK_NULL_HANDLE;
            }
        }
        shaderInfo = {};
    }

    void LoadVertexShader(const std::vector<uint32_t>& code) { Load(0, code); }

    void LoadFragmentShader(const std::vector<uint32_t>& code) { Load(1, code); }

    void Init(VkDevice device) { m_vkDevice = device; }

   private:
    VkDevice m_vkDevice{VK_NULL_HANDLE};

    const void Load(uint32_t index, const std::vector<uint32_t>& code) {
        VkShaderModuleCreateInfo modInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};

        auto& si = shaderInfo[index];
        si.pName = "main";
        std::string name;

        switch (index) {
            case 0:
                si.stage = VK_SHADER_STAGE_VERTEX_BIT;
                name = "vertex";
                break;
            case 1:
                si.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                name = "fragment";
                break;
            default:
                THROW(Fmt("Unknown code index %d", index));
        }

        modInfo.codeSize = code.size() * sizeof(code[0]);
        modInfo.pCode = &code[0];
        CHECK_MSG((modInfo.codeSize > 0) && modInfo.pCode, Fmt("Invalid %s shader ", name.c_str()));

        CHECK_VKCMD(vkCreateShaderModule(m_vkDevice, &modInfo, nullptr, &si.module));

        Log::Write(Log::Level::Info, Fmt("Loaded %s shader", name.c_str()));
    }
};

// VertexBuffer base class
struct VertexBufferBase {
    VkBuffer idxBuf{VK_NULL_HANDLE};
    VkDeviceMemory idxMem{VK_NULL_HANDLE};
    VkBuffer vtxBuf{VK_NULL_HANDLE};
    VkDeviceMemory vtxMem{VK_NULL_HANDLE};
    VkVertexInputBindingDescription bindDesc{};
    std::vector<VkVertexInputAttributeDescription> attrDesc{};
    struct {
        uint32_t idx;
        uint32_t vtx;
    } count = {0, 0};

    VertexBufferBase() {}

    ~VertexBufferBase() {
        if (m_vkDevice) {
            if (idxBuf) vkDestroyBuffer(m_vkDevice, idxBuf, nullptr);
            if (idxMem) vkFreeMemory(m_vkDevice, idxMem, nullptr);
            if (vtxBuf) vkDestroyBuffer(m_vkDevice, vtxBuf, nullptr);
            if (vtxMem) vkFreeMemory(m_vkDevice, vtxMem, nullptr);
        }
        idxBuf = VK_NULL_HANDLE;
        idxMem = VK_NULL_HANDLE;
        vtxBuf = VK_NULL_HANDLE;
        vtxMem = VK_NULL_HANDLE;
        bindDesc = {};
        attrDesc.clear();
        count = {0, 0};
    }

    void Init(VkDevice device, const MemoryAllocator* memAllocator, const std::vector<VkVertexInputAttributeDescription>& attr) {
        m_vkDevice = device;
        m_memAllocator = memAllocator;
        attrDesc = attr;
    }

   protected:
    VkDevice m_vkDevice{VK_NULL_HANDLE};
    void AllocateBufferMemory(VkBuffer buf, VkDeviceMemory* mem) const {
        VkMemoryRequirements memReq = {};
        vkGetBufferMemoryRequirements(m_vkDevice, buf, &memReq);
        m_memAllocator->Allocate(memReq, mem);
    }

   private:
    const MemoryAllocator* m_memAllocator{nullptr};
};

// VertexBuffer template to wrap the indices and vertices
template <typename T>
struct VertexBuffer : public VertexBufferBase {
    bool Create(uint32_t idxCount, uint32_t vtxCount) {
        VkBufferCreateInfo bufInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        bufInfo.size = sizeof(uint16_t) * idxCount;
        CHECK_VKCMD(vkCreateBuffer(m_vkDevice, &bufInfo, nullptr, &idxBuf));
        AllocateBufferMemory(idxBuf, &idxMem);
        CHECK_VKCMD(vkBindBufferMemory(m_vkDevice, idxBuf, idxMem, 0));

        bufInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufInfo.size = sizeof(T) * vtxCount;
        CHECK_VKCMD(vkCreateBuffer(m_vkDevice, &bufInfo, nullptr, &vtxBuf));
        AllocateBufferMemory(vtxBuf, &vtxMem);
        CHECK_VKCMD(vkBindBufferMemory(m_vkDevice, vtxBuf, vtxMem, 0));

        bindDesc.binding = 0;
        bindDesc.stride = sizeof(T);
        bindDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        count = {idxCount, vtxCount};

        return true;
    }

    void UpdateIndicies(const uint16_t* data, uint32_t elements, uint32_t offset = 0) {
        uint16_t* map = nullptr;
        CHECK_VKCMD(vkMapMemory(m_vkDevice, idxMem, sizeof(map[0]) * offset, sizeof(map[0]) * elements, 0, (void**)&map));
        for (size_t i = 0; i < elements; ++i) map[i] = data[i];
        vkUnmapMemory(m_vkDevice, idxMem);
    }

    void UpdateVertices(const T* data, uint32_t elements, uint32_t offset = 0) {
        T* map = nullptr;
        CHECK_VKCMD(vkMapMemory(m_vkDevice, vtxMem, sizeof(map[0]) * offset, sizeof(map[0]) * elements, 0, (void**)&map));
        for (size_t i = 0; i < elements; ++i) map[i] = data[i];
        vkUnmapMemory(m_vkDevice, vtxMem);
    }
};

// RenderPass wrapper
struct RenderPass {
    VkFormat colorFmt{};
    VkFormat depthFmt{};
    VkRenderPass pass{VK_NULL_HANDLE};

    RenderPass() {}

    bool Create(VkDevice device, VkFormat aColorFmt, VkFormat aDepthFmt) {
        m_vkDevice = device;
        colorFmt = aColorFmt;
        depthFmt = aDepthFmt;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        VkAttachmentReference colorRef = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference depthRef = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

        std::array<VkAttachmentDescription, 2> at = {};

        VkRenderPassCreateInfo rpInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        rpInfo.attachmentCount = 0;
        rpInfo.pAttachments = at.data();
        rpInfo.subpassCount = 1;
        rpInfo.pSubpasses = &subpass;

        if (colorFmt != VK_FORMAT_UNDEFINED) {
            colorRef.attachment = rpInfo.attachmentCount++;

            at[colorRef.attachment].format = colorFmt;
            at[colorRef.attachment].samples = VK_SAMPLE_COUNT_1_BIT;
            at[colorRef.attachment].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            at[colorRef.attachment].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            at[colorRef.attachment].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            at[colorRef.attachment].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            at[colorRef.attachment].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            at[colorRef.attachment].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorRef;
        }

        if (depthFmt != VK_FORMAT_UNDEFINED) {
            depthRef.attachment = rpInfo.attachmentCount++;

            at[depthRef.attachment].format = depthFmt;
            at[depthRef.attachment].samples = VK_SAMPLE_COUNT_1_BIT;
            at[depthRef.attachment].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            at[depthRef.attachment].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            at[depthRef.attachment].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            at[depthRef.attachment].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            at[depthRef.attachment].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            at[depthRef.attachment].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            subpass.pDepthStencilAttachment = &depthRef;
        }

        CHECK_VKCMD(vkCreateRenderPass(m_vkDevice, &rpInfo, nullptr, &pass));

        return true;
    }

    ~RenderPass() {
        if (m_vkDevice) {
            if (pass) vkDestroyRenderPass(m_vkDevice, pass, nullptr);
        }
        pass = VK_NULL_HANDLE;
    }

   private:
    VkDevice m_vkDevice{VK_NULL_HANDLE};
};

// VkImage + framebuffer wrapper
struct RenderTarget {
    VkImage colorImage{VK_NULL_HANDLE};
    VkImage depthImage{VK_NULL_HANDLE};
    VkImageView colorView{VK_NULL_HANDLE};
    VkImageView depthView{VK_NULL_HANDLE};
    VkFramebuffer fb{VK_NULL_HANDLE};

    RenderTarget() {}

    ~RenderTarget() {
        if (m_vkDevice) {
            if (fb) vkDestroyFramebuffer(m_vkDevice, fb, nullptr);
            if (colorView) vkDestroyImageView(m_vkDevice, colorView, nullptr);
            if (depthView) vkDestroyImageView(m_vkDevice, depthView, nullptr);
        }

        // Note we don't own color/depthImage, it will get destroyed when xrDestroySwapchain is called
        colorImage = VK_NULL_HANDLE;
        depthImage = VK_NULL_HANDLE;
        colorView = VK_NULL_HANDLE;
        depthView = VK_NULL_HANDLE;
        fb = VK_NULL_HANDLE;
    }

    void Create(VkDevice device, VkImage aColorImage, VkImage aDepthImage, VkExtent2D size, RenderPass& renderPass) {
        m_vkDevice = device;

        colorImage = aColorImage;
        depthImage = aDepthImage;

        std::array<VkImageView, 2> attachments{};
        uint32_t attachmentCount = 0;

        // Create color image view
        if (colorImage) {
            VkImageViewCreateInfo colorViewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            colorViewInfo.image = colorImage;
            colorViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            colorViewInfo.format = renderPass.colorFmt;
            colorViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
            colorViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
            colorViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
            colorViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
            colorViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            colorViewInfo.subresourceRange.baseMipLevel = 0;
            colorViewInfo.subresourceRange.levelCount = 1;
            colorViewInfo.subresourceRange.baseArrayLayer = 0;
            colorViewInfo.subresourceRange.layerCount = 1;
            CHECK_VKCMD(vkCreateImageView(m_vkDevice, &colorViewInfo, nullptr, &colorView));
            attachments[attachmentCount++] = colorView;
        }

        // Create depth image view
        if (depthImage) {
            VkImageViewCreateInfo depthViewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            depthViewInfo.image = depthImage;
            depthViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            depthViewInfo.format = renderPass.depthFmt;
            depthViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
            depthViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
            depthViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
            depthViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
            depthViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            depthViewInfo.subresourceRange.baseMipLevel = 0;
            depthViewInfo.subresourceRange.levelCount = 1;
            depthViewInfo.subresourceRange.baseArrayLayer = 0;
            depthViewInfo.subresourceRange.layerCount = 1;
            CHECK_VKCMD(vkCreateImageView(m_vkDevice, &depthViewInfo, nullptr, &depthView));
            attachments[attachmentCount++] = depthView;
        }

        VkFramebufferCreateInfo fbInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        fbInfo.renderPass = renderPass.pass;
        fbInfo.attachmentCount = attachmentCount;
        fbInfo.pAttachments = attachments.data();
        fbInfo.width = size.width;
        fbInfo.height = size.height;
        fbInfo.layers = 1;
        CHECK_VKCMD(vkCreateFramebuffer(m_vkDevice, &fbInfo, nullptr, &fb));
    }

   private:
    VkDevice m_vkDevice{VK_NULL_HANDLE};
};

// Simple vertex MVP xform & color fragment shader layout
struct PipelineLayout {
    VkPipelineLayout layout{VK_NULL_HANDLE};

    PipelineLayout() {}

    ~PipelineLayout() {
        if (m_vkDevice) {
            if (layout) vkDestroyPipelineLayout(m_vkDevice, layout, nullptr);
        }
        layout = VK_NULL_HANDLE;
    }

    void Create(VkDevice device) {
        m_vkDevice = device;

        // MVP matrix is a push_constant
        VkPushConstantRange pcr = {};
        pcr.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pcr.offset = 0;
        pcr.size = 4 * 4 * sizeof(float);

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pipelineLayoutCreateInfo.pPushConstantRanges = &pcr;
        CHECK_VKCMD(vkCreatePipelineLayout(m_vkDevice, &pipelineLayoutCreateInfo, nullptr, &layout));
    }

   private:
    VkDevice m_vkDevice{VK_NULL_HANDLE};
};

// Pipeline wrapper for rendering pipeline state
struct Pipeline {
    VkPipeline pipe{VK_NULL_HANDLE};
    VkPrimitiveTopology topology{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
    std::vector<VkDynamicState> dynamicStateEnables;

    Pipeline() {}

    void Dynamic(VkDynamicState state) { dynamicStateEnables.emplace_back(state); }

    void Create(VkDevice device, VkExtent2D size, const PipelineLayout& layout, const RenderPass& rp, const ShaderProgram& sp,
                const VertexBufferBase& vb) {
        m_vkDevice = device;

        VkPipelineDynamicStateCreateInfo dynamicState{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
        dynamicState.dynamicStateCount = (uint32_t)dynamicStateEnables.size();
        dynamicState.pDynamicStates = dynamicStateEnables.data();

        VkPipelineVertexInputStateCreateInfo vi{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        vi.vertexBindingDescriptionCount = 1;
        vi.pVertexBindingDescriptions = &vb.bindDesc;
        vi.vertexAttributeDescriptionCount = (uint32_t)vb.attrDesc.size();
        vi.pVertexAttributeDescriptions = vb.attrDesc.data();

        VkPipelineInputAssemblyStateCreateInfo ia{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        ia.primitiveRestartEnable = VK_FALSE;
        ia.topology = topology;

        VkPipelineRasterizationStateCreateInfo rs{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        rs.polygonMode = VK_POLYGON_MODE_FILL;
        rs.cullMode = VK_CULL_MODE_BACK_BIT;
        rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rs.depthClampEnable = VK_FALSE;
        rs.rasterizerDiscardEnable = VK_FALSE;
        rs.depthBiasEnable = VK_FALSE;
        rs.depthBiasConstantFactor = 0;
        rs.depthBiasClamp = 0;
        rs.depthBiasSlopeFactor = 0;
        rs.lineWidth = 1.0f;

        VkPipelineColorBlendAttachmentState attachState{};
        attachState.blendEnable = 0;
        attachState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        attachState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        attachState.colorBlendOp = VK_BLEND_OP_ADD;
        attachState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        attachState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        attachState.alphaBlendOp = VK_BLEND_OP_ADD;
        attachState.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo cb{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        cb.attachmentCount = 1;
        cb.pAttachments = &attachState;
        cb.logicOpEnable = VK_FALSE;
        cb.logicOp = VK_LOGIC_OP_NO_OP;
        cb.blendConstants[0] = 1.0f;
        cb.blendConstants[1] = 1.0f;
        cb.blendConstants[2] = 1.0f;
        cb.blendConstants[3] = 1.0f;

        VkRect2D scissor = {{0, 0}, size};
#if defined(ORIGIN_BOTTOM_LEFT)
        // Flipped view so origin is bottom-left like GL (requires VK_KHR_maintenance1)
        VkViewport viewport = {0.0f, (float)size.height, (float)size.width, -(float)size.height, 0.0f, 1.0f};
#else
        // Will invert y after projection
        VkViewport viewport = {0.0f, 0.0f, (float)size.width, (float)size.height, 0.0f, 1.0f};
#endif
        VkPipelineViewportStateCreateInfo vp{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        vp.viewportCount = 1;
        vp.pViewports = &viewport;
        vp.scissorCount = 1;
        vp.pScissors = &scissor;

        VkPipelineDepthStencilStateCreateInfo ds{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        ds.depthTestEnable = VK_TRUE;
        ds.depthWriteEnable = VK_TRUE;
        ds.depthCompareOp = VK_COMPARE_OP_LESS;
        ds.depthBoundsTestEnable = VK_FALSE;
        ds.stencilTestEnable = VK_FALSE;
        ds.front.failOp = VK_STENCIL_OP_KEEP;
        ds.front.passOp = VK_STENCIL_OP_KEEP;
        ds.front.depthFailOp = VK_STENCIL_OP_KEEP;
        ds.front.compareOp = VK_COMPARE_OP_ALWAYS;
        ds.back = ds.front;
        ds.minDepthBounds = 0.0f;
        ds.maxDepthBounds = 1.0f;

        VkPipelineMultisampleStateCreateInfo ms{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkGraphicsPipelineCreateInfo pipeInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        pipeInfo.stageCount = (uint32_t)sp.shaderInfo.size();
        pipeInfo.pStages = sp.shaderInfo.data();
        pipeInfo.pVertexInputState = &vi;
        pipeInfo.pInputAssemblyState = &ia;
        pipeInfo.pTessellationState = nullptr;
        pipeInfo.pViewportState = &vp;
        pipeInfo.pRasterizationState = &rs;
        pipeInfo.pMultisampleState = &ms;
        pipeInfo.pDepthStencilState = &ds;
        pipeInfo.pColorBlendState = &cb;
        if (dynamicState.dynamicStateCount > 0) {
            pipeInfo.pDynamicState = &dynamicState;
        }
        pipeInfo.layout = layout.layout;
        pipeInfo.renderPass = rp.pass;
        pipeInfo.subpass = 0;
        CHECK_VKCMD(vkCreateGraphicsPipelines(m_vkDevice, VK_NULL_HANDLE, 1, &pipeInfo, nullptr, &pipe));
    }

    void Release() {
        if (m_vkDevice) {
            if (pipe) vkDestroyPipeline(m_vkDevice, pipe, nullptr);
        }
        pipe = VK_NULL_HANDLE;
    }

   private:
    VkDevice m_vkDevice{VK_NULL_HANDLE};
};

struct DepthBuffer {
    VkDeviceMemory depthMemory{VK_NULL_HANDLE};
    VkImage depthImage{VK_NULL_HANDLE};

    DepthBuffer() {}

    ~DepthBuffer() {
        if (depthImage) vkDestroyImage(m_vkDevice, depthImage, nullptr);
        if (depthMemory) vkFreeMemory(m_vkDevice, depthMemory, nullptr);
    }

    void Create(VkDevice device, MemoryAllocator* memAllocator, VkFormat depthFormat,
                const XrSwapchainCreateInfo& swapchainCreateInfo) {
        m_vkDevice = device;

        VkExtent2D size = {swapchainCreateInfo.width, swapchainCreateInfo.height};

        // Create a D32 depthbuffer
        VkImageCreateInfo imageInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = size.width;
        imageInfo.extent.height = size.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = depthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.samples = (VkSampleCountFlagBits)swapchainCreateInfo.sampleCount;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        CHECK_VKCMD(vkCreateImage(device, &imageInfo, nullptr, &depthImage));

        VkMemoryRequirements memRequirements{};
        vkGetImageMemoryRequirements(device, depthImage, &memRequirements);
        memAllocator->Allocate(memRequirements, &depthMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        CHECK_VKCMD(vkBindImageMemory(device, depthImage, depthMemory, 0));
    }

   private:
    VkDevice m_vkDevice{VK_NULL_HANDLE};
};

struct SwapchainImageContext {
    // A packed array of XrSwapchainImageVulkanKHR's for xrEnumerateSwapchainImages
    std::vector<XrSwapchainImageVulkanKHR> swapchainImages;
    std::vector<RenderTarget> renderTarget;
    VkExtent2D size{};
    DepthBuffer depthBuffer{};
    RenderPass rp{};
    Pipeline pipe{};

    SwapchainImageContext() {}

    std::vector<XrSwapchainImageBaseHeader*> Create(VkDevice device, MemoryAllocator* memAllocator, uint32_t capacity,
                                                    const XrSwapchainCreateInfo& swapchainCreateInfo, const PipelineLayout& layout,
                                                    const ShaderProgram& sp, const VertexBuffer<Geometry::Vertex>& vb) {
        m_vkDevice = device;

        size = {swapchainCreateInfo.width, swapchainCreateInfo.height};
        VkFormat colorFormat = (VkFormat)swapchainCreateInfo.format;
        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
        // XXX handle swapchainCreateInfo.sampleCount

        depthBuffer.Create(m_vkDevice, memAllocator, depthFormat, swapchainCreateInfo);
        rp.Create(m_vkDevice, colorFormat, depthFormat);
        pipe.Create(m_vkDevice, size, layout, rp, sp, vb);

        swapchainImages.resize(capacity);
        renderTarget.resize(capacity);
        std::vector<XrSwapchainImageBaseHeader*> bases(capacity);
        for (uint32_t i = 0; i < capacity; ++i) {
            swapchainImages[i] = {XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR};
            bases[i] = reinterpret_cast<XrSwapchainImageBaseHeader*>(&swapchainImages[i]);
        }

        return bases;
    }

    uint32_t ImageIndex(const XrSwapchainImageBaseHeader* swapchainImageHeader) {
        auto p = reinterpret_cast<const XrSwapchainImageVulkanKHR*>(swapchainImageHeader);
        return (uint32_t)(p - &swapchainImages[0]);
    }

    void BindRenderTarget(uint32_t index, VkRenderPassBeginInfo* renderPassBeginInfo) {
        if (renderTarget[index].fb == VK_NULL_HANDLE) {
            renderTarget[index].Create(m_vkDevice, swapchainImages[index].image, depthBuffer.depthImage, size, rp);
        }
        renderPassBeginInfo->renderPass = rp.pass;
        renderPassBeginInfo->framebuffer = renderTarget[index].fb;
        renderPassBeginInfo->renderArea.offset = {0, 0};
        renderPassBeginInfo->renderArea.extent = size;
    }

   private:
    VkDevice m_vkDevice{VK_NULL_HANDLE};
};

#if defined(USE_MIRROR_WINDOW)
// Swapchain
struct Swapchain {
    VkFormat format{VK_FORMAT_B8G8R8A8_SRGB};
    VkSurfaceKHR surface{VK_NULL_HANDLE};
    VkSwapchainKHR swapchain{VK_NULL_HANDLE};
    VkFence readyFence{VK_NULL_HANDLE};
    VkFence presentFence{VK_NULL_HANDLE};
    static const uint32_t maxImages = 4;
    uint32_t swapchainCount = 0;
    uint32_t renderImageIdx = 0;
    VkImage image[maxImages]{VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE};

    Swapchain() {}
    ~Swapchain() { Release(); }

    void Create(VkInstance instance, VkPhysicalDevice physDevice, VkDevice device, uint32_t queueFamilyIndex);
    void Prepare(VkCommandBuffer buf);
    void Wait();
    void Acquire(VkSemaphore readySemaphore = VK_NULL_HANDLE);
    void Present(VkQueue queue, VkSemaphore drawComplete = VK_NULL_HANDLE);
    void Release() {
        if (m_vkDevice) {
            // Flush any pending Present() calls which are using the fence
            Wait();
            if (swapchain) vkDestroySwapchainKHR(m_vkDevice, swapchain, nullptr);
            if (readyFence) vkDestroyFence(m_vkDevice, readyFence, nullptr);
        }

        if (m_vkInstance && surface) vkDestroySurfaceKHR(m_vkInstance, surface, nullptr);

        readyFence = VK_NULL_HANDLE;
        presentFence = VK_NULL_HANDLE;
        swapchain = VK_NULL_HANDLE;
        surface = VK_NULL_HANDLE;
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            image[i] = VK_NULL_HANDLE;
        }
        swapchainCount = 0;

#if defined(VK_USE_PLATFORM_WIN32_KHR)
        if (hWnd) {
            DestroyWindow(hWnd);
            hWnd = nullptr;
            UnregisterClassW(L"hello_xr", hInst);
        }
#endif
    }
    void Recreate() {
        Release();
        Create(m_vkInstance, m_vkPhysicalDevice, m_vkDevice, m_queueFamilyIndex);
    }

   private:
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    HINSTANCE hInst{NULL};
    HWND hWnd{NULL};
#endif
    const VkExtent2D size{640, 480};
    VkInstance m_vkInstance{VK_NULL_HANDLE};
    VkPhysicalDevice m_vkPhysicalDevice{VK_NULL_HANDLE};
    VkDevice m_vkDevice{VK_NULL_HANDLE};
    uint32_t m_queueFamilyIndex = 0;
};

void Swapchain::Create(VkInstance instance, VkPhysicalDevice physDevice, VkDevice device, uint32_t queueFamilyIndex) {
    m_vkInstance = instance;
    m_vkPhysicalDevice = physDevice;
    m_vkDevice = device;
    m_queueFamilyIndex = queueFamilyIndex;

// Create a WSI surface for the window:
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    hInst = GetModuleHandle(NULL);

    WNDCLASSW wc{};
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = DefWindowProcW;
    wc.cbWndExtra = sizeof(this);
    wc.hInstance = hInst;
    wc.lpszClassName = L"hello_xr";
    RegisterClassW(&wc);

// adjust the window size and show at InitDevice time
#if defined(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)
    // Make sure we're 1:1 for HMD pixels
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#endif
    RECT rect{0, 0, (LONG)size.width, (LONG)size.height};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
    hWnd = CreateWindowW(wc.lpszClassName, L"hello_xr (Vulkan)", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
                         rect.right - rect.left, rect.bottom - rect.top, 0, 0, hInst, 0);
    assert(hWnd != NULL);

    SetWindowLongPtr(hWnd, 0, LONG_PTR(this));

    VkWin32SurfaceCreateInfoKHR surfCreateInfo{VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    surfCreateInfo.flags = 0;
    surfCreateInfo.hinstance = hInst;
    surfCreateInfo.hwnd = hWnd;
    CHECK_VKCMD(vkCreateWin32SurfaceKHR(m_vkInstance, &surfCreateInfo, nullptr, &surface));
#else
#error CreateSurface not supported on this OS
#endif  // defined(VK_USE_PLATFORM_WIN32_KHR)

    VkSurfaceCapabilitiesKHR surfCaps;
    CHECK_VKCMD(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_vkPhysicalDevice, surface, &surfCaps));
    CHECK(surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    uint32_t surfFmtCount = 0;
    CHECK_VKCMD(vkGetPhysicalDeviceSurfaceFormatsKHR(m_vkPhysicalDevice, surface, &surfFmtCount, nullptr));
    std::vector<VkSurfaceFormatKHR> surfFmts(surfFmtCount);
    CHECK_VKCMD(vkGetPhysicalDeviceSurfaceFormatsKHR(m_vkPhysicalDevice, surface, &surfFmtCount, &surfFmts[0]));
    uint32_t foundFmt;
    for (foundFmt = 0; foundFmt < surfFmtCount; ++foundFmt) {
        if (surfFmts[foundFmt].format == format) break;
    }

    CHECK(foundFmt < surfFmtCount);

    uint32_t presentModeCount = 0;
    CHECK_VKCMD(vkGetPhysicalDeviceSurfacePresentModesKHR(m_vkPhysicalDevice, surface, &presentModeCount, nullptr));
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    CHECK_VKCMD(vkGetPhysicalDeviceSurfacePresentModesKHR(m_vkPhysicalDevice, surface, &presentModeCount, &presentModes[0]));

    // Do not use VSYNC for the mirror window, but Nvidia doesn't support IMMEDIATE so fall back to MAILBOX
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    for (uint32_t i = 0; i < presentModeCount; ++i) {
        if ((presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) || (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)) {
            presentMode = presentModes[i];
            break;
        }
    }

    VkBool32 presentable = false;
    CHECK_VKCMD(vkGetPhysicalDeviceSurfaceSupportKHR(m_vkPhysicalDevice, m_queueFamilyIndex, surface, &presentable));
    CHECK(presentable);

    VkSwapchainCreateInfoKHR swapchainInfo{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    swapchainInfo.flags = 0;
    swapchainInfo.surface = surface;
    swapchainInfo.minImageCount = surfCaps.minImageCount;
    swapchainInfo.imageFormat = format;
    swapchainInfo.imageColorSpace = surfFmts[foundFmt].colorSpace;
    swapchainInfo.imageExtent = size;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.queueFamilyIndexCount = 0;
    swapchainInfo.pQueueFamilyIndices = nullptr;
    swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = presentMode;
    swapchainInfo.clipped = true;
    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;
    CHECK_VKCMD(vkCreateSwapchainKHR(m_vkDevice, &swapchainInfo, nullptr, &swapchain));

    // Fence to throttle host on Acquire
    VkFenceCreateInfo fenceInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    CHECK_VKCMD(vkCreateFence(m_vkDevice, &fenceInfo, nullptr, &readyFence));

    swapchainCount = 0;
    CHECK_VKCMD(vkGetSwapchainImagesKHR(m_vkDevice, swapchain, &swapchainCount, nullptr));
    assert(swapchainCount < maxImages);
    CHECK_VKCMD(vkGetSwapchainImagesKHR(m_vkDevice, swapchain, &swapchainCount, image));
    if (swapchainCount > maxImages) {
        Log::Write(Log::Level::Info,
                   "Reducing swapchain length from " + std::to_string(swapchainCount) + " to " + std::to_string(maxImages));
        swapchainCount = maxImages;
    }

    Log::Write(Log::Level::Info, "Swapchain length " + std::to_string(swapchainCount));
}

void Swapchain::Prepare(VkCommandBuffer buf) {
    // Convert swapchain images to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    for (uint32_t i = 0; i < swapchainCount; ++i) {
        VkImageMemoryBarrier imgBarrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        imgBarrier.srcAccessMask = 0;  // XXX was VK_ACCESS_TRANSFER_READ_BIT wrong?
        imgBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imgBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imgBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        imgBarrier.image = image[i];
        imgBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        vkCmdPipelineBarrier(buf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                             &imgBarrier);
    }
}

void Swapchain::Wait() {
    if (presentFence) {
        // Wait for the fence...
        CHECK_VKCMD(vkWaitForFences(m_vkDevice, 1, &presentFence, VK_TRUE, UINT64_MAX));
        // ...then reset the fence for future Acquire calls
        CHECK_VKCMD(vkResetFences(m_vkDevice, 1, &presentFence));
        presentFence = VK_NULL_HANDLE;
    }
}

void Swapchain::Acquire(VkSemaphore readySemaphore) {
    // If we're not using a semaphore to rate-limit the GPU, rate limit the host with a fence instead
    if (readySemaphore == VK_NULL_HANDLE) {
        Wait();
        presentFence = readyFence;
    }

    CHECK_VKCMD(vkAcquireNextImageKHR(m_vkDevice, swapchain, UINT64_MAX, readySemaphore, presentFence, &renderImageIdx));
}

void Swapchain::Present(VkQueue queue, VkSemaphore drawComplete) {
    VkPresentInfoKHR presentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    if (drawComplete) {
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &drawComplete;
    }
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &renderImageIdx;
    auto res = vkQueuePresentKHR(queue, &presentInfo);
    if (res == VK_ERROR_OUT_OF_DATE_KHR) {
        Recreate();
        return;
    }
    CHECK_VKRESULT(res, "vkQueuePresentKHR");
}
#endif  // defined(USE_MIRROR_WINDOW)

struct VulkanGraphicsPlugin : public IGraphicsPlugin {
    VulkanGraphicsPlugin(const std::shared_ptr<Options>&, std::shared_ptr<IPlatformPlugin>){};

    std::vector<std::string> GetInstanceExtensions() const override { return {XR_KHR_VULKAN_ENABLE_EXTENSION_NAME}; }

    std::vector<const char*> ParseExtensionString(char* names) {
        std::vector<const char*> list;
        while (*names) {
            list.push_back(names);
            while (*(++names)) {
                if (*names == ' ') {
                    *names++ = '\0';
                    break;
                }
            }
        }
        return list;
    }

    void InitializeDevice(XrInstance instance, XrSystemId systemId) override {
        // Create the Vulkan device for the adapter associated with the system.
        XrGraphicsRequirementsVulkanKHR graphicsRequirements{XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR};
        CHECK_XRCMD(xrGetVulkanGraphicsRequirementsKHR(instance, systemId, &graphicsRequirements));

        uint32_t extensionNamesSize = 0;
        CHECK_XRCMD(xrGetVulkanInstanceExtensionsKHR(instance, systemId, 0, &extensionNamesSize, nullptr));
        std::vector<char> extensionNames(extensionNamesSize);
        CHECK_XRCMD(
            xrGetVulkanInstanceExtensionsKHR(instance, systemId, extensionNamesSize, &extensionNamesSize, &extensionNames[0]));

        std::vector<const char*> extensions = ParseExtensionString(&extensionNames[0]);
        extensions.push_back("VK_EXT_debug_report");

        std::vector<const char*> layers;
#if defined(_DEBUG)
        layers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif

        VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
        appInfo.pApplicationName = "hello_xr";
        appInfo.applicationVersion = 1;
        appInfo.pEngineName = "hello_xr";
        appInfo.engineVersion = 1;
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo instInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        instInfo.pApplicationInfo = &appInfo;
        instInfo.enabledLayerCount = (uint32_t)layers.size();
        instInfo.ppEnabledLayerNames = layers.size() ? &layers[0] : nullptr;
        instInfo.enabledExtensionCount = (uint32_t)extensions.size();
        instInfo.ppEnabledExtensionNames = extensions.size() ? &extensions[0] : nullptr;

        CHECK_VKCMD(vkCreateInstance(&instInfo, nullptr, &m_vkInstance));

        vkCreateDebugReportCallbackEXT =
            (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_vkInstance, "vkCreateDebugReportCallbackEXT");
        vkDestroyDebugReportCallbackEXT =
            (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(m_vkInstance, "vkDestroyDebugReportCallbackEXT");
        VkDebugReportCallbackCreateInfoEXT debugInfo{VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT};
        debugInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
#if defined(_DEBUG)
        debugInfo.flags |=
            VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
#endif
        debugInfo.pfnCallback = debugReportThunk;
        debugInfo.pUserData = this;
        CHECK_VKCMD(vkCreateDebugReportCallbackEXT(m_vkInstance, &debugInfo, nullptr, &m_vkDebugReporter));

        CHECK_XRCMD(xrGetVulkanGraphicsDeviceKHR(instance, systemId, m_vkInstance, &m_vkPhysicalDevice));

        VkDeviceQueueCreateInfo queueInfo{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        float queuePriorities = 0;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &queuePriorities;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilyProps(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice, &queueFamilyCount, &queueFamilyProps[0]);

        for (uint32_t i = 0; i < queueFamilyCount; ++i) {
            // Only need graphics (not presentation) for draw queue
            if (queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                m_queueFamilyIndex = queueInfo.queueFamilyIndex = i;
                break;
            }
        }

        uint32_t deviceExtensionNamesSize = 0;
        CHECK_XRCMD(xrGetVulkanDeviceExtensionsKHR(instance, systemId, 0, &deviceExtensionNamesSize, nullptr));
        std::vector<char> deviceExtensionNames(deviceExtensionNamesSize);
        CHECK_XRCMD(xrGetVulkanDeviceExtensionsKHR(instance, systemId, deviceExtensionNamesSize, &deviceExtensionNamesSize,
                                                   &deviceExtensionNames[0]));
        std::vector<const char*> deviceExtensions = ParseExtensionString(&deviceExtensionNames[0]);

        VkPhysicalDeviceFeatures features{};
        // features.samplerAnisotropy = VK_TRUE;
        // Setting this quiets down a validation error triggered by the Oculus runtime
        // features.shaderStorageImageMultisample = VK_TRUE;

        VkDeviceCreateInfo deviceInfo{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        deviceInfo.queueCreateInfoCount = 1;
        deviceInfo.pQueueCreateInfos = &queueInfo;
        deviceInfo.enabledLayerCount = 0;
        deviceInfo.ppEnabledLayerNames = nullptr;
        deviceInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
        deviceInfo.ppEnabledExtensionNames = deviceExtensions.size() ? &deviceExtensions[0] : nullptr;
        deviceInfo.pEnabledFeatures = &features;

        CHECK_VKCMD(vkCreateDevice(m_vkPhysicalDevice, &deviceInfo, nullptr, &m_vkDevice));

        vkGetDeviceQueue(m_vkDevice, queueInfo.queueFamilyIndex, 0, &m_vkQueue);

        m_memAllocator.Init(m_vkPhysicalDevice, m_vkDevice);

        InitializeResources();

        m_graphicsBinding.instance = m_vkInstance;
        m_graphicsBinding.physicalDevice = m_vkPhysicalDevice;
        m_graphicsBinding.device = m_vkDevice;
        m_graphicsBinding.queueFamilyIndex = queueInfo.queueFamilyIndex;
        m_graphicsBinding.queueIndex = 0;
    }

#ifdef USE_ONLINE_VULKAN_SHADERC
    // Compile a shader to a SPIR-V binary.
    std::vector<uint32_t> CompileGlslShader(const std::string& name, shaderc_shader_kind kind, const std::string& source) {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        options.SetOptimizationLevel(shaderc_optimization_level_size);

        shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, kind, name.c_str(), options);

        if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
            Log::Write(Log::Level::Error, Fmt("Shader %s compilation failed: %s", name.c_str(), module.GetErrorMessage().c_str()));
            return std::vector<uint32_t>();
        }

        return {module.cbegin(), module.cend()};
    }
#endif

    void InitializeResources() {
#ifdef USE_ONLINE_VULKAN_SHADERC
        auto vertexSPIRV = CompileGlslShader("vertex", shaderc_glsl_default_vertex_shader, VertexShaderGlsl);
        auto fragmentSPIRV = CompileGlslShader("fragment", shaderc_glsl_default_fragment_shader, FragmentShaderGlsl);
#else
        std::vector<uint32_t> vertexSPIRV =
#include "vert.spv"
            ;
        std::vector<uint32_t> fragmentSPIRV =
#include "frag.spv"
            ;
#endif
        if (vertexSPIRV.empty()) THROW("Failed to compile vertex shader");
        if (fragmentSPIRV.empty()) THROW("Failed to compile fragment shader");

        m_shaderProgram.Init(m_vkDevice);
        m_shaderProgram.LoadVertexShader(vertexSPIRV);
        m_shaderProgram.LoadFragmentShader(fragmentSPIRV);

        // Semaphore to block on draw complete
        VkSemaphoreCreateInfo semInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        CHECK_VKCMD(vkCreateSemaphore(m_vkDevice, &semInfo, nullptr, &m_vkDrawDone));

        if (!m_cmdBuffer.Init(m_vkDevice, m_queueFamilyIndex)) THROW("Failed to create command buffer");

        m_pipelineLayout.Create(m_vkDevice);

        static_assert(sizeof(Geometry::Vertex) == 24, "Unexpected Vertex size");
        m_drawBuffer.Init(m_vkDevice, &m_memAllocator,
                          {{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Geometry::Vertex, Position)},
                           {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Geometry::Vertex, Color)}});
        uint32_t numCubeIdicies = sizeof(Geometry::c_cubeIndices) / sizeof(Geometry::c_cubeIndices[0]);
        uint32_t numCubeVerticies = sizeof(Geometry::c_cubeVertices) / sizeof(Geometry::c_cubeVertices[0]);
        m_drawBuffer.Create(numCubeIdicies, numCubeVerticies);
        m_drawBuffer.UpdateIndicies(Geometry::c_cubeIndices, numCubeIdicies, 0);
        m_drawBuffer.UpdateVertices(Geometry::c_cubeVertices, numCubeVerticies, 0);

#if defined(USE_MIRROR_WINDOW)
        m_swapchain.Create(m_vkInstance, m_vkPhysicalDevice, m_vkDevice, m_graphicsBinding.queueFamilyIndex);

        m_cmdBuffer.Reset();
        m_cmdBuffer.Begin();
        m_swapchain.Prepare(m_cmdBuffer.buf);
        m_cmdBuffer.End();
        m_cmdBuffer.Exec(m_vkQueue);
        m_cmdBuffer.Wait();
#endif
    }

    int64_t SelectColorSwapchainFormat(const std::vector<int64_t>& runtimeFormats) const override {
        // List of supported color swapchain formats, in priority order.
        constexpr int64_t SupportedColorSwapchainFormats[] = {VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_R8G8B8A8_SRGB,
                                                              VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM};

        auto swapchainFormatIt =
            std::find_first_of(std::begin(SupportedColorSwapchainFormats), std::end(SupportedColorSwapchainFormats),
                               runtimeFormats.begin(), runtimeFormats.end());
        if (swapchainFormatIt == std::end(SupportedColorSwapchainFormats)) {
            THROW("No runtime swapchain format supported for color swapchain");
        }

        return *swapchainFormatIt;
    }

    const XrBaseInStructure* GetGraphicsBinding() const override {
        return reinterpret_cast<const XrBaseInStructure*>(&m_graphicsBinding);
    }

    std::vector<XrSwapchainImageBaseHeader*> AllocateSwapchainImageStructs(
        uint32_t capacity, const XrSwapchainCreateInfo& swapchainCreateInfo) override {
        // Allocate and initialize the buffer of image structs (must be sequential in memory for xrEnumerateSwapchainImages).
        // Return back an array of pointers to each swapchain image struct so the consumer doesn't need to know the type/size.
        // Keep the buffer alive by adding it into the list of buffers.
        m_swapchainImageContexts.push_back({});
        SwapchainImageContext& swapchainImageContext = m_swapchainImageContexts.back();

        std::vector<XrSwapchainImageBaseHeader*> bases = swapchainImageContext.Create(
            m_vkDevice, &m_memAllocator, capacity, swapchainCreateInfo, m_pipelineLayout, m_shaderProgram, m_drawBuffer);

        // Map every swapchainImage base pointer to this context
        for (auto& base : bases) {
            m_swapchainImageContextMap.emplace(std::make_pair(base, &swapchainImageContext));
        }

        return bases;
    }

    void RenderView(const XrCompositionLayerProjectionView& layerView, const XrSwapchainImageBaseHeader* swapchainImage,
                    int64_t /*swapchainFormat*/, const std::vector<Cube>& cubes) override {
        CHECK(layerView.subImage.imageArrayIndex == 0);  // Texture arrays not supported.

        auto swapchainContext = m_swapchainImageContextMap[swapchainImage];
        uint32_t imageIndex = swapchainContext->ImageIndex(swapchainImage);

        m_cmdBuffer.Reset();
        m_cmdBuffer.Begin();

        // Bind and clear eye render target
        static XrColor4f darkSlateGrey = {0.184313729f, 0.309803933f, 0.309803933f, 1.0f};
        static std::array<VkClearValue, 2> clearValues;
        clearValues[0].color.float32[0] = darkSlateGrey.r;
        clearValues[0].color.float32[1] = darkSlateGrey.g;
        clearValues[0].color.float32[2] = darkSlateGrey.b;
        clearValues[0].color.float32[3] = darkSlateGrey.a;
        clearValues[1].depthStencil.depth = 1.0f;
        clearValues[1].depthStencil.stencil = 0;
        VkRenderPassBeginInfo renderPassBeginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
        renderPassBeginInfo.pClearValues = clearValues.data();

        swapchainContext->BindRenderTarget(imageIndex, &renderPassBeginInfo);

        vkCmdBeginRenderPass(m_cmdBuffer.buf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(m_cmdBuffer.buf, VK_PIPELINE_BIND_POINT_GRAPHICS, swapchainContext->pipe.pipe);

        // Bind index and vertex buffers
        vkCmdBindIndexBuffer(m_cmdBuffer.buf, m_drawBuffer.idxBuf, 0, VK_INDEX_TYPE_UINT16);
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(m_cmdBuffer.buf, 0, 1, &m_drawBuffer.vtxBuf, &offset);

        // Compute the view-projection transform.
        // Note all matrixes (including OpenXR's) are column-major, right-handed.
        const auto& pose = layerView.pose;
        XrMatrix4x4f proj;
        XrMatrix4x4f_CreateProjectionFov(&proj, GRAPHICS_VULKAN, layerView.fov, 0.05f, 100.0f);
        XrMatrix4x4f toView;
        XrVector3f scale{1.f, 1.f, 1.f};
        XrMatrix4x4f_CreateTranslationRotationScale(&toView, &pose.position, &pose.orientation, &scale);
        XrMatrix4x4f view;
        XrMatrix4x4f_InvertRigidBody(&view, &toView);
        XrMatrix4x4f vp;
        XrMatrix4x4f_Multiply(&vp, &proj, &view);

        // Render each cube
        for (const Cube& cube : cubes) {
            // Compute the model-view-projection transform and push it.
            XrMatrix4x4f model;
            XrMatrix4x4f_CreateTranslationRotationScale(&model, &cube.Pose.position, &cube.Pose.orientation, &cube.Scale);
            XrMatrix4x4f mvp;
            XrMatrix4x4f_Multiply(&mvp, &vp, &model);
            vkCmdPushConstants(m_cmdBuffer.buf, m_pipelineLayout.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mvp.m), &mvp.m[0]);

            // Draw the cube.
            vkCmdDrawIndexed(m_cmdBuffer.buf, m_drawBuffer.count.idx, 1, 0, 0, 0);
        }

        vkCmdEndRenderPass(m_cmdBuffer.buf);

        m_cmdBuffer.End();
        m_cmdBuffer.Exec(m_vkQueue);
        // XXX Should double-buffer the command buffers, for now just flush
        m_cmdBuffer.Wait();

#if defined(USE_MIRROR_WINDOW)
        // Cycle the window's swapchain on the last view rendered
        if (swapchainContext == &m_swapchainImageContexts.back()) {
            m_swapchain.Acquire();
            m_swapchain.Present(m_vkQueue);
        }
#endif
    }

   private:
    XrGraphicsBindingVulkanKHR m_graphicsBinding{XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR};
    std::list<SwapchainImageContext> m_swapchainImageContexts;
    std::map<const XrSwapchainImageBaseHeader*, SwapchainImageContext*> m_swapchainImageContextMap;

    VkInstance m_vkInstance{VK_NULL_HANDLE};
    VkPhysicalDevice m_vkPhysicalDevice{VK_NULL_HANDLE};
    VkDevice m_vkDevice{VK_NULL_HANDLE};
    uint32_t m_queueFamilyIndex = 0;
    VkQueue m_vkQueue{VK_NULL_HANDLE};
    VkSemaphore m_vkDrawDone{VK_NULL_HANDLE};
    uint32_t m_vkDeviceLocalHeap = 0;

    MemoryAllocator m_memAllocator{};
    ShaderProgram m_shaderProgram{};
    CmdBuffer m_cmdBuffer{};
    PipelineLayout m_pipelineLayout{};
    VertexBuffer<Geometry::Vertex> m_drawBuffer{};

#if defined(USE_MIRROR_WINDOW)
    Swapchain m_swapchain{};
#endif

    PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT{nullptr};
    PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT{nullptr};
    VkDebugReportCallbackEXT m_vkDebugReporter{VK_NULL_HANDLE};

    VkBool32 debugReport(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t /*location*/,
                         int32_t /*messageCode*/, const char* pLayerPrefix, const char* pMessage) {
        std::string flagNames;
        std::string objName;
        Log::Level level = Log::Level::Error;

        if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
            flagNames += "DEBUG:";
            level = Log::Level::Verbose;
        }
        if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
            flagNames += "INFO:";
            level = Log::Level::Info;
        }
        if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
            flagNames += "PERF:";
            level = Log::Level::Warning;
        }
        if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
            flagNames += "WARN:";
            level = Log::Level::Warning;
        }
        if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
            flagNames += "ERROR:";
            level = Log::Level::Error;
        }

#define LIST_OBJECT_TYPES(_)        \
    _(UNKNOWN)                      \
    _(INSTANCE)                     \
    _(PHYSICAL_DEVICE)              \
    _(DEVICE)                       \
    _(QUEUE)                        \
    _(SEMAPHORE)                    \
    _(COMMAND_BUFFER)               \
    _(FENCE)                        \
    _(DEVICE_MEMORY)                \
    _(BUFFER)                       \
    _(IMAGE)                        \
    _(EVENT)                        \
    _(QUERY_POOL)                   \
    _(BUFFER_VIEW)                  \
    _(IMAGE_VIEW)                   \
    _(SHADER_MODULE)                \
    _(PIPELINE_CACHE)               \
    _(PIPELINE_LAYOUT)              \
    _(RENDER_PASS)                  \
    _(PIPELINE)                     \
    _(DESCRIPTOR_SET_LAYOUT)        \
    _(SAMPLER)                      \
    _(DESCRIPTOR_POOL)              \
    _(DESCRIPTOR_SET)               \
    _(FRAMEBUFFER)                  \
    _(COMMAND_POOL)                 \
    _(SURFACE_KHR)                  \
    _(SWAPCHAIN_KHR)                \
    _(DEBUG_REPORT_CALLBACK_EXT)    \
    _(DISPLAY_KHR)                  \
    _(DISPLAY_MODE_KHR)             \
    _(OBJECT_TABLE_NVX)             \
    _(INDIRECT_COMMANDS_LAYOUT_NVX) \
    _(DESCRIPTOR_UPDATE_TEMPLATE_KHR)

        switch (objectType) {
            default:
#define MK_OBJECT_TYPE_CASE(name)                  \
    case VK_DEBUG_REPORT_OBJECT_TYPE_##name##_EXT: \
        objName = #name;                           \
        break;
                LIST_OBJECT_TYPES(MK_OBJECT_TYPE_CASE)
        }

        if ((objectType == VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT) && (strcmp(pLayerPrefix, "Loader Message") == 0) &&
            (strncmp(pMessage, "Device Extension:", 17) == 0)) {
            return VK_FALSE;
        }

        Log::Write(level, Fmt("%s (%s 0x%llx) [%s] %s", flagNames.c_str(), objName.c_str(), object, pLayerPrefix, pMessage));
        if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
            return VK_FALSE;
        }
        if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
            return VK_FALSE;
        }
        return VK_FALSE;
    }

    static VkBool32 VKAPI_CALL debugReportThunk(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object,
                                                size_t location, int32_t messageCode, const char* pLayerPrefix,
                                                const char* pMessage, void* pUserData) {
        return static_cast<VulkanGraphicsPlugin*>(pUserData)->debugReport(flags, objectType, object, location, messageCode,
                                                                          pLayerPrefix, pMessage);
    }
};

}  // namespace

std::shared_ptr<IGraphicsPlugin> CreateGraphicsPlugin_Vulkan(const std::shared_ptr<Options>& options,
                                                             std::shared_ptr<IPlatformPlugin> platformPlugin) {
    return std::make_shared<VulkanGraphicsPlugin>(options, platformPlugin);
}

#endif  // XR_USE_GRAPHICS_API_VULKAN
