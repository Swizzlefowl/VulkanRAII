#include "Resources.h"
#include "Graphics.h"
#include "PresentationEngine.h"
#include "Renderer.h"
#include "stb_image.h"
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

void Resources::createBlitFrameBuffer() {
    vk::FramebufferCreateInfo bufferInfo{};
    bufferInfo.width = m_renderer.pEngine->swapChainExtent.width;
    bufferInfo.height = m_renderer.pEngine->swapChainExtent.height;
    bufferInfo.layers = 1;
    bufferInfo.renderPass = *m_renderer.pGraphics->renderPass;
    bufferInfo.attachmentCount = 1;
    bufferInfo.pAttachments = &(*m_renderer.pEngine->blitImageViews);

    blitFramebuffer = m_renderer.m_device.createFramebuffer(bufferInfo);
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

void Resources::createBuffers(vk::raii::Buffer& buffer, vk::raii::DeviceMemory& memory, vk::DeviceSize size, vk::BufferUsageFlagBits usage) {
    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.size = size;
    uint32_t queueIndex = m_renderer.getQueueFamilyIndex();
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;
    bufferInfo.usage = usage;

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

void Resources::mapMemory(vk::raii::DeviceMemory& memory, vk::DeviceSize size, const auto& vec) {
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

    for (size_t index{}; index < 4; index++) {
        Renderer::Vertex vertex{.pos = m_renderer.pos[index], .color = m_renderer.color[index]};
        m_renderer.vertices.push_back(vertex);
    }
    createframebuffers();
    createCommandPools();
    createCommandbuffer();
    createSyncObjects();
    vk::DeviceSize vertexSize = static_cast<vk::DeviceSize>(sizeof(m_renderer.vertices[0]) * m_renderer.vertices.size());
    vk::DeviceSize colorSize = static_cast<vk::DeviceSize>(sizeof(m_renderer.color[0]) * m_renderer.color.size());
    vk::DeviceSize indexSize = static_cast<vk::DeviceSize>(sizeof(m_renderer.indices[0]) * m_renderer.indices.size());
    vk::DeviceSize uboSize = static_cast<vk::DeviceSize>(sizeof(Renderer::MeshPushConstants));
    createBuffers(posBuffer, posBufferMemory, vertexSize, vk::BufferUsageFlagBits::eVertexBuffer);
    createBuffers(colorBuffer, colorBufferMemory, colorSize, vk::BufferUsageFlagBits::eVertexBuffer);
    createBuffers(indexBuffer, indexBufferMemory, indexSize, vk::BufferUsageFlagBits::eIndexBuffer);
    createBuffers(uniformBuffer, uniformBufferMemory, uboSize, vk::BufferUsageFlagBits::eUniformBuffer);
    auto vertexPtr = posBufferMemory.mapMemory(0, vertexSize);
    memcpy(vertexPtr, m_renderer.vertices.data(), vertexSize);
    mapMemory(indexBufferMemory, indexSize, m_renderer.indices);
    colorPtr = colorBufferMemory.mapMemory(0, colorSize);
    memcpy(colorPtr, m_renderer.color.data(), colorSize);
    uboPtr = uniformBufferMemory.mapMemory(0, uboSize);
    posBufferMemory.unmapMemory();
    indexBufferMemory.unmapMemory();
}

void Resources::createDescriptorPool() {
    vk::DescriptorPoolSize poolSize{};
    poolSize.type = vk::DescriptorType::eUniformBuffer;
    poolSize.descriptorCount = 1;
    vk::DescriptorPoolCreateInfo createInfo{};
    createInfo.poolSizeCount = 1;
    createInfo.pPoolSizes = &poolSize;
    createInfo.maxSets = 1;
    createInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

    try {
        descriptorPool = m_renderer.m_device.createDescriptorPool(createInfo);
    } catch (vk::Error& err) {
        std::cout << err.what();
    }
}

void Resources::allocateDescriptorSets() {
    vk::DescriptorSetAllocateInfo allocateInfo{};
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.descriptorPool = *descriptorPool;
    allocateInfo.pSetLayouts = &*m_renderer.pGraphics->descriptorSetLayout;

    try {
        descriptorSet = m_renderer.m_device.allocateDescriptorSets(allocateInfo);
    } catch (vk::Error& err) {
        std::cout << err.what();
    }

    vk::DescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = *uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(Renderer::MeshPushConstants);

    vk::WriteDescriptorSet descriptorWrite{};
    descriptorWrite.dstSet = *descriptorSet[0];
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;
    descriptorWrite.pImageInfo = nullptr; // Optional
    descriptorWrite.pTexelBufferView = nullptr; // Optional

    m_renderer.m_device.updateDescriptorSets(descriptorWrite, nullptr);
}

vk::raii::Buffer Resources::createBuffer(vk::BufferUsageFlags usage, vk::DeviceSize size, VmaAllocationCreateFlags createFlags, VmaAllocation& allocation) {
    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.size = size;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.flags = createFlags;
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

    vk::Buffer buffer{};
    auto result = vmaCreateBuffer(m_renderer.allocator, reinterpret_cast<VkBufferCreateInfo*>(&bufferInfo), &allocInfo, reinterpret_cast<VkBuffer*>(&buffer), &allocation, nullptr);

    if (result != VkResult::VK_SUCCESS)
        std::cerr << "creating vkBuffer failed\n";

    return vk::raii::Buffer{m_renderer.m_device, buffer};
}

void Resources::mapMemory(const VmaAllocator& allocator, const VmaAllocation& allocation, void* src, VkDeviceSize size) {
    void* stagingMappedPtr{nullptr};
    auto result = vmaMapMemory(m_renderer.allocator, allocation, &stagingMappedPtr);

    if (result != VkResult::VK_SUCCESS)
        throw std::runtime_error("map memory failed");

    memcpy(stagingMappedPtr, src, size);
    vmaFlushAllocation(m_renderer.allocator, allocation, 0, size);
    vmaUnmapMemory(m_renderer.allocator, allocation);
}

void* Resources::mapPersistentMemory(const VmaAllocator& allocator, const VmaAllocation& allocation, VkDeviceSize size) {
    void* ptr{nullptr};
    auto result = vmaMapMemory(m_renderer.allocator, allocation, &ptr);

    if (result != VkResult::VK_SUCCESS)
        throw std::runtime_error("persistent map memory failed");
    return ptr;
}

void Resources::loadImage() {
    int texWidth{};
    int texHeight{};
    int texChannels{};
    stbi_uc* pixels{nullptr};
    pixels = stbi_load("statue.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    vk::DeviceSize imageSize{static_cast<vk::DeviceSize>(texWidth * texHeight * 4)};

    if (!pixels)
        throw std::runtime_error("failed to load image!");

    vk::raii::Buffer stagingBuffer{nullptr};
    VmaAllocation allocation{nullptr};
    stagingBuffer = createBuffer(vk::BufferUsageFlagBits::eTransferSrc, imageSize, VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, allocation);
    mapMemory(m_renderer.allocator, allocation, pixels, imageSize);

    VmaAllocation texImageAlloc{nullptr};
    auto texImage = createImage(texWidth, texHeight, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, 0, m_renderer.allocator, texImageAlloc);
    stbi_image_free(pixels);
    stagingBuffer.clear();
    texImage.clear();
    vmaFreeMemory(m_renderer.allocator, texImageAlloc);
    vmaFreeMemory(m_renderer.allocator, allocation);
}

vk::raii::Image Resources::createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, VmaAllocationCreateFlags createFlags, const VmaAllocator& allocator, VmaAllocation& allocation) {
    vk::ImageCreateInfo imageInfo{};
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent.width = static_cast<uint32_t>(width);
    imageInfo.extent.height = static_cast<uint32_t>(height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.usage = usage;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.tiling = tiling;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;
    imageInfo.samples = vk::SampleCountFlagBits::e1;

    vk::Image image{};
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.flags = createFlags;
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

    auto result = vmaCreateImage(allocator, reinterpret_cast<VkImageCreateInfo*>(&imageInfo), &allocInfo, reinterpret_cast<VkImage*>(&image), &allocation, nullptr);

    if (result != VK_SUCCESS) {
        throw std::runtime_error("image creation failed");
    }
    return vk::raii::Image{m_renderer.m_device, image};
    }
