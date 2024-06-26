#pragma once
#include "commonIncludes.h"
#include "vma/vk_mem_alloc.h"
#include <random>
// clang formats puts the glfw include above the vulkan include which breaks the program
// remeber to put it in the correct place after formatting

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pCallback);

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
    VkDebugUtilsMessengerEXT callback,
    const VkAllocationCallbacks* pAllocator);

class Renderer {
  private:
#ifdef NDEBUG
    const bool debug = false;
#else
    const bool debug = true;
#endif //
    friend class PresentationEngine;
    friend class Graphics;
    friend class Resources;
    GLFWwindow* window;
    const int width{1920};
    const int height{1080};
    // just a note to myself member variables are destroyed at the reverse order
    //  of declaration

    // arrays for triangle attributes
    struct Vertex {
        glm::vec3 pos{};
        glm::vec3 color{};
        glm::vec2 texCoord{};
    };


    std::vector<Vertex> vertices{};
    std::vector<uint32_t> indices{};

    struct MeshPushConstants {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };

    std::vector<std::string> faces {
        "right.jpg",
        "left.jpg",
        "top.jpg",
        "bottom.jpg",
        "front.jpg",
        "back.jpg"};

    std::vector<std::string> texNames{
        "viking_room.png",
        "statue.jpg"
        "kenergy.jpg"};

    vk::raii::Context m_context{};
    vk::raii::Instance m_instance{nullptr};
    vk::raii::PhysicalDevices m_physicalDevices{nullptr};
    vk::raii::PhysicalDevice m_physicalDevice{nullptr};
    vk::raii::Device m_device{nullptr};
    std::vector<const char*> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME};
    VmaAllocator allocator{};
    //  member variables for debugging
    std::vector<const char*> validationLayers{"VK_LAYER_KHRONOS_validation"};
    VkDebugUtilsMessengerEXT callback{};
    vk::raii::Queue m_queue{nullptr};
    PresentationEngine* pEngine{nullptr};
    Graphics* pGraphics{nullptr};
    Resources* pResources{nullptr};
    std::mt19937_64 mt{};
    bool framebufferResized{false};
    std::vector<std::string> args{};
    std::string modelName{};
  public:
    enum Colors {
        Red,
        Green,
        Blue,
        RG,
        RGB,
        GB,
        RB,
        NoColor
    };

    Renderer(const std::vector<std::string>& args);
    void run(PresentationEngine* engine, Graphics* Graphics, Resources* resources);
    ~Renderer();

  private:
    void initWindow();
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    void initVulkan();
    void createInstance();
    uint32_t getQueueFamilyIndex();
    void createDevice();
    void pickPhysicalDevice();
    bool isDeviceSuitable(vk::raii::PhysicalDevice device, vk::PhysicalDeviceType deviceType);
    bool checkDeviceExtensionSuppport(vk::raii::PhysicalDevice device);
    void createAllocator();
    void mainLoop();
    void recordCommandbuffer(vk::raii::CommandBuffer& commandBuffer, uint32_t imageIndex);
    void recordComputeCB(vk::raii::CommandBuffer& commandBuffer, uint32_t imageIndex);
    void createRandomNumberGenerator();
    void changeColor(Colors color);
    void drawFrame();
    void cleanupSwapchain();
    void recreateSwapchain();
    void transitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::raii::CommandBuffer& commandBuffer, const vk::Image& image, vk::ImageAspectFlags aspect, bool isCubeMap = false);
    Colors checkUserInput();
    int getUserInput();
    void screenCapture();

    // functions for debugging
    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayersSupport();
    void listExtensionNames();
    void setupDebugCallback();
    static VKAPI_ATTR VkBool32 VKAPI_CALL
    debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
};
