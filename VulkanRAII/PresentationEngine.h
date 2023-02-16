#pragma once
class Renderer;
#include "commonIncludes.h"
class PresentationEngine {
  public:
    struct SwapChainCapablities {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentMode;
    };

    vk::raii::SurfaceKHR m_surface{nullptr};
    vk::raii::SwapchainKHR m_swapChain{nullptr};
    vk::Format swapChainImagesFormat{};
    vk::Extent2D swapChainExtent{};
    std::vector<vk::raii::ImageView> swapChainImageViews{};

    PresentationEngine(Renderer& renderer);
    void createSurface();
    void createSwapchain();
    void createImageViews();

  private:
    Renderer& m_renderer;
    SwapChainCapablities getSwapChainCapabilities();
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<vk::SurfaceFormatKHR> availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(
        const std::vector<vk::PresentModeKHR> availablePresentModes);
    vk::Extent2D chooseSwapExtend(const vk::SurfaceCapabilitiesKHR& capabilities);
};
