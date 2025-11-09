#include "Render.h"
#include "../Data.h"
#include "../Engine/Entity/Character.h"
#include "../Engine/Entity/Cube.h"
#include "../Engine/Entity/ModelEntity.h"
#include "../Engine/Entity/Motion/Collision.h"
#include "../Engine/Entity/Motion/Friction.h"
#include "../Engine/Entity/Motion/Gravity.h"
#include "../Engine/Entity/Motion/Push.h"
#include "../Engine/Physical/Collision/CollisionSystem.h"
#include "../InputEvent/KeyInputEvent.h"
#include "../JsonConfigReader/JsonConfigReader.h"
#include "../Modelbase/Modelbase.h"

#include "../Resources/FontResource.h"
#include "../Resources/ModelResource.h"
#include "../Resources/ResourceManager.h"

#include "../VkGUI/Frame.h"
#include "../VkGUI/Text.h"
#include "../VkGUI/Widget.h"

#include <fstream>

Render::Render(const std::string &windowName)
{
    _window = createResource<Vkbase::Window>(windowName, windowName, 800, 600);
    if (auto pWindow = _window.lock<Vkbase::Window>())
    {
        if (auto pDevice = pWindow->device().lock())
            _deviceName = pDevice->name();
        pWindow->setMouseMoveCallback([this](double x, double y) { _camera.addViewBy(x, -y); });
        pWindow->setMouseScrollCallback([](double, double y) { _speed = std::min(std::max(_speed + y * 0.1, 0.0), 30.0); });
    }

    _cameraLight.movePosTo(50, 300, 40);
    _cameraLight.lookAt(glm::vec3(0.0f));
    _cameraLight.setLight(true);
    _cameraLight.setNearFar(-1000, 1000);
    _cameraLight.setFrameSize(glm::vec2(300.0f));

    json renderPassConfig = JsonConfigReader::load("./config/renderPass.json")[0];
    _renderPassManagers.emplace_back(renderPassConfig);
    _renderPassManagers.back().clearValue("Normal")[4].depthStencil.setDepth(1.0f);
    _renderPassManagers.back().clearValue("Shadow")[0].depthStencil.setDepth(1.0f);

    if (!Vkbase::VkResourceBase::resourceManager().resource(Vkbase::VkResourceType::Image, "Empty").lock())
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

    for (auto &object : _renderObjects)
    {
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

    _renderObjectManager.addObject("Normal", "g_buffer", _renderObjects.back());

    _rootFrameIndex = _renderObjects.size();
    _renderObjects.push_back(VkGUI::Widget::create<VkGUI::Frame>(_window, deviceName()));
    _renderObjectManager.addObject("Blend", "Widget", _renderObjects.back());
    std::dynamic_pointer_cast<VkGUI::Widget>(_renderObjects.back())->setRect({0, 0, 500, 1600});
    std::dynamic_pointer_cast<VkGUI::Widget>(_renderObjects.back())->setColor({0.0f, 0.0f, 0.0f, 0.5f});
    std::dynamic_pointer_cast<VkGUI::Widget>(_renderObjects.back())
        ->setCommandShouldRecordFunc([this]() { this->_renderPassManagers.back().shouldRecordFor("Widget"); });
    _fps = std::dynamic_pointer_cast<VkGUI::Widget>(_renderObjects.back())
               ->create<VkGUI::Text>(
                   Resources::ResourceManager::instance().getResource<Resources::FontResource>(deviceName(), "./src/fonts/AaXuanYuanTi-2.ttf", 48).font());
    if (auto p = _fps.lock())
    {
        p->setText("测试");
        p->setTextColor(glm::vec4(1.0f));
    }

    createDescriptorSets();
    createRenderPass();
    createRenderDelegator();

    initWindowEvents();
}

const std::string &Render::deviceName() { return _deviceName; }

void Render::initWindowEvents()
{
    if (auto pWindow = _resourceManager.resource(Vkbase::VkResourceType::Window, "mainWindow").lock<Vkbase::Window>())
    {
        InputEvent::KeyInputEvent &event = pWindow->keyInputEvent();
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
                event.addPressedKeyEvent(GLFW_KEY_LEFT_SHIFT,
                                         [&]() { Box1.setPosition(Box1.position() + glm::vec3(0.0f, -1.0f, 0.0f) * _speed * (_deltaTime)); });
            });

        event.addPressedKeyEvent(GLFW_KEY_SPACE, [&]() { _camera.moveUp(_speed * (_deltaTime)); });
        event.addPressedKeyEvent(GLFW_KEY_LEFT_SHIFT, [&]() { _camera.moveDown(_speed * (_deltaTime)); });
        event.addDownKeyEvent(GLFW_KEY_ESCAPE,
                              []()
                              {
                                  if (auto pWindow = _resourceManager.resource(Vkbase::VkResourceType::Window, "mainWindow").lock<Vkbase::Window>())
                                      pWindow->switchCursorState();
                              });
    }
}

