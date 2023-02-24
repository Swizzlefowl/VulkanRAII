#pragma once
#include "commonIncludes.h"
#include <fstream>

class Renderer;
class Graphics {
  private:
    Renderer& m_renderer;
    vk::raii::ShaderModule createShaderModules(const std::string& fileName);

  public:
    vk::raii::RenderPass renderPass{nullptr};
    vk::raii::DescriptorSetLayout descriptorSetLayout{nullptr};
    vk::raii::PipelineLayout pipelineLayout{nullptr};
    vk::raii::Pipeline graphicsPipeline{nullptr};

    Graphics(Renderer& renderer);
    void createDescriptorLayout();
    void createGraphicsPipeline();
    void createRenderPass();
};
