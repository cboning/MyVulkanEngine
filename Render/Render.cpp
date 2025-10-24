#include "Render.h"
#include "../Cloud.h"
#include "../Data.h"
#include "../Data/Octree.h"
#include "../Engine/Entity/Character.h"
#include "../Engine/Entity/Cube.h"
#include "../Engine/Entity/ModelEntity.h"
#include "../Engine/Entity/Motion/Collision.h"
#include "../Engine/Entity/Motion/Friction.h"
#include "../Engine/Entity/Motion/Gravity.h"
#include "../Engine/Entity/Motion/Push.h"
#include "../Engine/Physical/Collision/CollisionSystem.h"
#include "../Event/KeyInputEvent.h"
#include "../JsonConfigReader/JsonConfigReader.h"
#include "../Modelbase/Modelbase.h"
#include "../Resources/FontResource.h"
#include "../Resources/ModelResource.h"
#include "../Resources/ResourceManager.h"

#include <fstream>

Render::Render(const std::string &windowName)
{
    Vkbase::Window *pResourcesWindow = createResource<Vkbase::Window>("resourceWindow", "Resources Window", 800, 600);
    _pWindow = createResource<Vkbase::Window>(windowName, windowName, 800, 600);
    _deviceName = _pWindow->device().name();

    _cameraLight.movePosTo(50, 300, 40);
    _cameraLight.lookAt(glm::vec3(0.0f));
    _cameraLight.setLight(true);
    _cameraLight.setNearFar(-1000, 1000);
    _cameraLight.setFrameSize(glm::vec2(300.0f));
    _pWindow->setMouseMoveCallback([this](double x, double y) { _camera.addViewBy(x, -y); });
    _pWindow->setMouseScrollCallback([](double, double y) { _speed = std::min(std::max(_speed + y * 0.1, 0.0), 30.0); });

    if (!Vkbase::VkResourceBase::resourceManager().resource(Vkbase::VkResourceType::Image, "Empty"))
        createResource<Vkbase::Image>("Empty", deviceName(), 1, 1, 1, vk::Format::eR8G8B8A8Srgb, vk::ImageType::e2D, vk::ImageViewType::e2D,
                                      vk::ImageUsageFlagBits::eSampled, (uint32_t[]){0xFFFF00FF});

    std::shared_ptr<Cube> pCube1 = std::make_shared<Cube>("1", deviceName(), _camera, _cameraLight);
    _renderObjects.push_back(pCube1);
    pCube1->addMotion("Gravity", (Motion *)(new Gravity()));
    pCube1->addMotion("Collision", (Motion *)(new Collision()));
    pCube1->addMotion("Friction", (Motion *)(new Friction()));
    _pPush = new Push();
    pCube1->addMotion("Push", (Motion *)_pPush);
    pCube1->object().setPosition(glm::vec3(5.0f, 200.0f, 0.0f));
    pCube1->object().setScale(glm::vec3(5.0f, 13.0f, 5.0f));

    std::shared_ptr<Cube> pCube4 = std::make_shared<Cube>("4", deviceName(), _camera, _cameraLight);
    _renderObjects.push_back(pCube4);
    pCube4->addMotion("Gravity", (Motion *)(new Gravity()));
    pCube4->addMotion("Collision", (Motion *)(new Collision()));
    pCube4->addMotion("Friction", (Motion *)(new Friction()));
    pCube4->object().setPosition(glm::vec3(5.0f, 50.0f, 0.0f));
    pCube4->object().setScale(glm::vec3(5.0f, 13.0f, 5.0f));

    Object cubeObject2;
    cubeObject2.setScale(glm::vec3(100.0f, 0.5f, 100.0f));
    cubeObject2.setPosition(glm::vec3(0.0f, -10.0f, 0.0f));
    std::shared_ptr<Cube> pCube2 = std::make_shared<Cube>("2", deviceName(), _camera, _cameraLight, false, cubeObject2);
    _renderObjects.push_back(pCube2);

    Object cubeObject3;
    cubeObject3.setScale(glm::vec3(50.0f, 0.5f, 50.0f));
    cubeObject3.setPosition(glm::vec3(0.0f, -9.5f, 0.0f));
    std::shared_ptr<Cube> pCube3 = std::make_shared<Cube>("3", deviceName(), _camera, _cameraLight, false, cubeObject3);
    _renderObjects.push_back(pCube3);
    for (auto &object : _renderObjects){
        _renderObjectManager.addObject("Normal", "GeometryPipeline", object);
        _renderObjectManager.addObject("Shadow", "GeometryShadow", object);
    }

    Object cubeObject5;
    cubeObject5.setPosition(glm::vec3(30.0f, 30.0f, 30.0f));

    std::shared_ptr<ModelEntity> pCharacter = std::make_shared<ModelEntity>(
        "5", deviceName(), _camera,
        *Resources::ResourceManager::instance()
             .getResource<Resources::ModelResource>(deviceName(), Vkbase::Sampler::getDefaultCreateInfo(), JsonConfigReader::load("config/model.json")[0])
             .model(),
        false, cubeObject5);
    _renderObjects.push_back(pCharacter);

    pCharacter->modelObject().setScale(glm::vec3(0.01f));
    _renderObjectManager.addObject("Normal", "GeometryPipeline", _renderObjects.back());

    // delete new Cloud();

    _pText = Resources::ResourceManager::instance()
                  .getResource<Resources::FontResource>(deviceName(), "./src/fonts/Minecraft.ttf")
                  .font()
                  ->createText("Hello Vulkan!", glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(10.0f, 50.0f), 1.0f);
    
    // register text using a shared_ptr; avoid passing the weak_ptr directly which doesn't match addObject overload
    if (auto spText = _pText.lock())
        _renderObjectManager.addObject("Blend", "text", std::weak_ptr<Vkbase::RenderObjectDelegator>(spText));
    json renderPassConfig = JsonConfigReader::load("./config/renderPass.json")[0];
    _renderPassManagers.emplace_back(renderPassConfig);
    _renderPassManagers.back().clearValue("Normal")[4].depthStencil.setDepth(1.0f);
    _renderPassManagers.back().clearValue("Shadow")[0].depthStencil.setDepth(1.0f);

    createDescriptorSets();
    createRenderPass();
    createRenderDelegator();

    initWindowEvents();
}

