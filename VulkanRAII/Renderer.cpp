#include "Renderer.h"

void Renderer::run() {
    initWindow();
    initVulkan();
    mainLoop();
}

vk::raii::PhysicalDevices Renderer::getPhyDevices() {
    vk::raii::PhysicalDevices devices{instance};

    return devices;
}

void Renderer::initWindow() {
    // telling glfw to not create an OpenGL context and bind it to the window
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(width, height, "hello Vulkan", nullptr, nullptr);
}

const vk::SurfaceKHR* Renderer::getSurface() {
    const vk::SurfaceKHR* surfacePtr{&(*surface)};
    return surfacePtr;
}

void Renderer::initVulkan() {
    createInstance();
    createSurface();
    device.createDevice(getPhyDevices(), getSurface());
    setupDebugCallback();
    listExtensionNames();
}

void Renderer::createInstance() {
    if (debug && !checkValidationLayersSupport())
        throw std::runtime_error("validation layers requested, but not available!");

    vk::ApplicationInfo appInfo{
        "hello triangle",
        1,
        "renderer",
        1,
        VK_API_VERSION_1_0};

    vk::InstanceCreateInfo createInfo{};
    createInfo.pApplicationInfo = &appInfo;
    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();
    if (debug) {
        createInfo.enabledLayerCount = validationLayers.size();
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    try {
        instance = context.createInstance(createInfo);
    } catch (vk::SystemError& err) {
        throw std::runtime_error("failed to create an instance");
    }
}

// functions to get all the required extensions for glfw to create a surface
std::vector<const char*> Renderer::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions,
        glfwExtensions + glfwExtensionCount);

    if (debug)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

bool Renderer::checkValidationLayersSupport() {
    uint32_t layerCount{};
    std::vector<vk::LayerProperties> availableLayers{
        context.enumerateInstanceLayerProperties()};
    // the loop will compare every string from the queried layers with our
    // required layer
    for (const char* layerName : validationLayers) {
        bool layerFound{false};
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound)
            return false;
    }
    return true;
}

void Renderer::listExtensionNames() {
    std::vector<vk::ExtensionProperties> extensions{
        context.enumerateInstanceExtensionProperties()};
    std::vector<vk::LayerProperties> layers{context.enumerateInstanceLayerProperties()};

    for (auto& extension : extensions)
        std::cout << extension.extensionName << '\n';
    for (auto& layer : layers)
        std::cout << layer.layerName << '\n';
}

void Renderer::mainLoop() {
    while (!glfwWindowShouldClose(window))
        glfwPollEvents();
}

void Renderer::createSurface() {
    VkSurfaceKHR c_surface{};
    if (glfwCreateWindowSurface(*instance, window, nullptr, &c_surface) != VK_SUCCESS)
        throw std::runtime_error("failed to create a surface!");
    surface = vk::raii::SurfaceKHR{
        instance, c_surface};
}

// I really dont know how it works but it works
void Renderer::setupDebugCallback() {
    if (!debug)
        return;

    auto createInfo = vk::DebugUtilsMessengerCreateInfoEXT(
        vk::DebugUtilsMessengerCreateFlagsEXT(),
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        debugCallback, nullptr);

    // NOTE: Vulkan-hpp has methods for this, but they trigger linking errors...
    // instance->createDebugUtilsMessengerEXT(createInfo);
    // instance->createDebugUtilsMessengerEXTUnique(createInfo);

    // NOTE: reinterpret_cast is also used by vulkan.hpp internally for all these
    // structs
    if (CreateDebugUtilsMessengerEXT(
            *instance,
            reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT*>(
                &createInfo),
            nullptr, &callback)
        != VK_SUCCESS)
        throw std::runtime_error("failed to set up debug callback!");
}

VKAPI_ATTR VkBool32 VKAPI_CALL Renderer::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

Renderer::~Renderer() {
    glfwDestroyWindow(window);
    glfwTerminate();
    
    if (debug)
        DestroyDebugUtilsMessengerEXT(*instance, callback, nullptr);
}

Renderer::Renderer() {
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
        return func(instance, pCreateInfo, pAllocator, pCallback);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
        func(instance, callback, pAllocator);
}
