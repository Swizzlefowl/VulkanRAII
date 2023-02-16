#pragma once
#include "commonIncludes.h"

class Renderer;
class Resources {
  private:
    Renderer& m_renderer;
    void createframebuffers();

  public:
    std::vector<vk::raii::Framebuffer> frambebuffers;

    Resources(Renderer& renderer);
    void createResources();
};
