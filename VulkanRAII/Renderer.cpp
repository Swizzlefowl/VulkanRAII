#include "Renderer.h"
#include "Graphics.h"
#include "PresentationEngine.h"
#include "Resources.h"

void Renderer::run(PresentationEngine* engine, Graphics* Graphics, Resources* resources) {
    pEngine = engine;
    pGraphics = Graphics;
    pResources = resources;
    createRandomNumberGenerator();
    initWindow();
    initVulkan();
    mainLoop();
}

void Renderer::initWindow() {
    // telling glfw to not create an OpenGL context and bind it to the window
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(width, height, "hello Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void Renderer::initVulkan() {
    createInstance();
    setupDebugCallback();
    pEngine->createSurface();
    createDevice();
    pEngine->createSwapchain();
    pEngine->createImageViews();
    pGraphics->createRenderPass();
    pGraphics->createGraphicsPipeline();
    pResources->createResources();
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
        m_instance = m_context.createInstance(createInfo);
    } catch (vk::SystemError& err) {
        throw std::runtime_error("failed to create an instance");
    }
}

uint32_t Renderer::getQueueFamilyIndex() {
    std::vector<vk::QueueFamilyProperties> queueFamilies{
        m_physicalDevice.getQueueFamilyProperties()};

    uint32_t queueFamilyIndex{0};
    for (auto& queueFamily : queueFamilies)
        // remember to do a bitwise and operation
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics && m_physicalDevice.getSurfaceSupportKHR(queueFamilyIndex, *pEngine->m_surface))
            break;
        else
            queueFamilyIndex++;
    return queueFamilyIndex;
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

void Renderer::createDevice() {
    m_physicalDevices = vk::raii::PhysicalDevices(m_instance);
    pickPhysicalDevice();
    auto queueFamilyIndex = getQueueFamilyIndex();
    float queuePriority = 1.0f;

    vk::DeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    vk::PhysicalDeviceFeatures deviceFeatures{};
    vk::DeviceCreateInfo createInfo{};
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
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
    } catch (vk::SystemError& err) {
        throw std::runtime_error("failed to create logical device");
    }
    m_queue = m_device.getQueue(queueFamilyIndex, 0);
}

bool Renderer::checkValidationLayersSupport() {
    uint32_t layerCount{};
    std::vector<vk::LayerProperties> availableLayers{
        m_context.enumerateInstanceLayerProperties()};
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
        m_context.enumerateInstanceExtensionProperties()};
    std::vector<vk::LayerProperties> layers{m_context.enumerateInstanceLayerProperties()};

    for (auto& extension : extensions)
        std::cout << extension.extensionName << '\n';
    for (auto& layer : layers)
        std::cout << layer.layerName << '\n';
}

void Renderer::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        changeColor(checkUserInput());
        drawFrame();
    }
    m_device.waitIdle();
}