const std::string &Render::deviceName() { return _deviceName; }

void Render::initWindowEvents()
{
    Event::KeyInputEvent &event = dynamic_cast<Vkbase::Window *>(_resourceManager.resource(Vkbase::VkResourceType::Window, "mainWindow"))->keyInputEvent();
    Object &Box1 = Entity::entity<Cube>("1")->object();
    event.addPressedKeyEvent(GLFW_KEY_W, [&]() { _camera.moveFront(_speed * (_deltaTime)); });
    event.addPressedKeyEvent(GLFW_KEY_S, [&]() { _camera.moveBack(_speed * (_deltaTime)); });
    event.addPressedKeyEvent(GLFW_KEY_A, [&]() { _camera.moveLeft(_speed * (_deltaTime)); });
    event.addPressedKeyEvent(GLFW_KEY_D, [&]() { _camera.moveRight(_speed * (_deltaTime)); });
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

                              event.addPressedKeyEvent(GLFW_KEY_W, [&]() { _camera.moveFront(_speed * (_deltaTime)); });
                              event.addPressedKeyEvent(GLFW_KEY_S, [&]() { _camera.moveBack(_speed * (_deltaTime)); });
                              event.addPressedKeyEvent(GLFW_KEY_A, [&]() { _camera.moveLeft(_speed * (_deltaTime)); });
                              event.addPressedKeyEvent(GLFW_KEY_D, [&]() { _camera.moveRight(_speed * (_deltaTime)); });
                              event.addPressedKeyEvent(GLFW_KEY_SPACE, [&]() { _camera.moveUp(_speed * (_deltaTime)); });
                              event.addPressedKeyEvent(GLFW_KEY_LEFT_SHIFT, [&]() { _camera.moveDown(_speed * (_deltaTime)); });
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

    event.addPressedKeyEvent(GLFW_KEY_SPACE, [&]() { _camera.moveUp(_speed * (_deltaTime)); });
    event.addPressedKeyEvent(GLFW_KEY_LEFT_SHIFT, [&]() { _camera.moveDown(_speed * (_deltaTime)); });
    event.addDownKeyEvent(GLFW_KEY_ESCAPE, []()
                          { dynamic_cast<Vkbase::Window *>(_resourceManager.resource(Vkbase::VkResourceType::Window, "mainWindow"))->switchCursorState(); });
}

