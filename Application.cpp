#include "Application.h"
#include "Vkbase/Vkbase.h"
#include "Modelbase/Modelbase.h"
#include "Data.h"
#define SPEED 10.0f

Camera Application::_camera({800, 600});

Application::Application()
    : _resourceManager(Vkbase::ResourceBase::resourceManager())
{
    init();
}

void Application::init()
{
    new Vkbase::Window("mainWindow1", "Vulkan Window", 800, 600);
    Vkbase::Window *pWindow = new Vkbase::Window("mainWindow", "Vulkan Window", 800, 600);
    pWindow->setMouseMoveCallback([this](double x, double y)
                                  { this->_camera.addViewBy(x, -y); });

    VertexData vertices[] = {{glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
                             {glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
                             {glm::vec3(1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
                             {glm::vec3(1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
                             {glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
                             {glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)}};

    new Vkbase::Buffer("Vertex", "0", sizeof(VertexData) * 6, vk::BufferUsageFlagBits::eVertexBuffer, vertices);
    new Vkbase::Buffer("UBO0", "0", sizeof(UniformBufferData), vk::BufferUsageFlagBits::eUniformBuffer);
    new Vkbase::Buffer("UBO1", "0", sizeof(UniformBufferData), vk::BufferUsageFlagBits::eUniformBuffer);
    new Vkbase::Buffer("UBO2", "0", sizeof(UniformBufferData), vk::BufferUsageFlagBits::eUniformBuffer);
    new Vkbase::Sampler("Sampler", "0");
    new Vkbase::Image("Texture", "0", "./src/texture1.jpg", vk::Format::eR8G8B8A8Srgb, vk::ImageType::e2D, vk::ImageViewType::e2D, vk::ImageUsageFlagBits::eSampled);

    auto pModel = new Modelbase::Model("0", "Graphics0", "src/model/zhongli-from-genshin-impact/ze+hongli.fbx", 1, dynamic_cast<const Vkbase::Sampler *>(_resourceManager.resource(Vkbase::ResourceType::Sampler, "Sampler"))->sampler());
    pModel->createNewInstance("1", {0, 0.0f});

    createDescriptorSets();
    createRenderPass();
}

void Application::updateUniformBuffer(Vkbase::ResourceManager &resourceManager, uint32_t index)
{
    _camera.updatePerspective();
    UniformBufferData cameraData;
    cameraData.view = _camera.view();
    cameraData.proj = _camera.perspective();
    cameraData.model = glm::scale(glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)), glm::vec3(1.0f, 1.0f, 1.0f));

    dynamic_cast<Vkbase::Buffer *>(resourceManager.resource(Vkbase::ResourceType::Buffer, "UBO" + std::to_string(index)))->updateBufferData(&cameraData);
}

void Application::createRenderPass()
{
    const Vkbase::Device &device = *(dynamic_cast<const Vkbase::Device *>(_resourceManager.resource(Vkbase::ResourceType::Device, "0")));
    vk::Format depthFormat = device.findSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint}, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);

    const Vkbase::Swapchain &swapchain = *dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow"));
    const Vkbase::Swapchain &swapchain1 = *dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow1"));

    std::vector<vk::AttachmentDescription> attachments(3);
    attachments[0].setFormat(swapchain.format()).setSamples(vk::SampleCountFlagBits::e1).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore).setStencilLoadOp(vk::AttachmentLoadOp::eDontCare).setStencilStoreOp(vk::AttachmentStoreOp::eDontCare).setInitialLayout(vk::ImageLayout::eUndefined).setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    attachments[1].setFormat(swapchain1.format()).setSamples(vk::SampleCountFlagBits::e1).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore).setStencilLoadOp(vk::AttachmentLoadOp::eDontCare).setStencilStoreOp(vk::AttachmentStoreOp::eDontCare).setInitialLayout(vk::ImageLayout::eUndefined).setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    attachments[2].setFormat(depthFormat).setSamples(vk::SampleCountFlagBits::e1).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eDontCare).setStencilLoadOp(vk::AttachmentLoadOp::eDontCare).setStencilStoreOp(vk::AttachmentStoreOp::eDontCare).setInitialLayout(vk::ImageLayout::eUndefined).setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    std::vector<vk::AttachmentReference> colorAttachmentRefs(2);
    colorAttachmentRefs[0].setAttachment(0).setLayout(vk::ImageLayout::eColorAttachmentOptimal);
    colorAttachmentRefs[1].setAttachment(1).setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::AttachmentReference depthStencilRef;
    depthStencilRef.setAttachment(2)
        .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::SubpassDescription subpass;
    subpass.setColorAttachments(colorAttachmentRefs)
        .setPDepthStencilAttachment(&depthStencilRef);

    std::vector<vk::SubpassDependency> dependencies;
    {
        vk::SubpassDependency &info = dependencies.emplace_back();
        info.setSrcSubpass(vk::SubpassExternal).setDstSubpass(0).setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput).setSrcAccessMask(vk::AccessFlagBits::eNone).setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput).setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);
    }

    vk::RenderPassCreateInfo createInfo;
    createInfo.setAttachments(attachments)
        .setDependencies(dependencies)
        .setSubpasses(subpass);

    const Vkbase::RenderPass &renderPass = *(new Vkbase::RenderPass("mainWindow", "0", createInfo));

    // --------------------------------

    vk::Extent2D extent = swapchain.extent();
    for (uint32_t i = 0; i < swapchain.imageNames().size(); ++i)
    {
        Vkbase::Image &depthImage = *(new Vkbase::Image("Depth_" + std::to_string(i), "0", extent.width, extent.height, 1, depthFormat, vk::ImageType::e2D, vk::ImageViewType::e2D, vk::ImageUsageFlagBits::eDepthStencilAttachment));
        renderPass.createFramebuffer("mainWindow_" + std::to_string(i), {swapchain.imageNames()[i], swapchain1.imageNames()[i], depthImage.name()}, extent.width, extent.height);
    }

    // --------------------------------
    std::vector<Vkbase::ShaderInfo> shaderInfo = {Vkbase::ShaderInfo("shader/baseShaderVert.spv", "main", vk::ShaderStageFlagBits::eVertex),
                                                  Vkbase::ShaderInfo("shader/baseShaderFrag.spv", "main", vk::ShaderStageFlagBits::eFragment)};

    Vkbase::VertexInfo vertexInfo(ModelData::Vertex::attributeDescriptions(), {ModelData::Vertex::bindingDescription()});

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
    // const Vkbase::DescriptorSets &descriptorSets = *dynamic_cast<const Vkbase::DescriptorSets *>(_resourceManager.resource(Vkbase::ResourceType::DescriptorSets, "UBO"));
    // descriptorSetLayouts.push_back(descriptorSets.layout("Camera"));
    // descriptorSetLayouts.push_back(descriptorSets.layout("Texture"));
    std::vector<vk::DescriptorSetLayout> layout = (*Modelbase::Model::models().begin())->descriptorSetLayout(0);
    descriptorSetLayouts.insert(descriptorSetLayouts.end(), layout.begin(), layout.end());

    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.setBlendEnable(vk::False)
        .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

    Vkbase::PipelineRenderInfo renderInfo = Vkbase::Pipeline::getDefaultRenderInfo();
    renderInfo.blendAttachments.push_back(colorBlendAttachment);
    renderInfo.blendAttachments.push_back(colorBlendAttachment);
    renderInfo.depthStencilStateInfo.setDepthTestEnable(vk::True)
        .setDepthWriteEnable(vk::True)
        .setDepthCompareOp(vk::CompareOp::eLess)
        .setDepthBoundsTestEnable(vk::False)
        .setMinDepthBounds(0.0f)
        .setMaxDepthBounds(1.0f)
        .setStencilTestEnable(vk::False);

    Vkbase::PipelineCreateInfo pipelineCreateInfo(shaderInfo, vertexInfo, descriptorSetLayouts, renderInfo);
    renderPass.createPipeline("base", pipelineCreateInfo);
    createRenderDelegator();
}