void Renderer::recordCommandbuffer(vk::raii::CommandBuffer& commandBuffer, uint32_t imageIndex) {
    vk::CommandBufferBeginInfo beginInfo{};
    commandBuffer.begin(beginInfo);

    vk::RenderPassBeginInfo renderPassInfo{};
    renderPassInfo.renderPass = *pGraphics->renderPass;
    renderPassInfo.framebuffer = *pResources->frambebuffers[imageIndex];
    renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
    renderPassInfo.renderArea.extent = pEngine->swapChainExtent;

    vk::ClearValue clearColor{{0.0f, 0.0f, 0.0f, 1.0f}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    std::vector<vk::Buffer> buffers{*pResources->posBuffer, *pResources->colorBuffer};
    std::vector<vk::DeviceSize> offsets{0, 0};

    commandBuffer.bindVertexBuffers(0, buffers, offsets);
    commandBuffer.bindIndexBuffer(*pResources->indexBuffer, 0, vk::IndexType::eUint16);
    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pGraphics->graphicsPipeline);

    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(pEngine->swapChainExtent.width);
    viewport.height = static_cast<float>(pEngine->swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    commandBuffer.setViewport(0, viewport);

    vk::Rect2D scissor{};
    scissor.offset = vk::Offset2D{0, 0};
    scissor.extent = pEngine->swapChainExtent;

    glm::mat4 trans = glm::mat4(1.0f);
    trans = glm::rotate(trans, static_cast<float>(glfwGetTime()), glm::vec3(0.0f, 0.0f, 1.0f));

    MeshPushConstants mesh{};
    mesh.render_matrix = trans;
    std::vector<MeshPushConstants> meshes{mesh};
    commandBuffer.setScissor(0, scissor);
    commandBuffer.pushConstants<MeshPushConstants>(*pGraphics->pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, meshes);
    commandBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);
    commandBuffer.endRenderPass();

    try {
        commandBuffer.end();
    } catch (vk::SystemError err) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void Renderer::createRandomNumberGenerator() {
    std::random_device rd{};
    std::seed_seq seq{
        rd.max(), rd.max(), rd.max(), rd.max(),
        rd.max(), rd.max(), rd.max(), rd.max(), rd.max(), rd.max(), rd.max()};
    mt = std::mt19937_64{seq};
}

void Renderer::changeColor(Colors color) {
    m_device.waitIdle();

    std::vector<glm::vec3> colorValue = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 1.0f}};
    vk::DeviceSize size = sizeof(colorValue[0]) * colorValue.size();
    std::uniform_real_distribution<float> dst{0.0f, 1.0f};

    // std::cout << "eneter a number\n";
    // std::cin >> num;
    switch (color) {
        case Red:
            for (auto& attribute : colorValue) {
                attribute.r = 1.0f;
                attribute.g = 0.0f;
                attribute.b = 0.0f;
            }
            break;
        case Green:
            for (auto& attribute : colorValue) {
                attribute.r = 0.0f;
                attribute.g = 1.0f;
                attribute.b = 0.0f;
            }
            break;
        case Blue:
            for (auto& attribute : colorValue) {
                attribute.r = 0.0f;
                attribute.g = 0.0f;
                attribute.b = 1.0f;
            }
            break;
        case RGB:
            for (auto& attribute : colorValue) {
                attribute.r = dst(mt);
                attribute.g = dst(mt);
                attribute.b = dst(mt);
            }
            break;
        case RG:
            for (auto& attribute : colorValue) {
                attribute.r = 1.0f;
                attribute.g = 1.0f;
                attribute.b = 0.0f;
            }
            break;
        case GB:
            for (auto& attribute : colorValue) {
                attribute.r = 0.0f;
                attribute.g = 1.0f;
                attribute.b = 1.0f;
            }
            break;
        case NoColor:
            return;
        default:
            break;
    }
    memcpy(pResources->colorPtr, colorValue.data(), static_cast<size_t>(size));
}

void Renderer::drawFrame() {
    m_device.waitForFences(*pResources->inFlightFences, VK_TRUE, UINT64_MAX);

    vk::Result result;
    uint32_t imageIndex{};

    // unfortunately vk raii seems to throw an exception if you cant present
    //  or aquire images anymore because the surface is incompatible
    //  so the try catch blocks are necessary to successfully recreate
    //  the swapchain
    try {
        std::tie(result, imageIndex) = pEngine->m_swapChain.acquireNextImage(UINT64_MAX,
            *pResources->imageAvailableSemaphores);
    } catch (vk::Error& err) {
        std::cout << "recreate the swap chain\n";
        recreateSwapchain();
        return;
    }

    m_device.resetFences(*pResources->inFlightFences);

    pResources->commandBuffer[0].reset();
    recordCommandbuffer(pResources->commandBuffer[0], imageIndex);

    vk::SubmitInfo submitInfo{};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &(*pResources->imageAvailableSemaphores);
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &(*pResources->commandBuffer[0]);
    vk::PipelineStageFlags waitStages{vk::PipelineStageFlagBits::eColorAttachmentOutput};
    submitInfo.pWaitDstStageMask = &waitStages;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &(*pResources->finishedRenderingSemaphores);
    m_queue.submit(submitInfo, *pResources->inFlightFences);

    vk::PresentInfoKHR presentInfo{};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &(*pResources->finishedRenderingSemaphores);
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &(*pEngine->m_swapChain);
    presentInfo.pResults = nullptr;

    try {
        result = m_queue.presentKHR(presentInfo);
    } catch (vk::Error& err) {
        std::cout << "recreate the swapchain\n";
        recreateSwapchain();
    }
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
            *m_instance,
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

void Renderer::recreateSwapchain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
    try {
        vkDeviceWaitIdle(*m_device);
        cleanupSwapchain();
        pEngine->createSwapchain();
        pEngine->createImageViews();
        pResources->createframebuffers();
    } catch (vk::Error& err) {
        throw("failed to recreate swapchainImage!");
    }
}