void Render::createRenderPass()
{
    vk::Format depthFormat;
    vk::Extent2D extent;
    if (auto pDevice = _resourceManager.resource(Vkbase::VkResourceType::Device, deviceName()).lock<Vkbase::Device>())
        depthFormat = pDevice->findSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                                                   vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
    if (auto pWindow = _window.lock<Vkbase::Window>())
        if (auto p = _resourceManager.resource(Vkbase::VkResourceType::Swapchain, pWindow->name()).lock<Vkbase::Swapchain>())
            extent = p->extent();

    json renderPassConfigs = JsonConfigReader::load("config/render.json");
    std::vector<std::string> renderPassNames;
    renderPassNames.reserve(renderPassConfigs.size());

    Vkbase::VertexInfo modelVertexInfo(ModelData::Vertex::attributeDescriptions(), {ModelData::Vertex::bindingDescription()});
    Vkbase::VertexInfo screenVertexInfo(RenderFrame::ScreenVertexData::attributeDescriptions(), {RenderFrame::ScreenVertexData::bindingDescription()});
    Vkbase::VertexInfo cubeVertexInfo(GeometryVertexData::attributeDescriptions(), {GeometryVertexData::bindingDescription()});
    Vkbase::VertexInfo guiVertexInfo(VkGUI::Widget::Vertex::attributeDescriptions(), {VkGUI::Widget::Vertex::bindingDescription()});

    const std::unordered_map<std::string, Vkbase::VertexInfo> vertexInfos = {
        {"blend", screenVertexInfo},       {"blur_h", screenVertexInfo},         {"blur_v", screenVertexInfo},
        {"light", screenVertexInfo},       {"present", screenVertexInfo},        {"Widget", guiVertexInfo},
        {"g_buffer", modelVertexInfo},     {"GeometryPipeline", cubeVertexInfo}, {"GeometryOutlinePipeline", cubeVertexInfo},
        {"GeometryShadow", cubeVertexInfo}};

    const std::pair<std::vector<vk::Rect2D>, std::vector<vk::Viewport>> viewportInfo = {{vk::Rect2D().setExtent(extent)},
                                                                                        {vk::Viewport().setWidth(extent.width).setHeight(extent.height)}};

    const std::unordered_map<std::string, std::pair<std::vector<vk::Rect2D>, std::vector<vk::Viewport>>> viewportInfos = {
        {"blend", viewportInfo},         {"blur_h", viewportInfo},           {"blur_v", viewportInfo},
        {"light", viewportInfo},         {"present", viewportInfo},          {"Widget", viewportInfo},
        {"g_buffer", viewportInfo},      {"GeometryPipeline", viewportInfo}, {"GeometryOutlinePipeline", viewportInfo},
        {"GeometryShadow", viewportInfo}};

    const std::unordered_map<std::string, std::vector<vk::DescriptorSetLayout>> descriptorSetLayouts = {
        {"blend", {}},
        {"blur_h", {}},
        {"blur_v", {}},
        {"light", {}},
        {"present", {}},
        {"Widget", {std::dynamic_pointer_cast<VkGUI::Widget>(_renderObjects[_rootFrameIndex])->descriptorSetsLayout()}},
        {"g_buffer", {(*Modelbase::Model::models().begin())->descriptorSetLayout(0, "g_buffer")}},
        {"GeometryPipeline", Entity::entity<Cube>("1")->descriptorSetLayouts()},
        {"GeometryOutlinePipeline", Entity::entity<Cube>("1")->descriptorSetLayouts()},
        {"GeometryShadow", Entity::entity<Cube>("1")->descriptorSetLayouts()}};
    if (auto pWindow = _window.lock())
        for (const auto &config : renderPassConfigs.items())
        {
            if (auto pRenderPass = createResource<Vkbase::RenderPass>(config.key(), deviceName(), config.value()["renderPass"], pWindow->name(), depthFormat)
                                       .lock<Vkbase::RenderPass>())
            {
                renderPassNames.push_back(pRenderPass->name());
                pRenderPass->createFramebuffer(config.key(), config.value()["framebuffers"], extent.width, extent.height, pWindow->name(), depthFormat);
            }
        }

    for (const auto &config : renderPassConfigs.items())
    {
        if (auto pRenderPass = _resourceManager.resource(Vkbase::VkResourceType::RenderPass, config.key()).lock<Vkbase::RenderPass>())
        {
            Vkbase::VkResourceManagerHolder::WeakReference descriptorSets = pRenderPass->descriptorSets();
            if (auto pDescriptorSets = pRenderPass->descriptorSets().lock<Vkbase::DescriptorSets>())
            {
                pDescriptorSets->addDescriptorSetCreateConfigWithJson(config.value()["descriptorSets"]["sets"]);
                pDescriptorSets->init();
                pDescriptorSets->writeSetsWithJson(config.value()["descriptorSets"]["write"]);
            }
            pRenderPass->createPipelines(config.value()["pipelines"], vertexInfos, descriptorSetLayouts, viewportInfos);
        }
    }

    for (auto &[pipelineName, renderPassName, descriptorSetName] : std::vector<std::array<std::string, 3>>{{"light", "G_Buffer", "G_BufferAttachments"},
                                                                                                           {"blur_h", "Bloom", "HighLight"},
                                                                                                           {"blur_v", "Bloom", "BlurSampler1"},
                                                                                                           {"blend", "Blend", "Output"},
                                                                                                           {"present", "Present", "FinalPresent"}})
    {
        std::vector<std::vector<std::pair<std::string, std::pair<std::string, uint32_t>>>> frameDescriptorSets(MAX_FLIGHT_COUNT);
        for (uint32_t i = 0; i < MAX_FLIGHT_COUNT; ++i)
            if (auto pRenderPass = _resourceManager.resource(Vkbase::VkResourceType::RenderPass, renderPassName).lock<Vkbase::RenderPass>())
                if (auto pDescriptorSets = pRenderPass->descriptorSets().lock())
                    frameDescriptorSets[i] = {{pDescriptorSets->name(), {descriptorSetName, i}}};
        _renderObjects.push_back(std::make_shared<RenderFrame>(deviceName(), frameDescriptorSets));

        _renderObjectManager.addObject(renderPassName, pipelineName, _renderObjects.back());
    }

    VkGUI::Widget::setScreenSize(glm::uvec2(extent.width, extent.height));
}

