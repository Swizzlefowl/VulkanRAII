#include "PresentationEngine.h"
#include "Renderer.h"
#include "Resources.h"

PresentationEngine::SwapChainCapablities PresentationEngine::getSwapChainCapabilities() {
    SwapChainCapablities swapChainSupport{
        .capabilities = m_renderer.m_physicalDevice.getSurfaceCapabilitiesKHR(*m_surface),
        .formats = m_renderer.m_physicalDevice.getSurfaceFormatsKHR(*m_surface),
        .presentMode = m_renderer.m_physicalDevice.getSurfacePresentModesKHR(*m_surface)};
    return swapChainSupport;
}

vk::SurfaceFormatKHR PresentationEngine::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> availableFormats) {
    for (const auto& availableFormat : availableFormats)
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            return availableFormat;
    return availableFormats[0];
}

vk::PresentModeKHR PresentationEngine::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> availablePresentModes) {
    for (const auto& presentMode : availablePresentModes)
        if (presentMode == vk::PresentModeKHR::eMailbox)
            return presentMode;
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D PresentationEngine::chooseSwapExtend(const vk::SurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(m_renderer.window, &width, &height);

        vk::Extent2D actualExtent{static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)};
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width);

        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height);

        return actualExtent;
    }
    return vk::Extent2D();
}

void PresentationEngine::createImageViews() {
    // we are creating temporary imageviews to eventually
    // move ownership to our imageviews
    // if you dont do it this way it will throw an exception when
    // recreating the swapchain

    std::vector<vk::Image> images = m_swapChain.getImages();
    std::vector<vk::raii::ImageView> imageviews{};
    for (auto& image : images) {
        vk::ImageViewCreateInfo createInfo{};
        createInfo.image = image;
        createInfo.format = swapChainImagesFormat;
        createInfo.viewType = vk::ImageViewType::e2D;
        vk::ComponentMapping mappings{
            vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity};
        createInfo.components = mappings;

        // base	MipmapLevel = 0, levelcount = 1, baseArrayLayer = 0, layerCount
        // =
        // 1
        vk::ImageSubresourceRange imageSubResource{vk::ImageAspectFlagBits::eColor,
            0, 1, 0, 1};
        createInfo.subresourceRange = imageSubResource;
        try {
            imageviews.push_back(m_renderer.m_device.createImageView(createInfo));
        } catch (vk::SystemError& err) {
            throw std::runtime_error("failed to create image views");
        }
    }
    swapChainImageViews = std::move(imageviews);
}

PresentationEngine::PresentationEngine(Renderer& renderer)
    : m_renderer{renderer} {
}

void PresentationEngine::createSurface() {
    // you have to give glfwCreatewindowSurface a vkSurface handle
    VkSurfaceKHR c_surface{};
    if (glfwCreateWindowSurface(*m_renderer.m_instance, m_renderer.window, nullptr, &c_surface) != VK_SUCCESS)
        throw std::runtime_error("failed to create a surface!");
    // luckily you can create a vk::raii::surface with a vkSurface
    m_surface = vk::raii::SurfaceKHR{
        m_renderer.m_instance, c_surface};
}

void PresentationEngine::createSwapchain() {
    SwapChainCapablities swapchainCapabilites{getSwapChainCapabilities()};
    vk::SurfaceFormatKHR surfaceFormat{chooseSwapSurfaceFormat(swapchainCapabilites.formats)};
    vk::PresentModeKHR presentMode{chooseSwapPresentMode(swapchainCapabilites.presentMode)};
    vk::Extent2D extent{chooseSwapExtend(swapchainCapabilites.capabilities)};
    uint32_t imageCount{swapchainCapabilites.capabilities.minImageCount + 1};

    if (swapchainCapabilites.capabilities.maxImageCount > 0 && imageCount > swapchainCapabilites.capabilities.maxImageCount)
        imageCount = swapchainCapabilites.capabilities.maxImageCount;

    vk::SwapchainCreateInfoKHR createInfo{};
    createInfo.surface = *m_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;

    auto queueFamilyIndex = m_renderer.getQueueFamilyIndex();
    createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;

    createInfo.preTransform = swapchainCapabilites.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    try {
        m_swapChain = m_renderer.m_device.createSwapchainKHR(createInfo);
    } catch (vk::Error& err) {
        throw std::runtime_error("failed to create swap chain!");
    }
    swapChainImagesFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void PresentationEngine::createSwapchainImages() {
    swapChainImages = m_swapChain.getImages();
}

void PresentationEngine::createBlitImage() {
    vk::ImageCreateInfo imageInfo{};
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent.width = swapChainExtent.width;
    imageInfo.extent.height = swapChainExtent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = swapChainImagesFormat;
    imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;
    imageInfo.samples = vk::SampleCountFlagBits::e1;

    blitImage = m_renderer.m_device.createImage(imageInfo);

     vk::MemoryRequirements memRequirements;
    memRequirements = blitImage.getMemoryRequirements();

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = m_renderer.pResources->findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
    blitImageMemory = m_renderer.m_device.allocateMemory(allocInfo);

    blitImage.bindMemory(*blitImageMemory, 0);
}

void PresentationEngine::createBlitImageView() {
    vk::ImageViewCreateInfo createInfo{};
    createInfo.image = *blitImage;
    createInfo.format = swapChainImagesFormat;
    createInfo.viewType = vk::ImageViewType::e2D;
    vk::ComponentMapping mappings{
        vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity};
    createInfo.components = mappings;

    // base	MipmapLevel = 0, levelcount = 1, baseArrayLayer = 0, layerCount
    // =
    // 1
    vk::ImageSubresourceRange imageSubResource{vk::ImageAspectFlagBits::eColor,
        0, 1, 0, 1};
    createInfo.subresourceRange = imageSubResource;

    blitImageViews = m_renderer.m_device.createImageView(createInfo);
}
