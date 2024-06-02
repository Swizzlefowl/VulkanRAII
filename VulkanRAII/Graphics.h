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
    vk::raii::DescriptorSetLayout skyDescriptorSetLayout{nullptr};
    vk::raii::PipelineLayout skyPipelineLayout{nullptr};
    vk::raii::Pipeline skyGraphicsPipeline{nullptr};
    vk::raii::Pipeline computePipeline{nullptr};
    vk::raii::DescriptorSetLayout computeDescriptorSetLayout{nullptr};
    vk::raii::PipelineLayout computePipelineLayout{nullptr};

    Graphics(Renderer& renderer);
    void createDescriptorLayout();
    void createGraphicsPipeline();
    void createSkyBoxPipeline();
    void createSkyBoxDescriptorLayout();
    void createComputeDescriptorLayout();
    void createComputePipeline();
    void createRenderPass();
};
