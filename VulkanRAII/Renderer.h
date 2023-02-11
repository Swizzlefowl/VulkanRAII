#ifndef RENDERER_H
#define RENDERER_H
#include "QueueFamilyIndices.h"
#include <exception>
#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <utility>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>

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

    GLFWwindow* window;
    const int width{1280};
    const int height{720};

    std::vector<const char*> validationLayers{"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkDebugUtilsMessengerEXT callback{};
    vk::raii::Context context{};
    vk::raii::Instance instance{nullptr};
    vk::raii::SurfaceKHR surface{nullptr};
    vk::raii::PhysicalDevices m_physicalDevices{nullptr};
    vk::raii::PhysicalDevice m_physicalDevice{nullptr};
    vk::raii::Device m_device{nullptr};

  public:
    Renderer();
    void run();
    vk::raii::PhysicalDevices getPhyDevices();
    ~Renderer();
    const vk::SurfaceKHR* getSurface();

  private:
    void initWindow();
    void initVulkan();
    void createInstance();
    void createSurface();
    void createDevice();
    void pickPhysicalDevice();
    bool isDeviceSuitable(vk::raii::PhysicalDevice device);
    bool checkDeviceExtensionSuppport(vk::raii::PhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(vk::raii::PhysicalDevice device);
    void mainLoop();

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
#endif