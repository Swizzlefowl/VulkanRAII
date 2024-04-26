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
    vk::raii::Buffer vertexBuffer{nullptr};
    vk::raii::DeviceMemory vertexBufferMemory{nullptr};
    vk::raii::Buffer indexBuffer{nullptr};
    vk::raii::DeviceMemory indexBufferMemory{nullptr};
    vk::raii::Buffer uniformBuffer{nullptr};
    vk::raii::DeviceMemory uniformBufferMemory{nullptr};
    vk::raii::DescriptorPool descriptorPool{nullptr};
    std::vector<vk::raii::DescriptorSet> descriptorSet{};
    vk::Buffer textureBuffer{};
    VmaAllocation texallocation;
    vk::raii::Framebuffer blitFramebuffer{nullptr};
    vk::raii::Image texImage{nullptr};
    VmaAllocation texImageAlloc{nullptr};
    vk::raii::ImageView texImageView{nullptr};
    vk::raii::Sampler texSampler{nullptr};
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
    vk::raii::Buffer createBuffer(vk::BufferUsageFlags usage, vk::DeviceSize size, VmaAllocationCreateFlags createFlags, VmaAllocation& allocation);
    void mapMemory(const VmaAllocator& allocator, const VmaAllocation& allocation, void* src, VkDeviceSize size);
    void* mapPersistentMemory(const VmaAllocator& allocator, const VmaAllocation& allocation, VkDeviceSize size);
    void loadImage();
    vk::raii::Image createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, VmaAllocationCreateFlags createFlags, const VmaAllocator& allocator, VmaAllocation& allocation);
    vk::raii::CommandBuffer createSingleTimeCB();
    vk::raii::ImageView createImageView(const vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags);
    vk::raii::Sampler createSampler();
    void copyBufferToImage(const vk::raii::CommandBuffer& commandBuffer, const vk::raii::Buffer& buffer, const vk::Image& image, uint32_t width, uint32_t height);

};
