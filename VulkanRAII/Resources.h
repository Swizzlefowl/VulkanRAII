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
      class Mesh {
      public:
       Mesh(const VmaAllocator& allocator);
       ~Mesh();
        const VmaAllocator& allocator;
        vk::raii::Buffer vertexBuffer{nullptr};
        VmaAllocation vertexAlloc{nullptr};
        std::size_t verticesCount{};
        vk::raii::Buffer indexBuffer{nullptr};
        VmaAllocation indexAlloc{nullptr};
        std::uint32_t indicesCount{};
        vk::raii::Image image{nullptr};
        VmaAllocation imageAlloc{nullptr};
        vk::raii::ImageView imageView{nullptr};
        vk::raii::Sampler sampler{nullptr};
    };

      struct Vertex {
          glm::vec3 pos{};
          glm::vec3 color{};
          glm::vec2 texCoord{};
      };

    std::vector<vk::raii::Framebuffer> frambebuffers;
    vk::raii::CommandPool commandPool{nullptr};
    vk::raii::CommandBuffers commandBuffer{nullptr};
    vk::raii::Semaphore imageAvailableSemaphores{nullptr};
    vk::raii::Semaphore finishedRenderingSemaphores{nullptr};
    vk::raii::Fence inFlightFences{nullptr};
    vk::raii::Fence screenCaptureFence{nullptr};
    vk::raii::Buffer vertexBuffer{nullptr};
    vk::raii::DeviceMemory vertexBufferMemory{nullptr};
    vk::raii::Buffer indexBuffer{nullptr};
    vk::raii::DeviceMemory indexBufferMemory{nullptr};
    vk::raii::Buffer uniformBuffer{nullptr};
    vk::raii::Buffer uniformBuffer2{nullptr};
    vk::raii::DeviceMemory uniformBufferMemory{nullptr};
    vk::raii::DeviceMemory uniformBufferMemory2{nullptr};
    vk::raii::DescriptorPool descriptorPool{nullptr};
    std::vector<vk::raii::DescriptorSet> descriptorSet{};
    vk::Buffer textureBuffer{};
    vk::raii::Framebuffer blitFramebuffer{nullptr};
    Mesh cube;
    Mesh viking;
    Mesh atlasCube;
    VmaAllocation depthAlloc{nullptr};
    vk::raii::Image depthImage{nullptr};
    vk::raii::ImageView depthImageView{nullptr};
    vk::raii::Image texImage3{nullptr};
    VmaAllocation texImageAlloc3{nullptr};
    vk::raii::ImageView texImageView3{nullptr};
    vk::raii::Sampler texSampler3{nullptr};
    std::vector<vk::raii::DescriptorSet> skyDescriptorSet{};
    vk::raii::DescriptorSet computeDescriptorSet{nullptr};
    vk::raii::Image skyBoxImage{nullptr};
    VmaAllocation skyBoxImageAlloc{nullptr};
    vk::raii::ImageView skyBoxImageView{nullptr};
    vk::raii::Sampler skyBoxSampler{nullptr};
    void* colorPtr{nullptr};
    void* uboPtr{nullptr};
    void* uboPtr2{nullptr};
    std::vector<glm::vec3> instances{};
    vk::raii::Buffer instanceBuffer{nullptr};
    VmaAllocation instanceAlloc{nullptr};

    Resources(Renderer& renderer);
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    void createframebuffers();
    void createBlitFrameBuffer();
    ~Resources();
    void createResources();
    void createDescriptorPool();
    void allocateDescriptorSets();
    void allocateSkyDescriptorSet();
    void allocateComputeDescSet();
    vk::raii::Buffer createBuffer(vk::BufferUsageFlags usage, vk::DeviceSize size, VmaAllocationCreateFlags createFlags, VkMemoryPropertyFlags propertyFlags, VmaAllocation& allocation);
    void mapMemory(const VmaAllocator& allocator, const VmaAllocation& allocation, void* src, VkDeviceSize size);
    void* mapPersistentMemory(const VmaAllocator& allocator, const VmaAllocation& allocation, VkDeviceSize size);
    void loadImage(const std::string& imageName, vk::raii::Image& image, vk::raii::ImageView& imageView, VmaAllocation& imageAlloc, vk::raii::Sampler& sampler);
    vk::raii::Image createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, VmaAllocationCreateFlags createFlags, VkMemoryPropertyFlags propertyFlags, const VmaAllocator& allocator, VmaAllocation& allocation);
    vk::raii::CommandBuffer createSingleTimeCB();
    vk::raii::ImageView createImageView(const vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags);
    vk::raii::Sampler createSampler();
    void createDepthBuffer();
    void loadModel(const std::string& name, std::vector<Resources::Vertex>& vertices, std::vector<std::uint32_t>& indices, bool customUV = false);
    void createVertexBuffer(const VmaAllocator& allocator, vk::raii::Buffer& buffer, vk::BufferUsageFlags usage, VmaAllocation& alloc, void* src, vk::DeviceSize size);
    void copyBufferToImage(const vk::raii::CommandBuffer& commandBuffer, const vk::raii::Buffer& buffer, const vk::Image& image, uint32_t width, uint32_t height);
    void createMesh(const std::string& Modelname, const std::string& textureName, Mesh& mesh, bool customUV = false);
    void copyBuffer(vk::raii::CommandBuffer& cb, const vk::Buffer& srcBuffer, const vk::Buffer& dstBuffer, vk::DeviceSize size);
    void createSkyBox();
    void createInstanceData();
};
