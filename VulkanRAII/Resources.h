#pragma once
#include "commonIncludes.h"

class Renderer;
class Resources {
  private:
    Renderer& m_renderer;
    void createCommandPools();
    void createCommandbuffer();
    void createSyncObjects();
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    void createBuffers(vk::raii::Buffer& buffer, vk::raii::DeviceMemory& memory, vk::DeviceSize size, vk::BufferUsageFlagBits usage);
    void mapMemory(vk::raii::DeviceMemory& memory, vk::DeviceSize size, const auto& vec);

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
    vk::raii::Buffer indexBuffer{nullptr};
    vk::raii::DeviceMemory indexBufferMemory{nullptr};
    void* colorPtr{nullptr};

    Resources(Renderer& renderer);
    void createframebuffers();
    ~Resources();
    void createResources();
};
