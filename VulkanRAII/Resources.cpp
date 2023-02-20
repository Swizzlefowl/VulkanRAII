#include "Resources.h"
#include "Graphics.h"
#include "PresentationEngine.h"
#include "Renderer.h"

// the attachments refer to the vkImage views which itself is a view into
// our swapchain images
void Resources::createframebuffers() {
    // we are creating temporary framebuffers to eventually
    // move ownership to our framebuffers
    // if you dont do it this way it will throw an exception when
    // recreating the swapchain

    std::vector<vk::raii::Framebuffer> framebuffs{};
    for (auto& attachment : m_renderer.pEngine->swapChainImageViews) {
        vk::FramebufferCreateInfo bufferInfo{};
        bufferInfo.width = m_renderer.pEngine->swapChainExtent.width;
        bufferInfo.height = m_renderer.pEngine->swapChainExtent.height;
        bufferInfo.layers = 1;
        bufferInfo.renderPass = *m_renderer.pGraphics->renderPass;
        bufferInfo.attachmentCount = 1;
        bufferInfo.pAttachments = &(*attachment);

        try {
            framebuffs.push_back(m_renderer.m_device.createFramebuffer(bufferInfo));
        } catch (vk::Error& err) {
            std::cout << err.what();
        }
    }
    frambebuffers = std::move(framebuffs);
}

void Resources::createCommandPools() {
    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = m_renderer.getQueueFamilyIndex();
    try {
        commandPool = m_renderer.m_device.createCommandPool(poolInfo);
    } catch (vk::Error& err) {
        std::cout << err.what();
    }
}

void Resources::createCommandbuffer() {
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool = *commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;
    commandBuffer = vk::raii::CommandBuffers{m_renderer.m_device, allocInfo};
}

void Resources::createSyncObjects() {
    vk::FenceCreateInfo fenceInfo;
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
    vk::SemaphoreCreateInfo semaphoreInfo{};

    try {
        inFlightFences = m_renderer.m_device.createFence(fenceInfo);
        imageAvailableSemaphores = m_renderer.m_device.createSemaphore(semaphoreInfo);
        finishedRenderingSemaphores = m_renderer.m_device.createSemaphore(semaphoreInfo);
    } catch (vk::Error& err) {
        std::cout << err.what();
    }
}

uint32_t Resources::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    vk::PhysicalDeviceMemoryProperties memProperties{
        m_renderer.m_physicalDevice.getMemoryProperties()};

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    throw std::runtime_error("failed to find suitable memory type!");
}

void Resources::createBuffers(vk::raii::Buffer& buffer, vk::raii::DeviceMemory& memory, vk::DeviceSize size) {
    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.size = size;
    uint32_t queueIndex = m_renderer.getQueueFamilyIndex();
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;
    bufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;

    try {
        buffer = m_renderer.m_device.createBuffer(bufferInfo);
    } catch (vk::Error& err) {
        std::cout << err.what();
    }
    vk::MemoryRequirements memRequirments{buffer.getMemoryRequirements()};
    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.allocationSize = memRequirments.size;
    allocInfo.memoryTypeIndex = findMemoryType(
        memRequirments.memoryTypeBits,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    try {
        memory = m_renderer.m_device.allocateMemory(allocInfo);
        buffer.bindMemory(*memory, 0);
    } catch (vk::Error& err) {
        std::cout << err.what();
    }
}

void Resources::mapMemory(vk::raii::DeviceMemory& memory, vk::DeviceSize size, const std::vector<glm::vec3>& vec) {
    auto data = memory.mapMemory(0, size);
    memcpy(data, vec.data(), size);
}

Resources::Resources(Renderer& renderer)
    : m_renderer{renderer} {
}

Resources::~Resources() {
    colorBufferMemory.unmapMemory();
}

void Resources::createResources() {
    createframebuffers();
    createCommandPools();
    createCommandbuffer();
    createSyncObjects();
    vk::DeviceSize posSize = static_cast<vk::DeviceSize>(sizeof(m_renderer.pos[0]) * m_renderer.pos.size());
    vk::DeviceSize colorSize = static_cast<vk::DeviceSize>(sizeof(m_renderer.color[0]) * m_renderer.color.size());
    createBuffers(posBuffer, posBufferMemory, posSize);
    createBuffers(colorBuffer, colorBufferMemory, colorSize);
    mapMemory(posBufferMemory, posSize, m_renderer.pos);
    colorPtr = colorBufferMemory.mapMemory(0, colorSize);
    memcpy(colorPtr, m_renderer.color.data(), colorSize);
    posBufferMemory.unmapMemory();
}
