#pragma once
#include "commonIncludes.h"
#include <fstream>

class Renderer;
class Graphics {
  private:
    Renderer& m_renderer;

  public:
    vk::raii::Pipeline GraphicsPipeline{nullptr};
    vk::raii::RenderPass renderPass{nullptr};

    Graphics(Renderer& renderer);
};
