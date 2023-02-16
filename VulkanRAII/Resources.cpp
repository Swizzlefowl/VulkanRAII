#include "Resources.h"
#include "Graphics.h"
#include "PresentationEngine.h"
#include "Renderer.h"

// the attachments refer to the vkImage views which itself is a view into
// our swapchain images
void Resources::createframebuffers() {
    for (auto& attachment : m_renderer.pEngine->swapChainImageViews) {
        vk::FramebufferCreateInfo bufferInfo{};
        bufferInfo.width = m_renderer.pEngine->swapChainExtent.width;
        bufferInfo.height = m_renderer.pEngine->swapChainExtent.height;
        bufferInfo.layers = 1;
        bufferInfo.renderPass = *m_renderer.pGraphics->renderPass;
        bufferInfo.attachmentCount = 1;
        bufferInfo.pAttachments = &(*attachment);

        try {
            frambebuffers.push_back(m_renderer.m_device.createFramebuffer(bufferInfo));
        } catch (vk::Error& err) {
            err.what();
        }
    }
}

void Resources::createCommandPools() {
    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = m_renderer.getQueueFamilyIndex();
    try {
        commandPool = m_renderer.m_device.createCommandPool(poolInfo);
    } catch (vk::Error& err) {
        err.what();
    }
}

void Resources::createCommandbuffer() {
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool = *commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;
    commandBuffer = vk::raii::CommandBuffers{m_renderer.m_device, allocInfo};
}

void Resources::createSyncObjects() {
    vk::FenceCreateInfo fenceInfo;
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
    vk::SemaphoreCreateInfo semaphoreInfo{};

    try {
        inFlightFences = m_renderer.m_device.createFence(fenceInfo);
        imageAvailableSemaphores = m_renderer.m_device.createSemaphore(semaphoreInfo);
        finishedRenderingSemaphores = m_renderer.m_device.createSemaphore(semaphoreInfo);
    } catch (vk::Error& err) {
        err.what();
    }
}

Resources::Resources(Renderer& renderer)
    : m_renderer{renderer} {
}

void Resources::createResources() {
    createframebuffers();
    createCommandPools();
    createCommandbuffer();
    createSyncObjects();
}
