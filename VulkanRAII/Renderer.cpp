#include "Renderer.h"

void Renderer::run() {
    initWindow();
    mainLoop();
}

void Renderer::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(width, height, "hello Vulkan", nullptr, nullptr);
}

void Renderer::mainLoop() {
    while (!glfwWindowShouldClose(window))
        glfwPollEvents();
}

Renderer::~Renderer() {
    glfwDestroyWindow(window);
    glfwTerminate();
}
