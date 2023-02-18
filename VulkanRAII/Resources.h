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
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    void createBuffers(vk::raii::Buffer& buffer, vk::raii::DeviceMemory& memory, vk::DeviceSize size);
    void mapMemory(vk::raii::DeviceMemory& memory, vk::DeviceSize size, const std::vector<glm::vec3>& vec);

  public:
    std::vector<vk::raii::Framebuffer> frambebuffers;
    vk::raii::CommandPool commandPool{nullptr};
    vk::raii::CommandBuffers commandBuffer{nullptr};
    vk::raii::Semaphore imageAvailableSemaphores{nullptr};
    vk::raii::Semaphore finishedRenderingSemaphores{nullptr};
    vk::raii::Fence inFlightFences{nullptr};
    vk::raii::Buffer posBuffer{nullptr};
    vk::raii::DeviceMemory posBufferMemory{nullptr};
    vk::raii::Buffer colorBuffer{nullptr};
    vk::raii::DeviceMemory colorBufferMemory{nullptr};

    Resources(Renderer& renderer);
    void createResources();
};