void Render::createRenderPass()
{
    const Vkbase::Device &device = *(dynamic_cast<const Vkbase::Device *>(_resourceManager.resource(Vkbase::VkResourceType::Device, deviceName())));
    vk::Format depthFormat = device.findSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                                                        vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
    const std::string &windowName = _pWindow->name();

    vk::Extent2D extent = dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::VkResourceType::Swapchain, windowName))->extent();

    json renderPassConfigs = JsonConfigReader::load("config/render.json");
    std::vector<std::string> renderPassNames;
    renderPassNames.reserve(renderPassConfigs.size());

    Vkbase::VertexInfo modelVertexInfo(ModelData::Vertex::attributeDescriptions(), {ModelData::Vertex::bindingDescription()});
    Vkbase::VertexInfo screenVertexInfo(RenderFrame::ScreenVertexData::attributeDescriptions(), {RenderFrame::ScreenVertexData::bindingDescription()});
    Vkbase::VertexInfo textVertexInfo(Text::Vertex::attributeDescriptions(), {Text::Vertex::bindingDescription()});
    Vkbase::VertexInfo cubeVertexInfo(GeometryVertexData::attributeDescriptions(), {GeometryVertexData::bindingDescription()});

    const std::unordered_map<std::string, Vkbase::VertexInfo> vertexInfos = {
        {"blend", screenVertexInfo},       {"blur_h", screenVertexInfo},         {"blur_v", screenVertexInfo},
        {"light", screenVertexInfo},       {"present", screenVertexInfo},        {"text", textVertexInfo},
        {"g_buffer", modelVertexInfo},     {"GeometryPipeline", cubeVertexInfo}, {"GeometryOutlinePipeline", cubeVertexInfo},
        {"GeometryShadow", cubeVertexInfo}};

    const std::pair<std::vector<vk::Rect2D>, std::vector<vk::Viewport>> viewportInfo = {{vk::Rect2D().setExtent(extent)},
                                                                                        {vk::Viewport().setWidth(extent.width).setHeight(extent.height)}};

    const std::unordered_map<std::string, std::pair<std::vector<vk::Rect2D>, std::vector<vk::Viewport>>> viewportInfos = {
        {"blend", viewportInfo},         {"blur_h", viewportInfo},           {"blur_v", viewportInfo},
        {"light", viewportInfo},         {"present", viewportInfo},          {"text", viewportInfo},
        {"g_buffer", viewportInfo},      {"GeometryPipeline", viewportInfo}, {"GeometryOutlinePipeline", viewportInfo},
        {"GeometryShadow", viewportInfo}};

    const std::unordered_map<std::string, std::vector<vk::DescriptorSetLayout>> descriptorSetLayouts = {
        {"blend", {}},
        {"blur_h", {}},
        {"blur_v", {}},
        {"light", {}},
        {"present", {}},
        {"text",
         {Resources::ResourceManager::instance().getResource<Resources::FontResource>(deviceName(), "./src/fonts/Minecraft.ttf").font()->layout(),
          Font::projectiveLayout("MainDescriptorSets")}},
        {"g_buffer", {(*Modelbase::Model::models().begin())->descriptorSetLayout(0, "g_buffer")}},
        {"GeometryPipeline", Entity::entity<Cube>("1")->descriptorSetLayouts()},
        {"GeometryOutlinePipeline", Entity::entity<Cube>("1")->descriptorSetLayouts()},
        {"GeometryShadow", Entity::entity<Cube>("1")->descriptorSetLayouts()}};

    for (const auto &config : renderPassConfigs.items())
    {
        Vkbase::RenderPass &renderPass =
            *(createResource<Vkbase::RenderPass>(config.key(), deviceName(), config.value()["renderPass"], windowName, depthFormat));
        renderPassNames.push_back(renderPass.name());
        renderPass.createFramebuffer(config.key(), config.value()["framebuffers"], extent.width, extent.height, windowName, depthFormat);
    }

    for (const auto &config : renderPassConfigs.items())
    {
        Vkbase::RenderPass &renderPass = *dynamic_cast<Vkbase::RenderPass *>(_resourceManager.resource(Vkbase::VkResourceType::RenderPass, config.key()));
        Vkbase::DescriptorSets &descriptorSets = renderPass.descriptorSets();
        descriptorSets.addDescriptorSetCreateConfigWithJson(config.value()["descriptorSets"]["sets"]);
        descriptorSets.init();
        descriptorSets.writeSetsWithJson(config.value()["descriptorSets"]["write"]);
        renderPass.createPipelines(config.value()["pipelines"], vertexInfos, descriptorSetLayouts, viewportInfos);
    }

    for (auto &[pipelineName, renderPassName, descriptorSetName] : std::vector<std::array<std::string, 3>>{{"light", "G_Buffer", "G_BufferAttachments"},
                                                                                                           {"blur_h", "Bloom", "HighLight"},
                                                                                                           {"blur_v", "Bloom", "BlurSampler1"},
                                                                                                           {"blend", "Blend", "Output"},
                                                                                                           {"present", "Present", "FinalPresent"}})
    {
        std::vector<std::vector<std::pair<std::string, std::pair<std::string, uint32_t>>>> frameDescriptorSets(MAX_FLIGHT_COUNT);
        for (uint32_t i = 0; i < MAX_FLIGHT_COUNT; ++i)
            frameDescriptorSets[i] = {
                {dynamic_cast<Vkbase::RenderPass *>(_resourceManager.resource(Vkbase::VkResourceType::RenderPass, renderPassName))->descriptorSets().name(),
                 {descriptorSetName, i}}};
        _renderObjects.push_back(std::make_shared<RenderFrame>(deviceName(), frameDescriptorSets));

        _renderObjectManager.addObject(renderPassName, pipelineName, _renderObjects.back());
    }
}

