#ifndef RENDERER_H
#define RENDERER_H
#include <exception>
#include <iostream>
#include <vector>
#include <optional>
#include <memory>
#include <set>
#include <utility>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>
#include "QueueFamilyIndices.h"
#include "Device.h"

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

    Device device;
    std::vector<const char*> validationLayers{"VK_LAYER_KHRONOS_validation"};
    VkDebugUtilsMessengerEXT callback{};
    vk::raii::Context context{};
    vk::raii::Instance instance{nullptr};
    vk::raii::SurfaceKHR surface{nullptr};

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