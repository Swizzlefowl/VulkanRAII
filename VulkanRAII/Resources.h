#pragma once
#include "commonIncludes.h"
#include "vma/vk_mem_alloc.h"
class Renderer;
class Resources {
  private:
    Renderer& m_renderer;
    void createCommandPools();
    void createCommandbuffer();
    void createSyncObjects();
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
    vk::raii::Buffer uniformBuffer{nullptr};
    vk::raii::DeviceMemory uniformBufferMemory{nullptr};
    vk::raii::DescriptorPool descriptorPool{nullptr};
    std::vector<vk::raii::DescriptorSet> descriptorSet{};
    vk::Buffer textureBuffer{};
    //VmaAllocation texBufferAllocation;
    vk::raii::Framebuffer blitFramebuffer{nullptr};
    void* colorPtr{nullptr};
    void* uboPtr{nullptr};

    Resources(Renderer& renderer);
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    void createframebuffers();
    void createBlitFrameBuffer();
    ~Resources();
    void createResources();
    void createDescriptorPool();
    void allocateDescriptorSets();
    void createBuffer(vk::Buffer& buffer, vk::BufferUsageFlags usage, vk::DeviceSize size, VmaAllocationCreateFlags createFlags, VmaAllocation& allocation);
    void mapMemory(const VmaAllocator& allocator, const VmaAllocation& allocation, void* src, VkDeviceSize size);
    void* mapPersistentMemory(const VmaAllocator& allocator, const VmaAllocation& allocation, VkDeviceSize size);
    void loadImage();
};
