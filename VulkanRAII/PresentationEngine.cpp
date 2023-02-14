#include "PresentationEngine.h"
#include "Renderer.h"

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
    std::vector<vk::Image> images = m_swapChain.getImages();

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
            swapChainImageViews.push_back(m_renderer.m_device.createImageView(createInfo));
        } catch (vk::SystemError& err) {
            throw std::runtime_error("failed to create image views");
        }
    }
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
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

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
