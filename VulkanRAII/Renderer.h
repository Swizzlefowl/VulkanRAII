#pragma once
#include <iostream>
#include <exception>
#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>

class Renderer {
  private:
    GLFWwindow* window;
    const int width{1280};
    const int height{720};

  public:
    void run();
    ~Renderer();

  private:
    void initWindow();
    void mainLoop();
    
};
