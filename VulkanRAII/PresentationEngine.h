#pragma once
class Renderer;
#include "commonIncludes.h"
class PresentationEngine {
  private:
    Renderer& m_renderer;
    
  public:
    vk::raii::SurfaceKHR m_surface{nullptr};
    vk::raii::SwapchainKHR swapChain{nullptr};
    PresentationEngine(Renderer& renderer);
    void createSurface();
};
