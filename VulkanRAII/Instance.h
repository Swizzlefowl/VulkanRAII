#pragma once
#include "Renderer.h"

class Instance {
  private:
    vk::raii::Context context;
    vk::raii::Instance instance{nullptr};

  public:
    Instance(const char* appName, const char* engineName);
};