void Application::createDescriptorSets()
{
    Vkbase::DescriptorSets *pDescriptorSets = new Vkbase::DescriptorSets("UBO", "0");
    pDescriptorSets->addDescriptorSetCreateConfig("Camera", {{vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex}}, Vkbase::RenderDelegator::maxFlightCount());
    pDescriptorSets->addDescriptorSetCreateConfig("Texture", {{vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment}}, 1);
    pDescriptorSets->init();

    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.setOffset(0)
        .setRange(sizeof(UniformBufferData));

    {
        vk::DescriptorImageInfo imageInfo;
        imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setImageView(dynamic_cast<const Vkbase::Image *>(_resourceManager.resource(Vkbase::ResourceType::Image, "Texture"))->view())
            .setSampler(dynamic_cast<const Vkbase::Sampler *>(_resourceManager.resource(Vkbase::ResourceType::Sampler, "Sampler"))->sampler());
        std::vector<vk::DescriptorImageInfo> imageInfos(MAX_FLIGHT_COUNT, imageInfo);
        pDescriptorSets->writeSets("Texture", 0, {}, imageInfos, 1);
    }

    {
        std::vector<vk::DescriptorBufferInfo> bufferInfos(Vkbase::RenderDelegator::maxFlightCount(), bufferInfo);
        for (int i = 0; i < Vkbase::RenderDelegator::maxFlightCount(); ++i)
            bufferInfos[i].setBuffer(dynamic_cast<Vkbase::Buffer *>(_resourceManager.resource(Vkbase::ResourceType::Buffer, "UBO" + std::to_string(i)))->buffer());
        pDescriptorSets->writeSets("Camera", 0, bufferInfos, {}, Vkbase::RenderDelegator::maxFlightCount());
    }
}

void Application::createRenderDelegator()
{
    const Vkbase::Swapchain &swapchain = *dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow"));
    const Vkbase::Swapchain &swapchain1 = *dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow1"));

    _pRenderDelegator = new Vkbase::RenderDelegator("mainRender", "0", {swapchain.name(), swapchain1.name()}, "Graphics0");
    _pRenderDelegator->setCommandRecordFunc([this](const vk::CommandBuffer &commandBuffer, uint32_t imageIndex, uint32_t currentFrame)
                                            { this->recordCommand(commandBuffer, imageIndex, currentFrame); });
    _pRenderDelegator->setRenderPassCreateFunc([this]()
                                               { this->createRenderPass(); });
}

