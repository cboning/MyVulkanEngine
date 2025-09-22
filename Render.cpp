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

    Vkbase::RenderPass &renderPass = *(Vkbase::ResourceBase::resourceManager().create<Vkbase::RenderPass>(
        "mainWindow", "Device", JsonConfigReader::load("config/render.json")[0]["renderPass"], "mainWindow", depthFormat));
    vk::Extent2D extent = swapchain.extent();
    renderPass.createFramebuffer("mainWindow", JsonConfigReader::load("config/render.json")[0]["framebuffers"], extent.width, extent.height, "", depthFormat);
    Vkbase::DescriptorSets &descriptorSets = renderPass.descriptorSets();
    descriptorSets.addDescriptorSetCreateConfigWithJson(JsonConfigReader::load("config/render.json")[0]["descriptorSets"]["sets"]);
    descriptorSets.init();
    descriptorSets.writeSetsWithJson(JsonConfigReader::load("config/render.json")[0]["descriptorSets"]["write"]);

    Vkbase::VertexInfo modelVertexInfo(ModelData::Vertex::attributeDescriptions(), {ModelData::Vertex::bindingDescription()});
    Vkbase::VertexInfo screenVertexInfo(VertexData::attributeDescriptions(), {VertexData::bindingDescription()});
    Vkbase::VertexInfo textVertexInfo(Text::Vertex::attributeDescriptions(), {Text::Vertex::bindingDescription()});

    const std::unordered_map<std::string, Vkbase::VertexInfo> vertexInfos = {{"blend", screenVertexInfo},  {"blur_h", screenVertexInfo},
                                                                             {"blur_v", screenVertexInfo}, {"light", screenVertexInfo},
                                                                             {"text", textVertexInfo},     {"g_buffer", modelVertexInfo}};

    const std::unordered_map<std::string, std::vector<vk::DescriptorSetLayout>> descriptorSetLayouts = {
        {"blend", {descriptorSets.layout("BlendInputAttachments")}},
        {"blur_h", {descriptorSets.layout("BlurSampler1")}},
        {"blur_v", {descriptorSets.layout("BlurSampler2")}},
        {"light", {descriptorSets.layout("G_BufferInputAttachments")}},
        {"text", {_pFont->layout(), Font::projectiveLayout("MainDescriptorSets")}},
        {"g_buffer", (*Modelbase::Model::models().begin())->descriptorSetLayout(0, "g_buffer")}};

    const std::pair<std::vector<vk::Rect2D>, std::vector<vk::Viewport>> viewportInfo = {{vk::Rect2D().setExtent(swapchain.extent())},
                                                                                        {vk::Viewport().setWidth(extent.width).setHeight(extent.height)}};

    const std::unordered_map<std::string, std::pair<std::vector<vk::Rect2D>, std::vector<vk::Viewport>>> viewportInfos = {
        {"blend", viewportInfo}, {"blur_h", viewportInfo}, {"blur_v", viewportInfo},
        {"light", viewportInfo}, {"text", viewportInfo},   {"g_buffer", viewportInfo}};

    renderPass.createPipelines(JsonConfigReader::load("config/render.json")[0]["pipelines"], vertexInfos, descriptorSetLayouts, viewportInfos);

    createRenderDelegator();
}

void Render::createDescriptorSets()
{
    Vkbase::DescriptorSets *pDescriptorSets = Vkbase::ResourceBase::resourceManager().create<Vkbase::DescriptorSets>("MainDescriptorSets", "Device");
    pDescriptorSets->addDescriptorSetCreateConfig("Camera", {{vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex}},
                                                  Vkbase::RenderDelegator::maxFlightCount());
    Font::addProjectiveDescriptorSet(pDescriptorSets->name());
    pDescriptorSets->init();

    Font::writeProjectiveDescriptorSet(pDescriptorSets->name(), "Device");
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

    _pRenderDelegator = Vkbase::ResourceBase::resourceManager().create<Vkbase::RenderDelegator>("MainRender", "Device",
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
}

void Render::recordCommand(const vk::CommandBuffer &commandBuffer, uint32_t imageIndex, uint32_t currentFrame)
{
    updateUniformBuffer(_resourceManager, currentFrame);
    _pText->setText(std::to_string(1 / _deltaTime));

    Vkbase::RenderPass &renderPass = *dynamic_cast<Vkbase::RenderPass *>(_resourceManager.resource(Vkbase::ResourceType::RenderPass, "mainWindow"));
    const Vkbase::Swapchain &swapchain = *dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow"));
    const Vkbase::Pipeline &g_bufferPipeline = *dynamic_cast<const Vkbase::Pipeline *>(_resourceManager.resource(Vkbase::ResourceType::Pipeline, "g_buffer"));
    const Vkbase::DescriptorSets &descriptorSets = renderPass.descriptorSets();

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