void Render::createDescriptorSets()
{
    Vkbase::DescriptorSets *pDescriptorSets = createResource<Vkbase::DescriptorSets>("MainDescriptorSets", deviceName());
    Font::addProjectiveDescriptorSet(pDescriptorSets->name());
    pDescriptorSets->init();

    Font::writeProjectiveDescriptorSet(pDescriptorSets->name(), deviceName());
    Font::setScreenSize({800, 600});
}

void Render::createRenderDelegator()
{
    const Vkbase::Swapchain &swapchain = *dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::VkResourceType::Swapchain, "mainWindow"));

    _pRenderDelegator = createResource<Vkbase::RenderDelegator>("", deviceName(), swapchain.name(), "Graphics" + _deviceName);
    _pRenderDelegator->setCommandRecordFunc([this](Vkbase::CommandBuffer *pCommandBuffer, uint32_t imageIndex, uint32_t currentFrame)
                                            { this->recordCommand(pCommandBuffer, imageIndex, currentFrame); });
    _pRenderDelegator->setRenderPassCreateFunc([spRender = std::shared_ptr<Render>(this, [](Render *) {})]() { spRender->createRenderPass(); });
    _pRenderDelegator->setUpdateFunc([this](uint32_t currentFrame) { this->update(currentFrame); });
}

