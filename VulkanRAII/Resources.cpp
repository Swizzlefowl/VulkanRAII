#include "Resources.h"
#include "Graphics.h"
#include "PresentationEngine.h"
#include "Renderer.h"

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

Resources::Resources(Renderer& renderer)
    : m_renderer{renderer} {
}

void Resources::createResources() {
    createframebuffers();
}
