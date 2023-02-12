#include "PresentationEngine.h"
#include "Renderer.h"

PresentationEngine::PresentationEngine(Renderer& renderer)
    : m_renderer{renderer} {
}

void PresentationEngine::createSurface() {
    // you have to give glfwCreatewindowSurface a vkSurface handle
    VkSurfaceKHR c_surface{};
    if (glfwCreateWindowSurface(*m_renderer.m_instance, m_renderer.window, nullptr, &c_surface) != VK_SUCCESS)
        throw std::runtime_error("failed to create a surface!");
    // luckily you can create a vk::raii::surface with a vkSurface
    m_surface = vk::raii::SurfaceKHR{
        m_renderer.m_instance, c_surface};
}