void Render::draw()
{
    glfwPollEvents();
    calcDeltaTime();
    Vkbase::Device::collectAllDelayResource();
    Event::KeyInputEvent::processing();

    if (_resourceManager.resources().count(Vkbase::VkResourceType::RenderDelegator))
    {
        const std::unordered_map<std::string, Vkbase::VkResourceBase *> resources = _resourceManager.resources().at(Vkbase::VkResourceType::RenderDelegator);
        for (auto renderDelegator : resources)
            dynamic_cast<Vkbase::RenderDelegator *>(renderDelegator.second)->draw();
    }

    Vkbase::Window::delayDestroy();
}

void Render::update(uint32_t currentFrame)
{
    // std::cout << 1 / _deltaTime << std::endl;
    _camera.updatePerspective();
    for (Modelbase::Model *pModel : Modelbase::Model::models())
    {
        // Modelbase::ModelInstance &instance = pModel->instance("1");
        pModel->updateAnimation(_deltaTime);
        // instance.updateUniformBuffers(currentFrame, _camera);
    }

    Entity::updateEntities(currentFrame);
}

void Render::cleanup() {}

void Render::recordCommand(Vkbase::CommandBuffer *pCommandBuffer, uint32_t imageIndex, uint32_t currentFrame)
{
    for (const auto &renderPassManager : _renderPassManagers)
        renderPassManager.draw(pCommandBuffer, &_renderObjectManager, imageIndex, currentFrame);
    // Vkbase::RenderPass &renderPass = *dynamic_cast<Vkbase::RenderPass *>(_resourceManager.resource(Vkbase::VkResourceType::RenderPass, "Normal"));

    // vk::CommandBuffer commandBuffer = pCommandBuffer->commandBuffer();

    // std::vector<vk::ClearValue> clearValues = {vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}), vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}),
    //                                            vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}), vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}),
    //                                            vk::ClearValue().setColor({1.0f, 1.0f, 1.0f, 1.0f}), vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}),
    //                                            vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}), vk::ClearValue().setColor({1.0f, 1.0f, 1.0f, 1.0f}),
    //                                            vk::ClearValue().setDepthStencil({1.0f, 0}),         vk::ClearValue().setDepthStencil({1.0f, 0})};

    // renderPass.begin(pCommandBuffer, "mainWindow_" + std::to_string(imageIndex), clearValues);

    // uint32_t subpass = 0;

    // Entity::drawEntities(pCommandBuffer, currentFrame,
    //                      std::vector<std::any>{subpass, std::unordered_map<std::string, std::vector<std::any>>{
    //                                                         {"Box", {std::string("GeometryPipeline"), std::string("UBO"), false}}}});
    // commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    // ++subpass;
    // Entity::drawEntities(pCommandBuffer, currentFrame,
    //                      std::vector<std::any>{subpass, std::unordered_map<std::string, std::vector<std::any>>{
    //                                                         {"Box", {std::string("GeometryShadow"), std::string("ShadowUBO"), true}}}});

    // uint32_t index = 0;
    // commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    // ++subpass;
    // _renderFrames[index++].onDraw(pCommandBuffer, imageIndex);

    // commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    // ++subpass;
    // _renderFrames[index++].onDraw(pCommandBuffer, imageIndex);

    // commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    // ++subpass;
    // _renderFrames[index++].draw(pCommandBuffer, imageIndex, currentFrame);

    // commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    // ++subpass;
    // _renderFrames[index++].onDraw(pCommandBuffer, imageIndex);

    // Vkbase::Pipeline &textPipeline = *dynamic_cast<Vkbase::Pipeline *>(_resourceManager.resource(Vkbase::VkResourceType::Pipeline, "text"));
    // pCommandBuffer->bindPipeline(&textPipeline);
    // if (auto pText = _pText.lock())
    //     pText->draw(pCommandBuffer, textPipeline, {Font::projectiveSet("MainDescriptorSets")});

    // renderPass.end(pCommandBuffer);
}

void Render::calcDeltaTime()
{
    _lastTime = _currentTime;
    _currentTime = glfwGetTime();
    _deltaTime = _currentTime - _lastTime;
}

bool Render::shouldEndApplication() { return _resourceManager.resources().count(Vkbase::VkResourceType::Window); }