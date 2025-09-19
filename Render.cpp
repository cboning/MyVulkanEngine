#include "Render.h"
#include "Cloud.h"
#include "Data.h"
#include "Event/KeyInputEvent.h"
#include "JsonConfigReader/JsonConfigReader.h"
#include "Modelbase/Modelbase.h"

#include <fstream>

void Render::init()
{
    resourceInit();
    initWindowEvents();
}

void Render::resourceInit()
{
    Vkbase::ResourceBase::resourceManager().create<Vkbase::Window>("mainWindow1", "Vulkan Window", 800, 600);
    Vkbase::Window *pWindow = Vkbase::ResourceBase::resourceManager().create<Vkbase::Window>("mainWindow", "Vulkan Window", 800, 600);
    pWindow->setMouseMoveCallback([this](double x, double y) { Render::camera().addViewBy(x, -y); });

    VertexData frameVertices[] = {{glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
                                  {glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
                                  {glm::vec3(1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
                                  {glm::vec3(1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
                                  {glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
                                  {glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)}};
    _pFrameVerticesBuffer = Vkbase::ResourceBase::resourceManager().create<Vkbase::Buffer>("Vertex", "Device", sizeof(VertexData) * 6,
                                                                                           vk::BufferUsageFlagBits::eVertexBuffer, frameVertices);
    for (uint32_t i = 0; i < MAX_FLIGHT_COUNT; ++i)
        Vkbase::ResourceBase::resourceManager().create<Vkbase::Buffer>("UBO" + std::to_string(i), "Device", sizeof(UniformBufferData),
                                                                       vk::BufferUsageFlagBits::eUniformBuffer);

    Vkbase::ResourceBase::resourceManager().create<Vkbase::Sampler>("Sampler", "Device");

    delete new Cloud();

    Modelbase::Model *pModel =
        new Modelbase::Model("Device", dynamic_cast<const Vkbase::Sampler *>(_resourceManager.resource(Vkbase::ResourceType::Sampler, "Sampler"))->sampler(),
                             JsonConfigReader::load("config/model.json")[0]);

    _pFont = new Font("Device", "./src/fonts/Minecraft.ttf");
    _pText = new Text(*_pFont, "Hello Vulkan!", glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(10.0f, 50.0f), 1.0f);

    Modelbase::ModelInstance &instance = pModel->createNewInstance("1", {0, 0.0f});
    Object &modelObject = instance.object();
    modelObject.setScale(glm::vec3(1.0f));

    createDescriptorSets();
    createRenderPass();
}

void Render::initWindowEvents()
{
    Event::KeyInputEvent &event = dynamic_cast<Vkbase::Window *>(_resourceManager.resource(Vkbase::ResourceType::Window, "mainWindow"))->keyInputEvent();
    event.addPressedKeyEvent(GLFW_KEY_W, []() { _camera.moveFront(_speed * (_deltaTime)); });
    event.addPressedKeyEvent(GLFW_KEY_S, []() { _camera.moveBack(_speed * (_deltaTime)); });
    event.addPressedKeyEvent(GLFW_KEY_A, []() { _camera.moveLeft(_speed * (_deltaTime)); });
    event.addPressedKeyEvent(GLFW_KEY_D, []() { _camera.moveRight(_speed * (_deltaTime)); });
    event.addPressedKeyEvent(GLFW_KEY_SPACE, []() { _camera.moveUp(_speed * (_deltaTime)); });
    event.addPressedKeyEvent(GLFW_KEY_LEFT_SHIFT, []() { _camera.moveDown(_speed * (_deltaTime)); });
    event.addDownKeyEvent(GLFW_KEY_ESCAPE,
                          []() { dynamic_cast<Vkbase::Window *>(_resourceManager.resource(Vkbase::ResourceType::Window, "mainWindow"))->switchCursorState(); });
}

void Render::updateUniformBuffer(Vkbase::ResourceManager &resourceManager, uint32_t index)
{
    _camera.updatePerspective();
    UniformBufferData cameraData;
    cameraData.view = _camera.view();
    cameraData.proj = _camera.perspective();
    cameraData.model = glm::scale(glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)), glm::vec3(1.0f, 1.0f, 1.0f));

    dynamic_cast<Vkbase::Buffer *>(resourceManager.resource(Vkbase::ResourceType::Buffer, "UBO" + std::to_string(index)))->updateBufferData(&cameraData);
}

void Render::createRenderPass()
{
    const Vkbase::Device &device = *(dynamic_cast<const Vkbase::Device *>(_resourceManager.resource(Vkbase::ResourceType::Device, "Device")));
    vk::Format depthFormat = device.findSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                                                        vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);

    const Vkbase::Swapchain &swapchain = *dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow"));

    const Vkbase::RenderPass &renderPass = *(Vkbase::ResourceBase::resourceManager().create<Vkbase::RenderPass>(
        "mainWindow", "Device", JsonConfigReader::load("config/render.json")[0]["renderPass"], "mainWindow", depthFormat));

    // -------------------------------- Framebuffer --------------------------------
    vk::Extent2D extent = swapchain.extent();
    renderPass.createFramebuffer("mainWindow", JsonConfigReader::load("config/render.json")[0]["framebuffers"],extent.width, extent.height, "", depthFormat);

    const Vkbase::DescriptorSets &descriptorSets =
        *dynamic_cast<const Vkbase::DescriptorSets *>(_resourceManager.resource(Vkbase::ResourceType::DescriptorSets, "MainDescriptorSets"));
    Vkbase::Sampler &sampler = *dynamic_cast<Vkbase::Sampler *>(_resourceManager.resource(Vkbase::ResourceType::Sampler, "Sampler"));

    vk::DescriptorImageInfo imageInfo;
    imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setSampler(sampler.sampler());
    {
        std::vector<vk::DescriptorImageInfo> imageInfos(swapchain.imageNames().size(), imageInfo);
        for (uint32_t i = 0; i < swapchain.imageNames().size(); ++i)
            imageInfos[i].setImageView(
                dynamic_cast<const Vkbase::Image *>(_resourceManager.resource(Vkbase::ResourceType::Image, "Framebuffer_Image_BlurColor1_" + std::to_string(i)))->view());
        descriptorSets.writeSets("BlurSampler1", 0, {}, imageInfos, swapchain.imageNames().size());
    }
    // {
    //     std::vector<vk::DescriptorImageInfo> imageInfos(swapchain.imageNames().size(), imageInfo);
    //     for (uint32_t i = 0; i < swapchain.imageNames().size(); ++i)
    //         imageInfos[i].setImageView(dynamic_cast<const Vkbase::Image *>(_resourceManager.resource(Vkbase::ResourceType::Image, "Cloud"))->view());
    //     descriptorSets.writeSets("BlurSampler1", 0, {}, imageInfos, swapchain.imageNames().size());
    // }
    {
        std::vector<vk::DescriptorImageInfo> imageInfos(swapchain.imageNames().size(), imageInfo);
        for (uint32_t i = 0; i < swapchain.imageNames().size(); ++i)
            imageInfos[i].setImageView(
                dynamic_cast<const Vkbase::Image *>(_resourceManager.resource(Vkbase::ResourceType::Image, "Framebuffer_Image_BlurColor2_" + std::to_string(i)))->view());
        descriptorSets.writeSets("BlurSampler2", 0, {}, imageInfos, swapchain.imageNames().size());
    }

    imageInfo.setSampler(nullptr);
    {
        std::vector<vk::DescriptorImageInfo> imageInfos(swapchain.imageNames().size(), imageInfo);
        for (uint32_t i = 0; i < swapchain.imageNames().size(); ++i)
            imageInfos[i].setImageView(
                dynamic_cast<const Vkbase::Image *>(_resourceManager.resource(Vkbase::ResourceType::Image, "Framebuffer_Image_Position_" + std::to_string(i)))->view());
        descriptorSets.writeSets("G_BufferInputAttachments", 0, {}, imageInfos, swapchain.imageNames().size());
    }
    {
        std::vector<vk::DescriptorImageInfo> imageInfos(swapchain.imageNames().size(), imageInfo);
        for (uint32_t i = 0; i < swapchain.imageNames().size(); ++i)
            imageInfos[i].setImageView(
                dynamic_cast<const Vkbase::Image *>(_resourceManager.resource(Vkbase::ResourceType::Image, "Framebuffer_Image_Normal_" + std::to_string(i)))->view());
        descriptorSets.writeSets("G_BufferInputAttachments", 1, {}, imageInfos, swapchain.imageNames().size());
    }
    {
        std::vector<vk::DescriptorImageInfo> imageInfos(swapchain.imageNames().size(), imageInfo);
        for (uint32_t i = 0; i < swapchain.imageNames().size(); ++i)
            imageInfos[i].setImageView(
                dynamic_cast<const Vkbase::Image *>(_resourceManager.resource(Vkbase::ResourceType::Image, "Framebuffer_Image_AlbedoSpec_" + std::to_string(i)))->view());
        descriptorSets.writeSets("G_BufferInputAttachments", 2, {}, imageInfos, swapchain.imageNames().size());
    }
    {
        std::vector<vk::DescriptorImageInfo> imageInfos(swapchain.imageNames().size(), imageInfo);
        for (uint32_t i = 0; i < swapchain.imageNames().size(); ++i)
            imageInfos[i].setImageView(
                dynamic_cast<const Vkbase::Image *>(_resourceManager.resource(Vkbase::ResourceType::Image, "Framebuffer_Image_OriginalColor_" + std::to_string(i)))->view());
        descriptorSets.writeSets("BlendInputAttachments", 0, {}, imageInfos, swapchain.imageNames().size());
    }
    {
        std::vector<vk::DescriptorImageInfo> imageInfos(swapchain.imageNames().size(), imageInfo);
        for (uint32_t i = 0; i < swapchain.imageNames().size(); ++i)
            imageInfos[i].setImageView(
                dynamic_cast<const Vkbase::Image *>(_resourceManager.resource(Vkbase::ResourceType::Image, "Framebuffer_Image_BlurColor1_" + std::to_string(i)))->view());
        descriptorSets.writeSets("BlendInputAttachments", 1, {}, imageInfos, swapchain.imageNames().size());
    }

    // --------------------------------
    std::vector<Vkbase::ShaderInfo> shaderInfos = {Vkbase::ShaderInfo("", "main", vk::ShaderStageFlagBits::eVertex),
                                                   Vkbase::ShaderInfo("", "main", vk::ShaderStageFlagBits::eFragment)};

    Vkbase::VertexInfo modelVertexInfo(ModelData::Vertex::attributeDescriptions(), {ModelData::Vertex::bindingDescription()});
    Vkbase::VertexInfo screenVertexInfo(VertexData::attributeDescriptions(), {VertexData::bindingDescription()});
    Vkbase::VertexInfo textVertexInfo(Text::Vertex::attributeDescriptions(), {Text::Vertex::bindingDescription()});

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;

    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.setBlendEnable(vk::False).setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                                                     vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

    Vkbase::PipelineRenderInfo renderInfo = Vkbase::Pipeline::getDefaultRenderInfo();
    renderInfo.blendAttachments.push_back(colorBlendAttachment);

    descriptorSetLayouts = {descriptorSets.layout("BlendInputAttachments")};
    shaderInfos[0].filename = "shader/bin/blendVert.spv";
    shaderInfos[1].filename = "shader/bin/blendFrag.spv";
    renderInfo.rasterizationStateInfo.setCullMode(vk::CullModeFlagBits::eNone);
    renderInfo.subpass = 4;
    renderPass.createPipeline("blend", Vkbase::PipelineCreateInfo(shaderInfos, screenVertexInfo, descriptorSetLayouts, &renderInfo));

    descriptorSetLayouts = {descriptorSets.layout("BlurSampler1")};
    shaderInfos[0].filename = "shader/bin/blur_hVert.spv";
    shaderInfos[1].filename = "shader/bin/blur_hFrag.spv";
    renderInfo.rasterizationStateInfo.setCullMode(vk::CullModeFlagBits::eFront);
    renderInfo.subpass = 2;
    renderPass.createPipeline("blur_h", Vkbase::PipelineCreateInfo(shaderInfos, screenVertexInfo, descriptorSetLayouts, &renderInfo));

    descriptorSetLayouts = {descriptorSets.layout("BlurSampler2")};
    shaderInfos[0].filename = "shader/bin/blur_vVert.spv";
    shaderInfos[1].filename = "shader/bin/blur_vFrag.spv";
    renderInfo.rasterizationStateInfo.setCullMode(vk::CullModeFlagBits::eFront);
    renderInfo.subpass = 3;
    renderPass.createPipeline("blur_v", Vkbase::PipelineCreateInfo(shaderInfos, screenVertexInfo, descriptorSetLayouts, &renderInfo));

    renderInfo.blendAttachments[0]
        .setBlendEnable(vk::True)
        .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
        .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
        .setColorBlendOp(vk::BlendOp::eAdd)
        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
        .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
        .setAlphaBlendOp(vk::BlendOp::eAdd);

    descriptorSetLayouts = {_pFont->layout(), Font::projectiveLayout("MainDescriptorSets")};
    shaderInfos[0].filename = "shader/bin/textVert.spv";
    shaderInfos[1].filename = "shader/bin/textFrag.spv";
    renderInfo.rasterizationStateInfo.setCullMode(vk::CullModeFlagBits::eNone);
    renderInfo.subpass = 4;
    renderPass.createPipeline("text", Vkbase::PipelineCreateInfo(shaderInfos, textVertexInfo, descriptorSetLayouts, &renderInfo));

    renderInfo.blendAttachments[0] = colorBlendAttachment;

    renderInfo.blendAttachments.push_back(colorBlendAttachment);
    descriptorSetLayouts = {descriptorSets.layout("G_BufferInputAttachments")};
    shaderInfos[0].filename = "shader/bin/baseShaderVert.spv";
    shaderInfos[1].filename = "shader/bin/baseShaderFrag.spv";
    renderInfo.rasterizationStateInfo.setCullMode(vk::CullModeFlagBits::eNone);
    renderInfo.subpass = 1;
    renderPass.createPipeline("light", Vkbase::PipelineCreateInfo(shaderInfos, screenVertexInfo, descriptorSetLayouts, &renderInfo));

    std::vector<vk::DescriptorSetLayout> layout = (*Modelbase::Model::models().begin())->descriptorSetLayout(0, "g_buffer");
    descriptorSetLayouts.clear();
    descriptorSetLayouts.insert(descriptorSetLayouts.end(), layout.begin(), layout.end());

    renderInfo.rasterizationStateInfo.setCullMode(vk::CullModeFlagBits::eNone);
    renderInfo.blendAttachments.push_back(colorBlendAttachment);
    renderInfo.depthStencilStateInfo.setDepthTestEnable(vk::True)
        .setDepthWriteEnable(vk::True)
        .setDepthCompareOp(vk::CompareOp::eLess)
        .setDepthBoundsTestEnable(vk::False)
        .setMinDepthBounds(0.0f)
        .setMaxDepthBounds(1.0f)
        .setStencilTestEnable(vk::False);

    shaderInfos[0].filename = "shader/bin/g_bufferVert.spv";
    shaderInfos[1].filename = "shader/bin/g_bufferFrag.spv";
    renderInfo.subpass = 0;
    renderPass.createPipeline("g_buffer", Vkbase::PipelineCreateInfo(shaderInfos, modelVertexInfo, descriptorSetLayouts, &renderInfo));

    shaderInfos[0].filename = "shader/bin/inverstedHullVert.spv";
    shaderInfos[1].filename = "shader/bin/inverstedHullFrag.spv";
    renderInfo.rasterizationStateInfo.setCullMode(vk::CullModeFlagBits::eBack);
    renderInfo.subpass = 0;
    renderPass.createPipeline("Inversted_Hull", Vkbase::PipelineCreateInfo(shaderInfos, modelVertexInfo, descriptorSetLayouts, &renderInfo));

    createRenderDelegator();
}

void Render::createDescriptorSets()
{
    Vkbase::DescriptorSets *pDescriptorSets = Vkbase::ResourceBase::resourceManager().create<Vkbase::DescriptorSets>("MainDescriptorSets", "Device");
    const Vkbase::Swapchain &swapchain = *dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow"));
    pDescriptorSets->addDescriptorSetCreateConfig("Camera", {{vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex}},
                                                  Vkbase::RenderDelegator::maxFlightCount());
    pDescriptorSets->addDescriptorSetCreateConfig("G_BufferInputAttachments",
                                                  {{vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment},
                                                   {vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment},
                                                   {vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment}},
                                                  swapchain.images().size());
    pDescriptorSets->addDescriptorSetCreateConfig("BlurSampler1", {{vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment}},
                                                  swapchain.images().size());
    pDescriptorSets->addDescriptorSetCreateConfig("BlurSampler2", {{vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment}},
                                                  swapchain.images().size());
    pDescriptorSets->addDescriptorSetCreateConfig("BlendInputAttachments",
                                                  {{vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment},
                                                   {vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment}},
                                                  swapchain.images().size());

    Font::addProjectiveDescriptorSet("MainDescriptorSets");
    pDescriptorSets->init();

    Font::writeProjectiveDescriptorSet("MainDescriptorSets", "Device");
    Font::setScreenSize({800, 600});

    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.setOffset(0).setRange(sizeof(UniformBufferData));

    {
        std::vector<vk::DescriptorBufferInfo> bufferInfos(Vkbase::RenderDelegator::maxFlightCount(), bufferInfo);
        for (uint32_t i = 0; i < Vkbase::RenderDelegator::maxFlightCount(); ++i)
            bufferInfos[i].setBuffer(
                dynamic_cast<Vkbase::Buffer *>(_resourceManager.resource(Vkbase::ResourceType::Buffer, "UBO" + std::to_string(i)))->buffer());
        pDescriptorSets->writeSets("Camera", 0, bufferInfos, {}, Vkbase::RenderDelegator::maxFlightCount());
    }
}

void Render::createRenderDelegator()
{
    const Vkbase::Swapchain &swapchain = *dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow"));

    _pRenderDelegator = Vkbase::ResourceBase::resourceManager().create<Vkbase::RenderDelegator>("mainRender", "Device",
                                                                                                std::vector<std::string>{swapchain.name()}, "GraphicsDevice");
    _pRenderDelegator->setCommandRecordFunc([this](const vk::CommandBuffer &commandBuffer, uint32_t imageIndex, uint32_t currentFrame)
                                            { this->recordCommand(commandBuffer, imageIndex, currentFrame); });
    _pRenderDelegator->setRenderPassCreateFunc([this]() { this->createRenderPass(); });
}

void Render::draw()
{
    glfwPollEvents();
    clacDeltaTime();
    Event::KeyInputEvent::processing();
    if (_resourceManager.resources().count(Vkbase::ResourceType::RenderDelegator))
    {
        const std::unordered_map<std::string, Vkbase::ResourceBase *> resources = _resourceManager.resources().at(Vkbase::ResourceType::RenderDelegator);
        for (auto renderDelegator : resources)
            dynamic_cast<Vkbase::RenderDelegator *>(renderDelegator.second)->draw();
    }
    Vkbase::Window::delayDestroy();
}

void Render::cleanup()
{
    delete _pText;
    delete _pFont;
    _resourceManager.resource(Vkbase::ResourceType::Device, "Device")->destroy();
}

void Render::recordCommand(const vk::CommandBuffer &commandBuffer, uint32_t imageIndex, uint32_t currentFrame)
{
    updateUniformBuffer(_resourceManager, currentFrame);
    _pText->setText(std::to_string(1 / _deltaTime));

    const Vkbase::RenderPass &renderPass = *dynamic_cast<const Vkbase::RenderPass *>(_resourceManager.resource(Vkbase::ResourceType::RenderPass, "mainWindow"));
    const Vkbase::Swapchain &swapchain = *dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow"));
    const Vkbase::Pipeline &g_bufferPipeline = *dynamic_cast<const Vkbase::Pipeline *>(_resourceManager.resource(Vkbase::ResourceType::Pipeline, "g_buffer"));
    // const Vkbase::Pipeline &pipeline1 = *dynamic_cast<const Vkbase::Pipeline *>(_resourceManager.resource(Vkbase::ResourceType::Pipeline, "Inversted_Hull"));
    const Vkbase::DescriptorSets &descriptorSets =
        *dynamic_cast<const Vkbase::DescriptorSets *>(_resourceManager.resource(Vkbase::ResourceType::DescriptorSets, "MainDescriptorSets"));

    std::vector<vk::ClearValue> clearValues = {vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}), vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}),
                                               vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}), vk::ClearValue().setColor({1.0f, 1.0f, 1.0f, 1.0f}),
                                               vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}), vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}),
                                               vk::ClearValue().setColor({1.0f, 1.0f, 1.0f, 1.0f}), vk::ClearValue().setDepthStencil({1.0f, 0})};

    vk::Extent2D extent = swapchain.extent();
    renderPass.begin(
        commandBuffer,
        *dynamic_cast<const Vkbase::Framebuffer *>(_resourceManager.resource(Vkbase::ResourceType::Framebuffer, "mainWindow_" + std::to_string(imageIndex))),
        clearValues, extent);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, g_bufferPipeline.pipeline());
    for (Modelbase::Model *pModel : Modelbase::Model::models())
    {
        Modelbase::ModelInstance &instance = pModel->instance("1");

        pModel->updateAnimation(_deltaTime);
        instance.updateUniformBuffers(currentFrame, _camera);
        pModel->draw(currentFrame, commandBuffer, 0);
    }

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    renderFrame(commandBuffer, "light", descriptorSets.sets("G_BufferInputAttachments")[imageIndex]);

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    renderFrame(commandBuffer, "blur_h", descriptorSets.sets("BlurSampler1")[imageIndex]);

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    renderFrame(commandBuffer, "blur_v", descriptorSets.sets("BlurSampler2")[imageIndex]);

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    renderFrame(commandBuffer, "blend", descriptorSets.sets("BlendInputAttachments")[imageIndex]);
    Vkbase::Pipeline &textPipeline = *dynamic_cast<Vkbase::Pipeline *>(_resourceManager.resource(Vkbase::ResourceType::Pipeline, "text"));
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, textPipeline.pipeline());
    _pText->draw(commandBuffer, textPipeline, {Font::projectiveSet("MainDescriptorSets")});

    renderPass.end(commandBuffer);
}

void Render::renderFrame(const vk::CommandBuffer &commandBuffer, const std::string &pipelineName, const vk::DescriptorSet &descriptorSet)
{
    Vkbase::Pipeline &pipeline = *dynamic_cast<Vkbase::Pipeline *>(_resourceManager.resource(Vkbase::ResourceType::Pipeline, pipelineName));
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.pipeline());
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.layout(), 0, descriptorSet, {});
    commandBuffer.bindVertexBuffers(0, {_pFrameVerticesBuffer->buffer()}, {0});
    commandBuffer.draw(6, 1, 0, 0);
}

Camera &Render::camera() { return _camera; }

void Render::clacDeltaTime()
{
    _lastTime = _currentTime;
    _currentTime = glfwGetTime();
    _deltaTime = _currentTime - _lastTime;
}

bool Render::shouldEndApplication() { return _resourceManager.resources().count(Vkbase::ResourceType::Window); }