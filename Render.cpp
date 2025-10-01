#include "Render.h"
#include "Cloud.h"
#include "Data.h"
#include "Engine/Entity/Cube.h"
#include "Engine/Physical/Motion/Collision.h"
#include "Engine/Physical/Motion/Friction.h"
#include "Engine/Physical/Motion/Gravity.h"
#include "Engine/Physical/Motion/Push.h"
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
    Vkbase::Window *pWindow = Vkbase::ResourceBase::resourceManager().create<Vkbase::Window>("mainWindow", "Vulkan Window", 800, 600);
    pWindow->setMouseMoveCallback([this](double x, double y) { Render::camera().addViewBy(x, -y); });
    pWindow->setMouseScrollCallback([](double, double y) { _speed = std::min(std::max(_speed + y * 0.1, 0.0), 30.0); });

    VertexData frameVertices[] = {{glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
                                  {glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
                                  {glm::vec3(1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
                                  {glm::vec3(1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
                                  {glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
                                  {glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)}};
    _pFrameVerticesBuffer = Vkbase::ResourceBase::resourceManager().create<Vkbase::Buffer>("Vertex", "Device", sizeof(VertexData) * 6,
                                                                                           vk::BufferUsageFlagBits::eVertexBuffer, frameVertices);

    Vkbase::ResourceBase::resourceManager().create<Vkbase::Sampler>("Sampler", "Device");
    Cube *pCube1 = new Cube("1");
    pCube1->addMotion("Gravity", (Motion *)(new Gravity()));
    pCube1->addMotion("Collision", (Motion *)(new Collision()));
    pCube1->addMotion("Friction", (Motion *)(new Friction()));
    _pPush = new Push();
    pCube1->addMotion("Push", (Motion *)_pPush);

    Cube *pCube4 = new Cube("4");
    pCube4->addMotion("Gravity", (Motion *)(new Gravity()));
    pCube4->addMotion("Collision", (Motion *)(new Collision()));
    pCube4->addMotion("Friction", (Motion *)(new Friction()));
    pCube4->object().setPosition(glm::vec3(5.0f, 3.0f, 0.0f));

    Object cubeObject2;
    cubeObject2.setScale(glm::vec3(100.0f, 0.5f, 100.0f));
    cubeObject2.setPosition(glm::vec3(0.0f, -10.0f, 0.0f));
    Cube *pCube2 = new Cube("2", false, cubeObject2);

    Object cubeObject3;
    cubeObject3.setScale(glm::vec3(50.0f, 0.5f, 50.0f));
    cubeObject3.setPosition(glm::vec3(0.0f, -9.5f, 0.0f));
    Cube *pCube3 = new Cube("3", false, cubeObject3);

    // delete new Cloud();

    Modelbase::Model *pModel =
        new Modelbase::Model("Device", dynamic_cast<const Vkbase::Sampler *>(_resourceManager.resource(Vkbase::ResourceType::Sampler, "Sampler"))->sampler(),
                             JsonConfigReader::load("config/model.json")[0]);

    _pFont = std::make_unique<Font>("Device", "./src/fonts/Minecraft.ttf");
    _pText = std::make_unique<Text>(*_pFont, "Hello Vulkan!", glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(10.0f, 50.0f), 1.0f);

    Modelbase::ModelInstance &instance = pModel->createNewInstance("1", {0, 0.0f});
    Object &modelObject = instance.object();
    modelObject.setScale(glm::vec3(1.0f));

    createDescriptorSets();
    createRenderPass();
}

void Render::initWindowEvents()
{
    Event::KeyInputEvent &event = dynamic_cast<Vkbase::Window *>(_resourceManager.resource(Vkbase::ResourceType::Window, "mainWindow"))->keyInputEvent();
    Object &Box1 = Entity::entity<Cube>("1").object();
    event.addPressedKeyEvent(GLFW_KEY_W, []() { _camera.moveFront(_speed * (_deltaTime)); });
    event.addPressedKeyEvent(GLFW_KEY_S, []() { _camera.moveBack(_speed * (_deltaTime)); });
    event.addPressedKeyEvent(GLFW_KEY_A, []() { _camera.moveLeft(_speed * (_deltaTime)); });
    event.addPressedKeyEvent(GLFW_KEY_D, []() { _camera.moveRight(_speed * (_deltaTime)); });
    event.addDownKeyEvent(GLFW_KEY_C,
                          [&]()
                          {
                              event.removeUpKeyEvent(GLFW_KEY_W);
                              event.removeUpKeyEvent(GLFW_KEY_S);
                              event.removeUpKeyEvent(GLFW_KEY_A);
                              event.removeUpKeyEvent(GLFW_KEY_D);
                              event.removeDownKeyEvent(GLFW_KEY_W);
                              event.removeDownKeyEvent(GLFW_KEY_S);
                              event.removeDownKeyEvent(GLFW_KEY_A);
                              event.removeDownKeyEvent(GLFW_KEY_D);

                              event.addPressedKeyEvent(GLFW_KEY_W, []() { _camera.moveFront(_speed * (_deltaTime)); });
                              event.addPressedKeyEvent(GLFW_KEY_S, []() { _camera.moveBack(_speed * (_deltaTime)); });
                              event.addPressedKeyEvent(GLFW_KEY_A, []() { _camera.moveLeft(_speed * (_deltaTime)); });
                              event.addPressedKeyEvent(GLFW_KEY_D, []() { _camera.moveRight(_speed * (_deltaTime)); });
                              event.addPressedKeyEvent(GLFW_KEY_SPACE, []() { _camera.moveUp(_speed * (_deltaTime)); });
                              event.addPressedKeyEvent(GLFW_KEY_LEFT_SHIFT, []() { _camera.moveDown(_speed * (_deltaTime)); });
                          });
    event.addDownKeyEvent(
        GLFW_KEY_R,
        [&]()
        {
            event.removeUpKeyEvent(GLFW_KEY_W);
            event.removeUpKeyEvent(GLFW_KEY_S);
            event.removeUpKeyEvent(GLFW_KEY_A);
            event.removeUpKeyEvent(GLFW_KEY_D);
            event.removeDownKeyEvent(GLFW_KEY_W);
            event.removeDownKeyEvent(GLFW_KEY_S);
            event.removeDownKeyEvent(GLFW_KEY_A);
            event.removeDownKeyEvent(GLFW_KEY_D);

            event.addPressedKeyEvent(GLFW_KEY_W,
                                     [&]() { Box1.setRotation(glm::angleAxis(_speed * _deltaTime, glm::vec3(1.0f, 0.0f, 0.0f)) * Box1.rotation()); });
            event.addPressedKeyEvent(GLFW_KEY_S,
                                     [&]() { Box1.setRotation(glm::angleAxis(_speed * _deltaTime, glm::vec3(-1.0f, 0.0f, 0.0f)) * Box1.rotation()); });
            event.addPressedKeyEvent(GLFW_KEY_A,
                                     [&]() { Box1.setRotation(glm::angleAxis(_speed * _deltaTime, glm::vec3(0.0f, -1.0f, 0.0f)) * Box1.rotation()); });
            event.addPressedKeyEvent(GLFW_KEY_D,
                                     [&]() { Box1.setRotation(glm::angleAxis(_speed * _deltaTime, glm::vec3(0.0f, 0.0f, 1.0f)) * Box1.rotation()); });
            event.addPressedKeyEvent(GLFW_KEY_SPACE,
                                     [&]() { Box1.setRotation(glm::angleAxis(_speed * _deltaTime, glm::vec3(0.0f, 1.0f, 0.0f)) * Box1.rotation()); });
            event.addPressedKeyEvent(GLFW_KEY_LEFT_SHIFT,
                                     [&]() { Box1.setRotation(glm::angleAxis(_speed * _deltaTime, glm::vec3(0.0f, -1.0f, 0.0f)) * Box1.rotation()); });
        });
    event.addDownKeyEvent(
        GLFW_KEY_P,
        [&]()
        {
            event.removePressedKeyEvent(GLFW_KEY_W);
            event.removePressedKeyEvent(GLFW_KEY_S);
            event.removePressedKeyEvent(GLFW_KEY_A);
            event.removePressedKeyEvent(GLFW_KEY_D);

            event.addDownKeyEvent(GLFW_KEY_W, [&]() { _pPush->setAcceleration(_pPush->acceleration() + _speed * glm::vec3(1.0f, 0.0f, 0.0f)); });
            event.addDownKeyEvent(GLFW_KEY_S, [&]() { _pPush->setAcceleration(_pPush->acceleration() + _speed * glm::vec3(-1.0f, 0.0f, 0.0f)); });
            event.addDownKeyEvent(GLFW_KEY_A, [&]() { _pPush->setAcceleration(_pPush->acceleration() + _speed * glm::vec3(0.0f, 0.0f, -1.0f)); });
            event.addDownKeyEvent(GLFW_KEY_D, [&]() { _pPush->setAcceleration(_pPush->acceleration() + _speed * glm::vec3(0.0f, 0.0f, 1.0f)); });

            event.addUpKeyEvent(GLFW_KEY_W, [&]() { _pPush->setAcceleration(_pPush->acceleration() - _speed * glm::vec3(1.0f, 0.0f, 0.0f)); });
            event.addUpKeyEvent(GLFW_KEY_S, [&]() { _pPush->setAcceleration(_pPush->acceleration() - _speed * glm::vec3(-1.0f, 0.0f, 0.0f)); });
            event.addUpKeyEvent(GLFW_KEY_A, [&]() { _pPush->setAcceleration(_pPush->acceleration() - _speed * glm::vec3(0.0f, 0.0f, -1.0f)); });
            event.addUpKeyEvent(GLFW_KEY_D, [&]() { _pPush->setAcceleration(_pPush->acceleration() - _speed * glm::vec3(0.0f, 0.0f, 1.0f)); });

            event.addPressedKeyEvent(GLFW_KEY_SPACE, [&]() { Box1.setPosition(Box1.position() + glm::vec3(0.0f, 1.0f, 0.0f) * _speed * (_deltaTime)); });
            event.addPressedKeyEvent(GLFW_KEY_LEFT_SHIFT, [&]() { Box1.setPosition(Box1.position() + glm::vec3(0.0f, -1.0f, 0.0f) * _speed * (_deltaTime)); });
        });
    event.addDownKeyEvent(
        GLFW_KEY_F,
        [&]()
        {
            event.removeUpKeyEvent(GLFW_KEY_W);
            event.removeUpKeyEvent(GLFW_KEY_S);
            event.removeUpKeyEvent(GLFW_KEY_A);
            event.removeUpKeyEvent(GLFW_KEY_D);
            event.removeDownKeyEvent(GLFW_KEY_W);
            event.removeDownKeyEvent(GLFW_KEY_S);
            event.removeDownKeyEvent(GLFW_KEY_A);
            event.removeDownKeyEvent(GLFW_KEY_D);

            event.addPressedKeyEvent(GLFW_KEY_W, [&]() { Box1.setScale(Box1.scale() + glm::vec3(1.0f, 0.0f, 0.0f) * _speed * (_deltaTime)); });
            event.addPressedKeyEvent(GLFW_KEY_S, [&]() { Box1.setScale(Box1.scale() + glm::vec3(-1.0f, 0.0f, 0.0f) * _speed * (_deltaTime)); });
            event.addPressedKeyEvent(GLFW_KEY_A, [&]() { Box1.setScale(Box1.scale() + glm::vec3(0.0f, 0.0f, -1.0f) * _speed * (_deltaTime)); });
            event.addPressedKeyEvent(GLFW_KEY_D, [&]() { Box1.setScale(Box1.scale() + glm::vec3(0.0f, 0.0f, 1.0f) * _speed * (_deltaTime)); });
            event.addPressedKeyEvent(GLFW_KEY_SPACE, [&]() { Box1.setScale(Box1.scale() + glm::vec3(0.0f, 1.0f, 0.0f) * _speed * (_deltaTime)); });
            event.addPressedKeyEvent(GLFW_KEY_LEFT_SHIFT, [&]() { Box1.setScale(Box1.scale() + glm::vec3(0.0f, -1.0f, 0.0f) * _speed * (_deltaTime)); });
        });

    event.addPressedKeyEvent(GLFW_KEY_SPACE, []() { _camera.moveUp(_speed * (_deltaTime)); });
    event.addPressedKeyEvent(GLFW_KEY_LEFT_SHIFT, []() { _camera.moveDown(_speed * (_deltaTime)); });
    event.addDownKeyEvent(GLFW_KEY_ESCAPE,
                          []() { dynamic_cast<Vkbase::Window *>(_resourceManager.resource(Vkbase::ResourceType::Window, "mainWindow"))->switchCursorState(); });
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
    renderPass.createFramebuffer("mainWindow", JsonConfigReader::load("config/render.json")[0]["framebuffers"], extent.width, extent.height, "mainWindow",
                                 depthFormat);
    Vkbase::DescriptorSets &descriptorSets = renderPass.descriptorSets();
    descriptorSets.addDescriptorSetCreateConfigWithJson(JsonConfigReader::load("config/render.json")[0]["descriptorSets"]["sets"]);
    descriptorSets.init();
    descriptorSets.writeSetsWithJson(JsonConfigReader::load("config/render.json")[0]["descriptorSets"]["write"]);

    Vkbase::VertexInfo modelVertexInfo(ModelData::Vertex::attributeDescriptions(), {ModelData::Vertex::bindingDescription()});
    Vkbase::VertexInfo screenVertexInfo(VertexData::attributeDescriptions(), {VertexData::bindingDescription()});
    Vkbase::VertexInfo textVertexInfo(Text::Vertex::attributeDescriptions(), {Text::Vertex::bindingDescription()});
    Vkbase::VertexInfo cubeVertexInfo(GeometryVertexData::attributeDescriptions(), {GeometryVertexData::bindingDescription()});

    const std::unordered_map<std::string, Vkbase::VertexInfo> vertexInfos = {
        {"blend", screenVertexInfo}, {"blur_h", screenVertexInfo},  {"blur_v", screenVertexInfo},        {"light", screenVertexInfo},
        {"text", textVertexInfo},    {"g_buffer", modelVertexInfo}, {"GeometryPipeline", cubeVertexInfo}};

    const std::unordered_map<std::string, std::vector<vk::DescriptorSetLayout>> descriptorSetLayouts = {
        {"blend", {descriptorSets.layout("BlendInputAttachments")}},
        {"blur_h", {descriptorSets.layout("BlurSampler1")}},
        {"blur_v", {descriptorSets.layout("BlurSampler2")}},
        {"light", {descriptorSets.layout("G_BufferInputAttachments")}},
        {"text", {_pFont->layout(), Font::projectiveLayout("MainDescriptorSets")}},
        {"g_buffer", (*Modelbase::Model::models().begin())->descriptorSetLayout(0, "g_buffer")},
        {"GeometryPipeline", Entity::entity<Cube>("1").descriptorSetLayouts()}};

    const std::pair<std::vector<vk::Rect2D>, std::vector<vk::Viewport>> viewportInfo = {{vk::Rect2D().setExtent(swapchain.extent())},
                                                                                        {vk::Viewport().setWidth(extent.width).setHeight(extent.height)}};

    const std::unordered_map<std::string, std::pair<std::vector<vk::Rect2D>, std::vector<vk::Viewport>>> viewportInfos = {
        {"blend", viewportInfo}, {"blur_h", viewportInfo},   {"blur_v", viewportInfo},          {"light", viewportInfo},
        {"text", viewportInfo},  {"g_buffer", viewportInfo}, {"GeometryPipeline", viewportInfo}};

    renderPass.createPipelines(JsonConfigReader::load("config/render.json")[0]["pipelines"], vertexInfos, descriptorSetLayouts, viewportInfos);

    createRenderDelegator();
}

void Render::createDescriptorSets()
{
    Vkbase::DescriptorSets *pDescriptorSets = Vkbase::ResourceBase::resourceManager().create<Vkbase::DescriptorSets>("MainDescriptorSets", "Device");
    Font::addProjectiveDescriptorSet(pDescriptorSets->name());
    pDescriptorSets->init();

    Font::writeProjectiveDescriptorSet(pDescriptorSets->name(), "Device");
    Font::setScreenSize({800, 600});
}

void Render::createRenderDelegator()
{
    const Vkbase::Swapchain &swapchain = *dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow"));

    _pRenderDelegator = Vkbase::ResourceBase::resourceManager().create<Vkbase::RenderDelegator>("MainRender", "Device", swapchain.name(), "GraphicsDevice");
    _pRenderDelegator->setCommandRecordFunc([this](const vk::CommandBuffer &commandBuffer, uint32_t imageIndex, uint32_t currentFrame)
                                            { this->recordCommand(commandBuffer, imageIndex, currentFrame); });
    _pRenderDelegator->setRenderPassCreateFunc([this]() { this->createRenderPass(); });
}

void Render::draw()
{
    glfwPollEvents();
    clacDeltaTime();
    Event::KeyInputEvent::processing();
    _camera.updatePerspective();

    if (_resourceManager.resources().count(Vkbase::ResourceType::RenderDelegator))
    {
        const std::unordered_map<std::string, Vkbase::ResourceBase *> resources = _resourceManager.resources().at(Vkbase::ResourceType::RenderDelegator);
        for (auto renderDelegator : resources)
            dynamic_cast<Vkbase::RenderDelegator *>(renderDelegator.second)->draw();
    }
    Vkbase::Window::delayDestroy();
}

void Render::cleanup() {}

void Render::recordCommand(const vk::CommandBuffer &commandBuffer, uint32_t imageIndex, uint32_t currentFrame)
{
    _pText->setText(std::to_string(_speed));

    Vkbase::RenderPass &renderPass = *dynamic_cast<Vkbase::RenderPass *>(_resourceManager.resource(Vkbase::ResourceType::RenderPass, "mainWindow"));
    const Vkbase::DescriptorSets &descriptorSets = renderPass.descriptorSets();

    std::vector<vk::ClearValue> clearValues = {vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}), vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}),
                                               vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}), vk::ClearValue().setColor({1.0f, 1.0f, 1.0f, 1.0f}),
                                               vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}), vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}),
                                               vk::ClearValue().setColor({1.0f, 1.0f, 1.0f, 1.0f}), vk::ClearValue().setDepthStencil({1.0f, 0})};

    vk::Extent2D extent = dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow"))->extent();
    renderPass.begin(
        commandBuffer,
        *dynamic_cast<const Vkbase::Framebuffer *>(_resourceManager.resource(Vkbase::ResourceType::Framebuffer, "mainWindow_" + std::to_string(imageIndex))),
        clearValues, extent);

    // for (Modelbase::Model *pModel : Modelbase::Model::models())
    // {
    //     Modelbase::ModelInstance &instance = pModel->instance("1");

    //     pModel->updateAnimation(_deltaTime);
    //     instance.updateUniformBuffers(currentFrame, _camera);
    //     pModel->draw(currentFrame, commandBuffer, 0);
    // }

    Entity::drawEntities(commandBuffer, _camera, currentFrame);

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