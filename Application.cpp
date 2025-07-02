#include "Application.h"
#include "Vkbase/ResourceBase.h"
#include "Vkbase/Window.h"
#include "Vkbase/RenderPass.h"
#include "Vkbase/Swapchain.h"
#include "Vkbase/Pipeline.h"

Application::Application()
    : _resourceManager(Vkbase::ResourceBase::resourceManager())
{
    init();
}

void Application::init()
{
    new Vkbase::Window("mainWindow", "Vulkan Window", 800, 600);
    new Vkbase::Window("mainWindow1", "Vulkan Window", 800, 600);
    createRenderPass();
}

void Application::createRenderPass()
{
    const Vkbase::Swapchain &swapchain = *dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow"));

    vk::AttachmentDescription colorAttachment;
    colorAttachment.setFormat(swapchain.format())
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference colorAttachmentRef;
    colorAttachmentRef.setAttachment(0)
        .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription subpass;
    subpass.setColorAttachments(colorAttachmentRef);

    vk::RenderPassCreateInfo createInfo;
    createInfo.setAttachments(colorAttachment)
        .setSubpasses(subpass);

    const Vkbase::RenderPass &renderPass = *(new Vkbase::RenderPass("mainWindow", "0", createInfo));

    vk::Extent2D extent = swapchain.extent();
    for (uint32_t i = 0; i < swapchain.imageNames().size(); ++i)
        renderPass.createFramebuffer("mainWindow_" + std::to_string(i), {swapchain.imageNames()[i]}, extent.width, extent.height);

    std::vector<Vkbase::ShaderInfo> shaderInfo = {Vkbase::ShaderInfo("shader/baseShaderVert.spv", "main", vk::ShaderStageFlagBits::eVertex),
                                                  Vkbase::ShaderInfo("shader/baseShaderFrag.spv", "main", vk::ShaderStageFlagBits::eFragment)};

    std::vector<vk::VertexInputAttributeDescription> inputAttributes;
    std::vector<vk::VertexInputBindingDescription> inputBindings;
    Vkbase::VertexInfo vertexInfo(inputAttributes, inputBindings);

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;

    Vkbase::PipelineRenderInfo renderInfo = Vkbase::Pipeline::getDefaultRenderInfo();
    {
        vk::PipelineColorBlendAttachmentState colorBlendAttachment;
        colorBlendAttachment.setBlendEnable(vk::False)
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
        
        renderInfo.blendAttachments.push_back(colorBlendAttachment);
    }

    Vkbase::PipelineCreateInfo pipelineCreateInfo(shaderInfo, vertexInfo, descriptorSetLayouts, renderInfo);
    renderPass.createPipeline("base", pipelineCreateInfo);
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
    }
}
void Application::cleanup()
{
    _resourceManager.remove(Vkbase::ResourceType::Pipeline, "base");
}