#pragma once
#include "commonIncludes.h"
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
    const int width{1280};
    const int height{720};
    // just a note to myself member variables are destroyed at the reverse order
    //  of declaration
    vk::raii::Context m_context{};
    vk::raii::Instance m_instance{nullptr};
    vk::raii::PhysicalDevices m_physicalDevices{nullptr};
    vk::raii::PhysicalDevice m_physicalDevice{nullptr};
    vk::raii::Device m_device{nullptr};
    std::vector<const char*> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    //  member variables for debugging
    std::vector<const char*> validationLayers{"VK_LAYER_KHRONOS_validation"};
    VkDebugUtilsMessengerEXT callback{};
    vk::raii::Queue m_queue{nullptr};
    PresentationEngine* pEngine{nullptr};
    Graphics* pGraphics{nullptr};
    Resources* pResources{nullptr};

  public:
    Renderer();
    void run(PresentationEngine* engine, Graphics* Graphics, Resources* resources);
    ~Renderer();

  private:
    void initWindow();
    void initVulkan();
    void createInstance();
    uint32_t getQueueFamilyIndex();
    void createDevice();
    void pickPhysicalDevice();
    bool isDeviceSuitable(vk::raii::PhysicalDevice device, vk::PhysicalDeviceType deviceType);
    bool checkDeviceExtensionSuppport(vk::raii::PhysicalDevice device);
    void mainLoop();
    void recordCommandbuffer(vk::raii::CommandBuffer& commandBuffer, uint32_t imageIndex);
    void drawFrame();

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
