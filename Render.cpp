#include "Render.h"
#include "Cloud.h"
#include "Data.h"
#include "Data/Octree.h"
#include "Engine/Entity/Character.h"
#include "Engine/Entity/Cube.h"
#include "Engine/Entity/ModelEntity.h"
#include "Engine/Entity/Motion/Collision.h"
#include "Engine/Entity/Motion/Friction.h"
#include "Engine/Entity/Motion/Gravity.h"
#include "Engine/Entity/Motion/Push.h"
#include "Engine/Physical/Collision/CollisionSystem.h"
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
    _cameraLight.movePosTo(50, 300, 40);
    _cameraLight.lookAt(glm::vec3(0.0f));
    _cameraLight.setLight(true);
    _cameraLight.setNearFar(-2000, 2000);
    _cameraLight.setFrameSize(glm::vec2(100.0f));
    Vkbase::Window *pResourcesWindow = createResource<Vkbase::Window>("resourceWindow", "Resources Window", 800, 600);
    Vkbase::Window *pWindow = createResource<Vkbase::Window>("mainWindow", "Vulkan Window", 800, 600);
    pWindow->setMouseMoveCallback([this](double x, double y) { Render::camera().addViewBy(x, -y); });
    pWindow->setMouseScrollCallback([](double, double y) { _speed = std::min(std::max(_speed + y * 0.1, 0.0), 30.0); });

    VertexData frameVertices[] = {{glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
                                  {glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
                                  {glm::vec3(1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
                                  {glm::vec3(1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
                                  {glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
                                  {glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)}};
    _pFrameVerticesBuffer = createResource<Vkbase::Buffer>("Vertex", "Device", sizeof(VertexData) * 6, vk::BufferUsageFlagBits::eVertexBuffer, frameVertices);

    if (!Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Image, "Empty"))
        createResource<Vkbase::Image>("Empty", "Device", 1, 1, 1, vk::Format::eR8G8B8A8Srgb, vk::ImageType::e2D, vk::ImageViewType::e2D,
                                      vk::ImageUsageFlagBits::eSampled, (uint32_t[]){0xFFFF00FF});

    createResource<Vkbase::Sampler>("Sampler", "Device");
    Cube *pCube1 = new Cube("1");
    pCube1->addMotion("Gravity", (Motion *)(new Gravity()));
    pCube1->addMotion("Collision", (Motion *)(new Collision()));
    pCube1->addMotion("Friction", (Motion *)(new Friction()));
    _pPush = new Push();
    pCube1->addMotion("Push", (Motion *)_pPush);
    pCube1->object().setPosition(glm::vec3(5.0f, 200.0f, 0.0f));
    pCube1->object().setScale(glm::vec3(5.0f, 13.0f, 5.0f));

    Cube *pCube4 = new Cube("4");
    pCube4->addMotion("Gravity", (Motion *)(new Gravity()));
    pCube4->addMotion("Collision", (Motion *)(new Collision()));
    pCube4->addMotion("Friction", (Motion *)(new Friction()));
    pCube4->object().setPosition(glm::vec3(5.0f, 50.0f, 0.0f));
    pCube4->object().setScale(glm::vec3(5.0f, 13.0f, 5.0f));

    Object cubeObject2;
    cubeObject2.setScale(glm::vec3(100.0f, 0.5f, 100.0f));
    cubeObject2.setPosition(glm::vec3(0.0f, -10.0f, 0.0f));
    Cube *pCube2 = new Cube("2", false, cubeObject2);

    Object cubeObject3;
    cubeObject3.setScale(glm::vec3(50.0f, 0.5f, 50.0f));
    cubeObject3.setPosition(glm::vec3(0.0f, -9.5f, 0.0f));
    Cube *pCube3 = new Cube("3", false, cubeObject3);

    Object cubeObject5;
    cubeObject5.setPosition(glm::vec3(30.0f, 30.0f, 30.0f));
    ModelEntity *pCharacter = new ModelEntity("5", false, cubeObject5, JsonConfigReader::load("config/model.json")[0]);

    // pCharacter->modelObject().setScale(glm::vec3(0.01f));

    // auto pOctrees = std::vector{CollisionSystem::instance().octree()};
    // int outlineIndex = 0;
    // while (pOctrees.size())
    // {
    //     auto &pCollisionOctree = *pOctrees.back();
    //     pOctrees.pop_back();

    //     if (pCollisionOctree.objects().size())
    //     {
    //         uint32_t level = pCollisionOctree.level();
    //         uint32_t x, y, z;
    //         x = pCollisionOctree.x();
    //         y = pCollisionOctree.y();
    //         z = pCollisionOctree.z();
    //         float powLevel2 = 1u << level;
    //         Object octreeBoundary;
    //         glm::vec3 postion = glm::vec3(-1000.0f), size = glm::vec3(2000.0f);
    //         octreeBoundary.setPosition(postion + size * glm::vec3(x + 0.5f, y + 0.5f, z + 0.5f) / (float)powLevel2);
    //         octreeBoundary.setScale(size / (float)powLevel2);
    //         new Cube("Outline" + std::to_string(outlineIndex++), false, octreeBoundary, true);
    //     }

    //     if (pCollisionOctree.hasSubTrees())
    //         for (uint8_t pos = 0; pos < 8; ++pos)
    //             pOctrees.push_back(pCollisionOctree.subTree(pos).get());
    // }

    // delete new Cloud();

    // Modelbase::Model *pModel =
    //     new Modelbase::Model("Device", dynamic_cast<const Vkbase::Sampler *>(_resourceManager.resource(Vkbase::ResourceType::Sampler, "Sampler"))->sampler(),
    //                          JsonConfigReader::load("config/model.json")[0]);
    // Modelbase::ModelInstance &instance = pModel->createNewInstance("1", {0, 0.0f});
    // Object &modelObject = instance.object();
    // modelObject.setScale(glm::vec3(0.01f));

    _pFont = std::make_unique<Font>("Device", "./src/fonts/Minecraft.ttf");
    _pText = std::make_unique<Text>(*_pFont, "Hello Vulkan!", glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(10.0f, 50.0f), 1.0f);

    createDescriptorSets();
    createRenderPass();
}

void Render::initWindowEvents()
{
    Event::KeyInputEvent &event = dynamic_cast<Vkbase::Window *>(_resourceManager.resource(Vkbase::ResourceType::Window, "mainWindow"))->keyInputEvent();
    Object &Box1 = Entity::entity<Cube>("1")->object();
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

    Vkbase::RenderPass &renderPass =
        *(createResource<Vkbase::RenderPass>("mainWindow", "Device", JsonConfigReader::load("config/render.json")[0]["renderPass"], "mainWindow", depthFormat));
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
        {"blend", screenVertexInfo},       {"blur_h", screenVertexInfo},  {"blur_v", screenVertexInfo},         {"light", screenVertexInfo},
        {"text", textVertexInfo},          {"g_buffer", modelVertexInfo}, {"GeometryPipeline", cubeVertexInfo}, {"GeometryOutlinePipeline", cubeVertexInfo},
        {"GeometryShadow", cubeVertexInfo}};

    const std::unordered_map<std::string, std::vector<vk::DescriptorSetLayout>> descriptorSetLayouts = {
        {"blend", {descriptorSets.layout("BlendInputAttachments")}},
        {"blur_h", {descriptorSets.layout("BlurSampler1")}},
        {"blur_v", {descriptorSets.layout("BlurSampler2")}},
        {"light", {descriptorSets.layout("G_BufferInputAttachments")}},
        {"text", {_pFont->layout(), Font::projectiveLayout("MainDescriptorSets")}},
        {"g_buffer", {(*Modelbase::Model::models().begin())->descriptorSetLayout(0, "g_buffer")}},
        {"GeometryPipeline", Entity::entity<Cube>("1")->descriptorSetLayouts()},
        {"GeometryOutlinePipeline", Entity::entity<Cube>("1")->descriptorSetLayouts()},
        {"GeometryShadow", Entity::entity<Cube>("1")->descriptorSetLayouts()}};

    const std::pair<std::vector<vk::Rect2D>, std::vector<vk::Viewport>> viewportInfo = {{vk::Rect2D().setExtent(swapchain.extent())},
                                                                                        {vk::Viewport().setWidth(extent.width).setHeight(extent.height)}};

    const std::unordered_map<std::string, std::pair<std::vector<vk::Rect2D>, std::vector<vk::Viewport>>> viewportInfos = {
        {"blend", viewportInfo},         {"blur_h", viewportInfo},   {"blur_v", viewportInfo},           {"light", viewportInfo},
        {"text", viewportInfo},          {"g_buffer", viewportInfo}, {"GeometryPipeline", viewportInfo}, {"GeometryOutlinePipeline", viewportInfo},
        {"GeometryShadow", viewportInfo}};

    renderPass.createPipelines(JsonConfigReader::load("config/render.json")[0]["pipelines"], vertexInfos, descriptorSetLayouts, viewportInfos);

    createRenderDelegator();
}

void Render::createDescriptorSets()
{
    Vkbase::DescriptorSets *pDescriptorSets = createResource<Vkbase::DescriptorSets>("MainDescriptorSets", "Device");
    Font::addProjectiveDescriptorSet(pDescriptorSets->name());
    pDescriptorSets->init();

    Font::writeProjectiveDescriptorSet(pDescriptorSets->name(), "Device");
    Font::setScreenSize({800, 600});
}

void Render::createRenderDelegator()
{
    const Vkbase::Swapchain &swapchain = *dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow"));

    _pRenderDelegator = createResource<Vkbase::RenderDelegator>("MainRender", "Device", swapchain.name(), "GraphicsDevice");
    _pRenderDelegator->setCommandRecordFunc([this](Vkbase::CommandBuffer *pCommandBuffer, uint32_t imageIndex, uint32_t currentFrame)
                                            { this->recordCommand(pCommandBuffer, imageIndex, currentFrame); });
    _pRenderDelegator->setRenderPassCreateFunc([this]() { this->createRenderPass(); });
}

void Render::draw()
{
    glfwPollEvents();
    clacDeltaTime();
    Vkbase::Device::collectAllDelayResource();
    Event::KeyInputEvent::processing();
    _camera.updatePerspective();
    _cameraLight.lookAt(glm::vec3(0.0f));
    _cameraLight.updatePerspective();

    if (_resourceManager.resources().count(Vkbase::ResourceType::RenderDelegator))
    {
        const std::unordered_map<std::string, Vkbase::ResourceBase *> resources = _resourceManager.resources().at(Vkbase::ResourceType::RenderDelegator);
        for (auto renderDelegator : resources)
            dynamic_cast<Vkbase::RenderDelegator *>(renderDelegator.second)->draw();
    }
    Vkbase::Window::delayDestroy();
}

void Render::cleanup() {}

void Render::recordCommand(Vkbase::CommandBuffer *pCommandBuffer, uint32_t imageIndex, uint32_t currentFrame)
{
    _pText->setText(std::to_string(_speed));
    vk::CommandBuffer commandBuffer = pCommandBuffer->commandBuffer();

    Vkbase::RenderPass &renderPass = *dynamic_cast<Vkbase::RenderPass *>(_resourceManager.resource(Vkbase::ResourceType::RenderPass, "mainWindow"));
    Vkbase::DescriptorSets &descriptorSets = renderPass.descriptorSets();

    std::vector<vk::ClearValue> clearValues = {vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}), vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}),
                                               vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}), vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}),
                                               vk::ClearValue().setColor({1.0f, 1.0f, 1.0f, 1.0f}), vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}),
                                               vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}), vk::ClearValue().setColor({1.0f, 1.0f, 1.0f, 1.0f}),
                                               vk::ClearValue().setDepthStencil({1.0f, 0}),         vk::ClearValue().setDepthStencil({1.0f, 0})};

    vk::Extent2D extent = dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow"))->extent();
    renderPass.begin(
        pCommandBuffer,
        dynamic_cast<Vkbase::Framebuffer *>(_resourceManager.resource(Vkbase::ResourceType::Framebuffer, "mainWindow_" + std::to_string(imageIndex))),
        clearValues, extent);

    for (Modelbase::Model *pModel : Modelbase::Model::models())
    {
        // Modelbase::ModelInstance &instance = pModel->instance("1");

        pModel->updateAnimation(_deltaTime);
        // instance.updateUniformBuffers(currentFrame, _camera);
        // pModel->draw(currentFrame, commandBuffer, 0);
    }

    Entity::drawEntities(pCommandBuffer, _camera, _cameraLight.perspective() * _cameraLight.view(), currentFrame, "GeometryPipeline", "UBO", "UBO");
    Entity::drawEntities(pCommandBuffer, _camera, _cameraLight.perspective() * _cameraLight.view(), currentFrame, "GeometryOutlinePipeline", "UBO", "UBO");
    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    // Entity::drawEntities(commandBuffer, _cameraLight, _cameraLight.perspective() * _cameraLight.view(), currentFrame, "GeometryShadow", "Shadow_UBO",
    //                      "ShadowUBO");

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    renderFrame(pCommandBuffer, "light", {{&descriptorSets, {"G_BufferInputAttachments", imageIndex}}});

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    renderFrame(pCommandBuffer, "blur_h", {{&descriptorSets, {"BlurSampler1", imageIndex}}});

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    renderFrame(pCommandBuffer, "blur_v", {{&descriptorSets, {"BlurSampler2", imageIndex}}});

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    renderFrame(pCommandBuffer, "blend", {{&descriptorSets, {"BlendInputAttachments", imageIndex}}});

    Vkbase::Pipeline &textPipeline = *dynamic_cast<Vkbase::Pipeline *>(_resourceManager.resource(Vkbase::ResourceType::Pipeline, "text"));
    pCommandBuffer->bindPipeline(&textPipeline);
    _pText->draw(pCommandBuffer, textPipeline, {Font::projectiveSet("MainDescriptorSets")});

    renderPass.end(pCommandBuffer);
}

void Render::renderFrame(Vkbase::CommandBuffer *pCommandBuffer, const std::string &pipelineName, const std::vector<std::pair<Vkbase::DescriptorSets *, std::pair<std::string, uint32_t>>> &descriptorSet)
{
    pCommandBuffer->bindPipeline(dynamic_cast<Vkbase::Pipeline *>(_resourceManager.resource(Vkbase::ResourceType::Pipeline, pipelineName)));
    pCommandBuffer->bindDescriptorSets(0, descriptorSet, {});
    pCommandBuffer->bindVertexBuffers(0, {_pFrameVerticesBuffer}, {0});
    pCommandBuffer->commandBuffer().draw(6, 1, 0, 0);
}

Camera &Render::camera() { return _camera; }

void Render::clacDeltaTime()
{
    _lastTime = _currentTime;
    _currentTime = glfwGetTime();
    _deltaTime = _currentTime - _lastTime;
}

bool Render::shouldEndApplication() { return _resourceManager.resources().count(Vkbase::ResourceType::Window); }