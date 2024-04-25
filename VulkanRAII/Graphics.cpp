#include "Graphics.h"
#include "PresentationEngine.h"
#include "Renderer.h"

Graphics::Graphics(Renderer& renderer)
    : m_renderer{renderer} {
}

void Graphics::createDescriptorLayout() {
    vk::DescriptorSetLayoutBinding binding{};
    vk::DescriptorSetLayoutCreateInfo createInfo{};

    binding.binding = 0;
    binding.descriptorCount = 1;
    binding.descriptorType = vk::DescriptorType::eUniformBuffer;
    binding.stageFlags = vk::ShaderStageFlagBits::eVertex;
    binding.pImmutableSamplers = nullptr;

    createInfo.bindingCount = 1;
    createInfo.pBindings = &binding;

    try {
        descriptorSetLayout = m_renderer.m_device.createDescriptorSetLayout(createInfo);
    } catch (vk::Error& err) {
        std::cout << err.what();
    }
}

void Graphics::createGraphicsPipeline() {
    auto vertShaderModule{createShaderModules("vertex.spv")};
    auto fragShaderModule{createShaderModules("fragment.spv")};

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
    vertShaderStageInfo.module = *vertShaderModule;
    vertShaderStageInfo.pName = "main";

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragShaderStageInfo.module = *fragShaderModule;
    fragShaderStageInfo.pName = "main";

    vk::PipelineShaderStageCreateInfo shaderStagesInfo[]{vertShaderStageInfo,
        fragShaderStageInfo};

    vk::VertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Renderer::Vertex);
    bindingDescription.inputRate = vk::VertexInputRate::eVertex;

    vk::VertexInputBindingDescription bindingDescription2{};
    bindingDescription2.binding = 1;
    bindingDescription2.stride = sizeof(glm::vec3);
    bindingDescription2.inputRate = vk::VertexInputRate::eVertex;

    std::vector<vk::VertexInputBindingDescription> bindings{
        bindingDescription, bindingDescription2};

    std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
    attributeDescriptions[0].offset = 0;

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
    attributeDescriptions[1].offset = offsetof(Renderer::Vertex, Renderer::Vertex::color);

    attributeDescriptions[2].binding = 1;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = vk::Format::eR32G32B32Sfloat;
    attributeDescriptions[2].offset = 0;

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.vertexBindingDescriptionCount = bindings.size();
    vertexInputInfo.pVertexBindingDescriptions = bindings.data();
    vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    vk::PipelineViewportStateCreateInfo viewportState{};
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    rasterizer.frontFace = vk::FrontFace::eClockwise;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    vk::PipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = VK_FALSE;

    vk::PipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = vk::LogicOp::eCopy;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::vector<vk::DynamicState> dynamicStates{
        vk::DynamicState::eViewport, vk::DynamicState::eScissor};

    vk::PipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // vk::PushConstantRange pushConstant{};
    // pushConstant.offset = 0;
    // pushConstant.size = sizeof(Renderer::MeshPushConstants);
    // pushConstant.stageFlags = vk::ShaderStageFlagBits::eVertex;
  
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &(*descriptorSetLayout);
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    // pipelineLayoutInfo.pushConstantRangeCount = 1;
    // pipelineLayoutInfo.pPushConstantRanges = &pushConstant;

    try {
        pipelineLayout = m_renderer.m_device.createPipelineLayout(pipelineLayoutInfo);
    } catch (vk::SystemError& err) {
        err.what();
    }

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStagesInfo;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = *pipelineLayout;
    //pipelineInfo.renderPass = *renderPass;
    pipelineInfo.renderPass = VK_NULL_HANDLE;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
    pipelineRenderingCreateInfo.colorAttachmentCount = 1;
    pipelineRenderingCreateInfo.pColorAttachmentFormats = &m_renderer.pEngine->swapChainImagesFormat;
    // Chain into the pipeline create info
    pipelineInfo.pNext = &pipelineRenderingCreateInfo;
    graphicsPipeline = m_renderer.m_device.createGraphicsPipeline(nullptr, pipelineInfo);
}

vk::raii::ShaderModule Graphics::createShaderModules(const std::string& fileName) {
    // remember to do a bitwise or operation between the flags instead of
    // adding a comma....
    std::ifstream file{fileName, std::ios::ate | std::ios::binary};

    if (!file.is_open())
        throw std::runtime_error("failed to open file");

    size_t fileSize{static_cast<size_t>(file.tellg())};
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.codeSize = buffer.size();
    createInfo.pCode = reinterpret_cast<uint32_t*>(buffer.data());

    try {
        return vk::raii::ShaderModule{m_renderer.m_device, createInfo};
    } catch (vk::Error& err) {
        err.what();
    }
}

void Graphics::createRenderPass() {
    // the color attachment refers to the framebuffer attachment which
    // will be binded to our renderpass at drawing time
    vk::AttachmentDescription colorAttachment{};
    colorAttachment.format = m_renderer.pEngine->swapChainImagesFormat;
    colorAttachment.samples = vk::SampleCountFlagBits::e1;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
    colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
    //colorAttachment.finalLayout = vk::ImageLayout::eTransferSrcOptimal;

    // a reference to our framebuffer attachment
    vk::AttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpass{};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    // subpass external refers to the subpass outside our renderpass
    // instance, it tell our subpass to wait until the layout transition
    // is finished before we start writing to the color attachment
    
    vk::SubpassDependency depedancy{};
    depedancy.srcSubpass = VK_SUBPASS_EXTERNAL;
    depedancy.dstSubpass = 0;
    depedancy.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    depedancy.srcAccessMask = vk::AccessFlagBits::eNone;
    depedancy.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    depedancy.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

    vk::RenderPassCreateInfo renderPassInfo{};
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &depedancy;

    try {
        renderPass = m_renderer.m_device.createRenderPass(renderPassInfo);
    } catch (vk::SystemError& err) {
        err.what();
    }
}
