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

}

void Resources::createResources() {
    createframebuffers();
    createCommandPools();
    createCommandbuffer();
    createSyncObjects();
    vk::DeviceSize vertexSize = static_cast<vk::DeviceSize>(sizeof(m_renderer.vertices[0]) * m_renderer.vertices.size());
    vk::DeviceSize indexSize = static_cast<vk::DeviceSize>(sizeof(m_renderer.indices[0]) * m_renderer.indices.size());
    vk::DeviceSize uboSize = static_cast<vk::DeviceSize>(sizeof(Renderer::MeshPushConstants));
    createBuffers(vertexBuffer, vertexBufferMemory, vertexSize, vk::BufferUsageFlagBits::eVertexBuffer);
    createBuffers(indexBuffer, indexBufferMemory, indexSize, vk::BufferUsageFlagBits::eIndexBuffer);
    createBuffers(uniformBuffer, uniformBufferMemory, uboSize, vk::BufferUsageFlagBits::eUniformBuffer);
    auto vertexPtr = vertexBufferMemory.mapMemory(0, vertexSize);
    memcpy(vertexPtr, m_renderer.vertices.data(), vertexSize);
    mapMemory(indexBufferMemory, indexSize, m_renderer.indices);
    uboPtr = uniformBufferMemory.mapMemory(0, uboSize);
    vertexBufferMemory.unmapMemory();
    indexBufferMemory.unmapMemory();
}

void Resources::createDescriptorPool() {
    std::array<vk::DescriptorPoolSize, 2> poolSize{};
    poolSize[0].type = vk::DescriptorType::eUniformBuffer;
    poolSize[0].descriptorCount = 1;

     poolSize[1].type = vk::DescriptorType::eCombinedImageSampler;
     poolSize[1].descriptorCount = 3;
    vk::DescriptorPoolCreateInfo createInfo{};
    createInfo.poolSizeCount = poolSize.size();
    createInfo.pPoolSizes = poolSize.data();
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

    vk::DescriptorImageInfo imageInfo{};
    imageInfo.imageView = *texImageView;
    imageInfo.sampler = *texSampler;
    imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    vk::DescriptorImageInfo imageInfo2{};
    imageInfo2.imageView = *texImageView2;
    imageInfo2.sampler = *texSampler2;
    imageInfo2.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    vk::DescriptorImageInfo imageInfo3{};
    imageInfo3.imageView = *texImageView3;
    imageInfo3.sampler = *texSampler3;
    imageInfo3.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    vk::DescriptorImageInfo imageInfos[3] = {imageInfo, imageInfo2, imageInfo3};
    std::array<vk::WriteDescriptorSet, 2> descriptorWrite{};
    descriptorWrite[0].dstSet = *descriptorSet[0];
    descriptorWrite[0].dstBinding = 0;
    descriptorWrite[0].dstArrayElement = 0;
    descriptorWrite[0].descriptorType = vk::DescriptorType::eUniformBuffer;
    descriptorWrite[0].descriptorCount = 1;
    descriptorWrite[0].pBufferInfo = &bufferInfo;
    
    // dstArray refers to an array of descriptors pointing to an array of buffers/samplers bound that that sets binding slot
    // dstArrayElement is the index of that element inside the array
    descriptorWrite[1].dstSet = *descriptorSet[0];
    descriptorWrite[1].dstBinding = 1;
    descriptorWrite[1].dstArrayElement = 0;
    descriptorWrite[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    descriptorWrite[1].descriptorCount = 3;
    descriptorWrite[1].pImageInfo = imageInfos;

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

void Resources::loadImage(const std::string& imageName, vk::raii::Image& image, vk::raii::ImageView& imageView, VmaAllocation& imageAlloc, vk::raii::Sampler& sampler) {
    int texWidth{};
    int texHeight{};
    int texChannels{};
    stbi_uc* pixels{nullptr};
    pixels = stbi_load(imageName.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    vk::DeviceSize imageSize{static_cast<vk::DeviceSize>(texWidth * texHeight * 4)};

    if (!pixels)
        throw std::runtime_error("failed to load image!");

    vk::raii::Buffer stagingBuffer{nullptr};
    VmaAllocation allocation{nullptr};
    stagingBuffer = createBuffer(vk::BufferUsageFlagBits::eTransferSrc, imageSize, VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, allocation);
    mapMemory(m_renderer.allocator, allocation, pixels, imageSize);

    image = createImage(texWidth, texHeight, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, 0, m_renderer.allocator, imageAlloc);

    auto commandBuffer{createSingleTimeCB()};
    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    commandBuffer.begin(beginInfo);

    m_renderer.transitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, commandBuffer, *image, vk::ImageAspectFlagBits::eColor);
    copyBufferToImage(commandBuffer, stagingBuffer, *image, static_cast<std::uint32_t>(texWidth), static_cast<std::uint32_t>(texHeight));
    m_renderer.transitionImageLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, commandBuffer, *image, vk::ImageAspectFlagBits::eColor);

   commandBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &*commandBuffer;
    m_renderer.m_queue.submit(submitInfo);
    m_renderer.m_queue.waitIdle();

    imageView = createImageView(*image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
    sampler = createSampler();
    commandBuffer.clear();
    stbi_image_free(pixels);
    stagingBuffer.clear();
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

vk::raii::CommandBuffer Resources::createSingleTimeCB() {
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;
    allocInfo.commandPool = *commandPool;

    vk::raii::CommandBuffer commandBuffer{nullptr};
    commandBuffer =  std::move(m_renderer.m_device.allocateCommandBuffers(allocInfo)[0]);
    return commandBuffer;
}

vk::raii::ImageView Resources::createImageView(const vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags) {
    vk::ImageViewCreateInfo createInfo{};
    createInfo.image = image;
    createInfo.viewType = vk::ImageViewType::e2D;
    createInfo.format = format;

    vk::ComponentMapping mappings{
        vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity};
    createInfo.components = mappings;

    // base	MipmapLevel = 0, levelcount = 1, baseArrayLayer = 0, layerCount
    // =
    // 1
    vk::ImageSubresourceRange imageSubResource{aspectFlags,
        0, 1, 0, 1};
    createInfo.subresourceRange = imageSubResource;

    return m_renderer.m_device.createImageView(createInfo);
}

vk::raii::Sampler Resources::createSampler() {
    vk::PhysicalDeviceProperties deviceProperties = m_renderer.m_physicalDevice.getProperties();
    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = vk::CompareOp::eAlways;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    return m_renderer.m_device.createSampler(samplerInfo);
}

void Resources::createDepthBuffer() {
    depthImage = createImage(m_renderer.pEngine->swapChainExtent.width, m_renderer.pEngine->swapChainExtent.width, vk::Format::eD32Sfloat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferDst, VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, m_renderer.allocator, depthAlloc);
    depthImageView = createImageView(*depthImage, vk::Format::eD32Sfloat, vk::ImageAspectFlagBits::eDepth);
    auto cb = createSingleTimeCB();
    
    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    cb.begin(beginInfo);

    m_renderer.transitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal, cb, *depthImage, vk::ImageAspectFlagBits::eDepth);
    cb.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &*cb;
    m_renderer.m_queue.submit(submitInfo);
    m_renderer.m_queue.waitIdle();

}

void Resources::copyBufferToImage(const vk::raii::CommandBuffer& commandBuffer, const vk::raii::Buffer& buffer, const vk::Image& image, uint32_t width, uint32_t height) {
    vk::BufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = vk::Offset3D{0, 0, 0};
    region.imageExtent = vk::Extent3D{
        width,
        height,
        1};

    commandBuffer.copyBufferToImage(*buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
}
