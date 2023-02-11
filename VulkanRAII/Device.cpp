#include "QueueFamilyIndices.h"
#include "Renderer.h"

void Device::createDevice(const vk::raii::PhysicalDevices& devices, const vk::SurfaceKHR* surf) {
    surface = surf;
    pickPhysicalDevice(devices);

    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{};
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
        indices.presentFamily.value()};
    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::PhysicalDeviceFeatures deviceFeatures{};

    vk::DeviceCreateInfo createInfo{};
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (debug) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }
    try {
        m_device = m_physicalDevice.createDevice(createInfo);
    }
    catch (vk::SystemError& err) {
        throw std::runtime_error("failed to create logical device");
    }
}

bool Device::checkDeviceExtensionSuppport(vk::raii::PhysicalDevice device) {
    std::vector<vk::ExtensionProperties> availableExtensions{
        device.enumerateDeviceExtensionProperties()};

    for (auto& ext : availableExtensions)

        std::cout << ext.extensionName << '\n';
    std::set<std::string> requiredExtensions{deviceExtensions.begin(),
        deviceExtensions.end()};

    for (const auto& extension : availableExtensions)
        requiredExtensions.erase(extension.extensionName);

    return requiredExtensions.empty();
}

Device::Device() {
}

QueueFamilyIndices Device::findQueueFamilies(vk::raii::PhysicalDevice device) {
    QueueFamilyIndices indices;
    std::vector<vk::QueueFamilyProperties> queueProperties{
        device.getQueueFamilyProperties()};
    vk::Bool32 presentSupport{false};

    int index{};
    for (auto& queueProperty : queueProperties) {
        if (queueProperty.queueFlags & vk::QueueFlagBits::eGraphics)
            indices.graphicsFamily = index;
        presentSupport = device.getSurfaceSupportKHR(index, *surface);
        if (presentSupport)
            indices.presentFamily = index;
        if (indices.isComplete())
            break;
        index++;
    }
    return indices;
}

void Device::pickPhysicalDevice(const vk::raii::PhysicalDevices& devices) {
    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            m_physicalDevice = device;
            std::cout << m_physicalDevice.getProperties().deviceName << '\n';
            break;
        }
    }
}

bool Device::isDeviceSuitable(const vk::raii::PhysicalDevice& device) {
    vk::PhysicalDeviceProperties deviceProperties{device.getProperties()};
    QueueFamilyIndices indices{
        findQueueFamilies(device)};
    bool extensionSupported{
        checkDeviceExtensionSuppport(device)};

    return deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu && extensionSupported;
}