void Application::run()
{
    mainLoop();
    cleanup();
}

void Application::mainLoop()
{
    while (_resourceManager.resources().count(Vkbase::ResourceType::Window))
    {
        glfwPollEvents();
        _lastTime = _currentTime;
        _currentTime = glfwGetTime();
        _deltaTime = _currentTime - _lastTime;
        processInputs();

        if (_resourceManager.resources().count(Vkbase::ResourceType::RenderDelegator))
        {
            const std::unordered_map<std::string, Vkbase::ResourceBase *> resources = _resourceManager.resources().at(Vkbase::ResourceType::RenderDelegator);
            for (auto renderDelegator : resources)
                dynamic_cast<Vkbase::RenderDelegator *>(renderDelegator.second)->draw();
        }
        Vkbase::Window::delayDestroy();
    }
}

void Application::processInputs()
{
    GLFWwindow *pWindow = dynamic_cast<Vkbase::Window *>(_resourceManager.resource(Vkbase::ResourceType::Window, "mainWindow"))->window();
    if (!pWindow)
        return;
    if (glfwGetKey(pWindow, GLFW_KEY_W) == GLFW_PRESS)
        _camera.moveFront(SPEED * (_deltaTime));
    if (glfwGetKey(pWindow, GLFW_KEY_S) == GLFW_PRESS)
        _camera.moveBack(SPEED * (_deltaTime));
    if (glfwGetKey(pWindow, GLFW_KEY_A) == GLFW_PRESS)
        _camera.moveLeft(SPEED * (_deltaTime));
    if (glfwGetKey(pWindow, GLFW_KEY_D) == GLFW_PRESS)
        _camera.moveRight(SPEED * (_deltaTime));
    if (glfwGetKey(pWindow, GLFW_KEY_SPACE) == GLFW_PRESS)
        _camera.moveUp(SPEED * (_deltaTime));
    if (glfwGetKey(pWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        _camera.moveDown(SPEED * (_deltaTime));
    if (glfwGetKey(pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        dynamic_cast<Vkbase::Window *>(_resourceManager.resource(Vkbase::ResourceType::Window, "mainWindow"))->switchCursorState();
    GLFWwindow *pWindow1 = dynamic_cast<Vkbase::Window *>(_resourceManager.resource(Vkbase::ResourceType::Window, "mainWindow1"))->window();
    if (glfwGetKey(pWindow1, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        dynamic_cast<Vkbase::Window *>(_resourceManager.resource(Vkbase::ResourceType::Window, "mainWindow1"))->switchCursorState();
}

void Application::cleanup()
{
    _resourceManager.resource(Vkbase::ResourceType::Device, "0")->destroy();
}

void Application::recordCommand(const vk::CommandBuffer &commandBuffer, uint32_t imageIndex, uint32_t currentFrame)
{
    updateUniformBuffer(_resourceManager, currentFrame);

    const Vkbase::RenderPass &renderPass = *dynamic_cast<const Vkbase::RenderPass *>(_resourceManager.resource(Vkbase::ResourceType::RenderPass, "mainWindow"));
    const Vkbase::Swapchain &swapchain = *dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow"));
    const Vkbase::Pipeline &pipeline = *dynamic_cast<const Vkbase::Pipeline *>(_resourceManager.resource(Vkbase::ResourceType::Pipeline, "base"));

    std::vector<vk::ClearValue> clearValues = {vk::ClearValue().setColor({0, 0, 0, 1}), vk::ClearValue().setColor({0, 0, 0, 1}), vk::ClearValue().setDepthStencil({1.0f, 0})};

    vk::Extent2D extent = swapchain.extent();
    renderPass.begin(commandBuffer, *dynamic_cast<const Vkbase::Framebuffer *>(_resourceManager.resource(Vkbase::ResourceType::Framebuffer, "mainWindow_" + std::to_string(imageIndex))), clearValues, extent);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.pipeline());
    // commandBuffer.bindVertexBuffers(0, {dynamic_cast<const Vkbase::Buffer *>(_resourceManager.resource(Vkbase::ResourceType::Buffer, "Vertex"))->buffer()}, {0});

    // const Vkbase::DescriptorSets &descriptorSets = *dynamic_cast<const Vkbase::DescriptorSets *>(_resourceManager.resource(Vkbase::ResourceType::DescriptorSets, "UBO"));

    // commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.layout(), 0, {descriptorSets.sets("Camera")[currentFrame], descriptorSets.sets("Texture")[0]}, {});
    // commandBuffer.draw(6, 1, 0, 0);

    for (Modelbase::Model *pModel : Modelbase::Model::models())
    {
        pModel->updateUniformBuffers(0, currentFrame, _camera);
        pModel->draw(currentFrame, commandBuffer, pipeline, 0);
    }

    renderPass.end(commandBuffer);
}