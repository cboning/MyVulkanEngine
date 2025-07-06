#include "Application.h"
#include "Vkbase/ResourceBase.h"
#include "Vkbase/Window.h"
#include "Vkbase/RenderPass.h"
#include "Vkbase/Swapchain.h"
#include "Vkbase/Pipeline.h"
#include "Vkbase/RenderDelegator.h"
#include "Vkbase/Framebuffer.h"

Application::Application()
    : _resourceManager(Vkbase::ResourceBase::resourceManager())
{
    init();
}

void Application::init()
{
    Vkbase::Window *pWindow = new Vkbase::Window("mainWindow", "Vulkan Window", 80, 60);
    new Vkbase::Window("mainWindow1", "Vulkan Window", 80, 60);
    createRenderPass();
}

void Application::createRenderPass()
{
    const Vkbase::Swapchain &swapchain = *dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow"));
    const Vkbase::Swapchain &swapchain1 = *dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow1"));

    std::vector<vk::AttachmentDescription> colorAttachments(2);
    colorAttachments[0].setFormat(swapchain.format())
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
    

    colorAttachments[1].setFormat(swapchain1.format())
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    std::vector<vk::AttachmentReference> colorAttachmentRefs(2);
    colorAttachmentRefs[0].setAttachment(0)
        .setLayout(vk::ImageLayout::eColorAttachmentOptimal);
    colorAttachmentRefs[1].setAttachment(1)
        .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription subpass;
    subpass.setColorAttachments(colorAttachmentRefs);

    vk::RenderPassCreateInfo createInfo;
    createInfo.setAttachments(colorAttachments)
        .setSubpasses(subpass);

    const Vkbase::RenderPass &renderPass = *(new Vkbase::RenderPass("mainWindow", "0", createInfo));

    // --------------------------------
    vk::Extent2D extent = swapchain.extent();
    for (uint32_t i = 0; i < swapchain.imageNames().size(); ++i)
        renderPass.createFramebuffer("mainWindow_" + std::to_string(i), {swapchain.imageNames()[i], swapchain1.imageNames()[i]}, extent.width, extent.height);

    // --------------------------------
    std::vector<Vkbase::ShaderInfo> shaderInfo = {Vkbase::ShaderInfo("shader/baseShaderVert.spv", "main", vk::ShaderStageFlagBits::eVertex),
                                                  Vkbase::ShaderInfo("shader/baseShaderFrag.spv", "main", vk::ShaderStageFlagBits::eFragment)};

    std::vector<vk::VertexInputAttributeDescription> inputAttributes;
    std::vector<vk::VertexInputBindingDescription> inputBindings;
    Vkbase::VertexInfo vertexInfo(inputAttributes, inputBindings);

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;

    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.setBlendEnable(vk::False)
        .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
    
    Vkbase::PipelineRenderInfo renderInfo = Vkbase::Pipeline::getDefaultRenderInfo();
    renderInfo.blendAttachments.push_back(colorBlendAttachment);
    renderInfo.blendAttachments.push_back(colorBlendAttachment);

    Vkbase::PipelineCreateInfo pipelineCreateInfo(shaderInfo, vertexInfo, descriptorSetLayouts, renderInfo);
    renderPass.createPipeline("base", pipelineCreateInfo);
    createRenderDelegator();
}

void Application::createRenderDelegator()
{
    const Vkbase::Swapchain &swapchain = *dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow"));
    const Vkbase::Swapchain &swapchain1 = *dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow1"));

    _pRenderDelegator = new Vkbase::RenderDelegator("mainRender", "0", {swapchain.name(), swapchain1.name()}, "Graphics0");
    _pRenderDelegator->setCommandRecordFunc(recordCommand);
    _pRenderDelegator->setRenderPassCreateFunc([this](){
        this->createRenderPass();
    });
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
        if (_resourceManager.resources().count(Vkbase::ResourceType::RenderDelegator))
        {
            const std::unordered_map<std::string, Vkbase::ResourceBase *> resources = _resourceManager.resources().at(Vkbase::ResourceType::RenderDelegator);
            for (auto renderDelegator : resources)
                dynamic_cast<Vkbase::RenderDelegator *>(renderDelegator.second)->draw();
        }
        Vkbase::Window::delayDestroy();
    }
}
void Application::cleanup()
{
    _resourceManager.resource(Vkbase::ResourceType::Pipeline, "base")->destroy();
}

void Application::recordCommand(Vkbase::ResourceManager &resourceManager, const vk::CommandBuffer &commandBuffer, uint32_t imageIndex)
{
    const Vkbase::RenderPass &renderPass = *dynamic_cast<const Vkbase::RenderPass *>(resourceManager.resource(Vkbase::ResourceType::RenderPass, "mainWindow"));
    const Vkbase::Swapchain &swapchain = *dynamic_cast<const Vkbase::Swapchain *>(resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow"));
    const Vkbase::Pipeline &pipeline = *dynamic_cast<const Vkbase::Pipeline *>(resourceManager.resource(Vkbase::ResourceType::Pipeline, "base"));

    std::vector<vk::ClearValue> clearValues = {vk::ClearValue().setColor({0, 0, 0, 1}), vk::ClearValue().setColor({0, 0, 0, 1})};

    vk::Extent2D extent = swapchain.extent();
    renderPass.begin(commandBuffer, *dynamic_cast<const Vkbase::Framebuffer *>(resourceManager.resource(Vkbase::ResourceType::Framebuffer, "mainWindow_" + std::to_string(imageIndex))), clearValues, extent);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.pipeline());
    commandBuffer.draw(3, 1, 0, 0);
    renderPass.end(commandBuffer);
}