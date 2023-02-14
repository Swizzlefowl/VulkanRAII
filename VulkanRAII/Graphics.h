#pragma once
#include "commonIncludes.h"
#include <fstream>

class Renderer;
class Graphics {
  private:
    Renderer& m_renderer;

  public:
    vk::raii::RenderPass renderPass{nullptr};
    vk::raii::PipelineLayout pipelineLayout{nullptr};
    vk::raii::Pipeline graphicsPipeline{nullptr};

    Graphics(Renderer& renderer);
    void createGraphicsPipeline();
    void createShaderModules();
    void createRenderPass();
};