void Renderer::cleanupSwapchain() {
    for (auto& framebuffer : pResources->frambebuffers)
        framebuffer.~Framebuffer();
    for (auto& imageViews : pEngine->swapChainImageViews)
        imageViews.~ImageView();
    pEngine->m_swapChain.~SwapchainKHR();
}

Renderer::Colors Renderer::checkUserInput() {
    std::array<int, 6> keys{};
    keys[0] = glfwGetKey(window, GLFW_KEY_1);
    keys[1] = glfwGetKey(window, GLFW_KEY_2);
    keys[2] = glfwGetKey(window, GLFW_KEY_3);
    keys[3] = glfwGetKey(window, GLFW_KEY_4);
    keys[4] = glfwGetKey(window, GLFW_KEY_5);
    keys[5] = glfwGetKey(window, GLFW_KEY_6);
    if (keys[0] == GLFW_PRESS)
        return Red;
    if (keys[1] == GLFW_PRESS)
        return Green;
    if (keys[2] == GLFW_PRESS)
        return Blue;
    if (keys[3] == GLFW_PRESS)
        return RGB;
    if (keys[4] == GLFW_PRESS)
        return RG;
    if (keys[5] == GLFW_PRESS)
        return GB;
    return NoColor;
}

int Renderer::getUserInput() {
    int getUserDevice{};

    while (true) {
        // use \ for newline character intead of / this

        std::cout << "Select your preferred GPU\n";
        std::cin >> getUserDevice;
        if (!std::cin || getUserDevice <= 0 || getUserDevice > 2) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cerr << "invalid number, please select again\n";
        } else
            return getUserDevice;
    }
}

void Renderer::pickPhysicalDevice() {
    if (m_physicalDevices.size() == 1) {
        m_physicalDevice = m_physicalDevices[0];
        return;
    }
    vk::PhysicalDeviceType deviceType{};
    auto getUserDevice{getUserInput()};

    if (getUserDevice == 1)
        deviceType = vk::PhysicalDeviceType::eIntegratedGpu;
    if (getUserDevice == 2)
        deviceType = vk::PhysicalDeviceType::eDiscreteGpu;

    for (const auto& device : m_physicalDevices) {
        if (isDeviceSuitable(device, deviceType)) {
            m_physicalDevice = device;
            std::cout << m_physicalDevice.getProperties().deviceName << '\n';
            break;
        }
    }
}

bool Renderer::checkDeviceExtensionSuppport(vk::raii::PhysicalDevice device) {
    std::vector<vk::ExtensionProperties> availableExtensions{
        device.enumerateDeviceExtensionProperties()};
    for (auto& extension : availableExtensions)
        std::cout << extension.extensionName << '\n';

    std::set<std::string> requiredExtensions{deviceExtensions.begin(), deviceExtensions.end()};
    for (const auto& extension : availableExtensions)
        requiredExtensions.erase(extension.extensionName);

    return requiredExtensions.empty();
}

void Renderer::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

bool Renderer::isDeviceSuitable(vk::raii::PhysicalDevice device, vk::PhysicalDeviceType deviceType) {
    vk::PhysicalDeviceProperties deviceProperties{device.getProperties()};
    bool extensionSupported{
        checkDeviceExtensionSuppport(device)};

    return deviceProperties.deviceType == deviceType && extensionSupported;
}

Renderer::~Renderer() {
    pEngine->m_swapChain.clear();
    m_physicalDevice.clear();
    m_physicalDevices.clear();
    m_device.clear();
    pEngine->m_surface.clear();
    if (debug)
        DestroyDebugUtilsMessengerEXT(*m_instance, callback, nullptr);
    m_instance.clear();
    glfwDestroyWindow(window);
    glfwTerminate();
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
