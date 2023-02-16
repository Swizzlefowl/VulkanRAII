#pragma once
#include "commonIncludes.h"

class Renderer;
class Resources {
  private:
    Renderer& m_renderer;
    void createframebuffers();
    void createCommandPools();
    void createCommandbuffer();
    void createSyncObjects();

  public:
    std::vector<vk::raii::Framebuffer> frambebuffers;
    vk::raii::CommandPool commandPool{nullptr};
    vk::raii::CommandBuffers commandBuffer{nullptr};
    vk::raii::Semaphore imageAvailableSemaphores{nullptr};
    vk::raii::Semaphore finishedRenderingSemaphores{nullptr};
    vk::raii::Fence inFlightFences{nullptr};

    Resources(Renderer& renderer);
    void createResources();
};