void Render::createDescriptorSets()
{
    if (auto p = createResource<Vkbase::DescriptorSets>("MainDescriptorSets", deviceName()).lock<Vkbase::DescriptorSets>())
        p->init();
}

void Render::createRenderDelegator()
{
    Vkbase::VkResourceManagerHolder::WeakReference swapchain = _resourceManager.resource(Vkbase::VkResourceType::Swapchain, "mainWindow");
    if (auto p = swapchain.lock())
        _renderDelegator = createResource<Vkbase::RenderDelegator>("", deviceName(), p->name(), "Graphics" + _deviceName);
    if (auto pRenderDelegator = _renderDelegator.lock<Vkbase::RenderDelegator>())
    {
        pRenderDelegator->setCommandRecordFunc([this](const Vkbase::VkResourceManagerHolder::WeakReference commandBuffer, uint32_t imageIndex,
                                                      uint32_t currentFrame) { this->recordCommand(commandBuffer, imageIndex, currentFrame); });
        pRenderDelegator->setRenderPassCreateFunc([spRender = std::shared_ptr<Render>(this, [](Render *) {})]() { spRender->createRenderPass(); });
        pRenderDelegator->setUpdateFunc([this](uint32_t currentFrame) { this->update(currentFrame); });
        pRenderDelegator->setSwapchainRecreatePrefunc(
            [this]()
            {
                for (auto &renderPassManager : this->_renderPassManagers)
                    renderPassManager.shouldRecordFor();
            });
    }
}

void Render::draw()
{
    glfwPollEvents();
    calcDeltaTime();
    Vkbase::Device::collectAllDelayResource();
    InputEvent::KeyInputEvent::processing();
    InputEvent::MouseInputEvent::processing();

    if (_resourceManager.resources().count(Vkbase::VkResourceType::RenderDelegator))
    {
        const std::unordered_map<std::string, Vkbase::VkResourceBase *> &resources =
            _resourceManager.resources().at(Vkbase::VkResourceType::RenderDelegator);
        for (auto &renderDelegator : resources)
            if (auto p = renderDelegator.second->weakReference().lock())
                dynamic_cast<Vkbase::RenderDelegator *>(p)->draw();
    }

    Vkbase::Window::delayDestroy();
}

void Render::update(uint32_t currentFrame)
{
    _camera.updatePerspective();
    for (Modelbase::Model *pModel : Modelbase::Model::models())
        pModel->updateAnimation(_deltaTime);

    for (auto object : _renderObjects)
        object->update(currentFrame);

    // if (auto p = _fps.lock())
    // {
    //     if (currentFrame == 0)
    //     {
    //         p->setText(std::to_string((int)(1 / _deltaTime)));
    //     }
    // }
}

void Render::cleanup() {}

void Render::recordCommand(const Vkbase::VkResourceManagerHolder::WeakReference &commandBuffer, uint32_t imageIndex, uint32_t currentFrame)
{
    for (auto &renderPassManager : _renderPassManagers)
    {
        renderPassManager.shouldRecordFor("g_buffer");
        renderPassManager.draw(commandBuffer, &_renderObjectManager, imageIndex, currentFrame);
    }
}

void Render::calcDeltaTime()
{
    _lastTime = _currentTime;
    _currentTime = glfwGetTime();
    _deltaTime = _currentTime - _lastTime;
}

bool Render::shouldEndApplication() { return _resourceManager.resources().count(Vkbase::VkResourceType::Window); }