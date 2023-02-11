#ifndef DEVICE_H
#define DEVICE_H
#include "Renderer.h"
class Device {
  private:
#ifdef NDEBUG
    const bool debug = false;
#else
    const bool debug = true;
#endif //

    vk::raii::PhysicalDevice m_physicalDevice{nullptr};
    vk::raii::Device m_device{nullptr};
    const vk::SurfaceKHR* surface{};
    std::vector<const char*> validationLayers{"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  public:
    Device();
    void createDevice(const vk::raii::PhysicalDevices& devices, const vk::SurfaceKHR*);

  private:
    void pickPhysicalDevice(const vk::raii::PhysicalDevices& devices);
    bool isDeviceSuitable(const vk::raii::PhysicalDevice& device);
    bool checkDeviceExtensionSuppport(vk::raii::PhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(vk::raii::PhysicalDevice device);
};
#endif
