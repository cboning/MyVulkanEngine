#include "Render.h"
#include "Modelbase/Modelbase.h"
#include "Data.h"
#include "Cloud.h"
#define SPEED 10.0f

void Render::init()
{
    resourceInit();
}

void Render::resourceInit()
{
    new Vkbase::Window("mainWindow1", "Vulkan Window", 800, 600);
    Vkbase::Window *pWindow = new Vkbase::Window("mainWindow", "Vulkan Window", 800, 600);
    pWindow->setMouseMoveCallback([this](double x, double y)
                                  { Render::camera().addViewBy(x, -y); });

    VertexData vertices[] = {{glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
                             {glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
                             {glm::vec3(1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
                             {glm::vec3(1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
                             {glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)},
                             {glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)}};

    new Vkbase::Buffer("Vertex", "0", sizeof(VertexData) * 6, vk::BufferUsageFlagBits::eVertexBuffer, vertices);
    for (uint32_t i = 0; i < MAX_FLIGHT_COUNT; ++i)
        new Vkbase::Buffer("UBO" + std::to_string(i), "0", sizeof(UniformBufferData), vk::BufferUsageFlagBits::eUniformBuffer);
    new Vkbase::Sampler("Sampler", "0");

    delete new Cloud();

    Modelbase::Model *pModel = new Modelbase::Model("0", "Graphics0", "src/model/zhongli-from-genshin-impact/ze+hongli.fbx", {aiTextureType_DIFFUSE}, dynamic_cast<const Vkbase::Sampler *>(_resourceManager.resource(Vkbase::ResourceType::Sampler, "Sampler"))->sampler());
    pModel->createNewInstance("1", {0, 0.0f});

    createDescriptorSets();
    createRenderPass();
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
    const Vkbase::Device &device = *(dynamic_cast<const Vkbase::Device *>(_resourceManager.resource(Vkbase::ResourceType::Device, "0")));
    vk::Format depthFormat = device.findSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint}, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);

    const Vkbase::Swapchain &swapchain = *dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow"));
    const Vkbase::Swapchain &swapchain1 = *dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow1"));
    const Vkbase::DescriptorSets &descriptorSets = *dynamic_cast<const Vkbase::DescriptorSets *>(_resourceManager.resource(Vkbase::ResourceType::DescriptorSets, "MainDescriptorSets"));
    Vkbase::Sampler &sampler = *dynamic_cast<Vkbase::Sampler *>(_resourceManager.resource(Vkbase::ResourceType::Sampler, "Sampler"));

    std::vector<vk::AttachmentDescription> attachments(8);
    attachments[0].setFormat(vk::Format::eR16G16B16A16Sfloat).setSamples(vk::SampleCountFlagBits::e1).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore).setStencilLoadOp(vk::AttachmentLoadOp::eDontCare).setStencilStoreOp(vk::AttachmentStoreOp::eDontCare).setInitialLayout(vk::ImageLayout::eUndefined).setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    attachments[1].setFormat(vk::Format::eR16G16B16A16Sfloat).setSamples(vk::SampleCountFlagBits::e1).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore).setStencilLoadOp(vk::AttachmentLoadOp::eDontCare).setStencilStoreOp(vk::AttachmentStoreOp::eDontCare).setInitialLayout(vk::ImageLayout::eUndefined).setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    attachments[2].setFormat(vk::Format::eR16G16B16A16Sfloat).setSamples(vk::SampleCountFlagBits::e1).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore).setStencilLoadOp(vk::AttachmentLoadOp::eDontCare).setStencilStoreOp(vk::AttachmentStoreOp::eDontCare).setInitialLayout(vk::ImageLayout::eUndefined).setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    attachments[3].setFormat(vk::Format::eR16G16B16A16Sfloat).setSamples(vk::SampleCountFlagBits::e1).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore).setStencilLoadOp(vk::AttachmentLoadOp::eDontCare).setStencilStoreOp(vk::AttachmentStoreOp::eDontCare).setInitialLayout(vk::ImageLayout::eUndefined).setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    attachments[4].setFormat(vk::Format::eR16G16B16A16Sfloat).setSamples(vk::SampleCountFlagBits::e1).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore).setStencilLoadOp(vk::AttachmentLoadOp::eDontCare).setStencilStoreOp(vk::AttachmentStoreOp::eDontCare).setInitialLayout(vk::ImageLayout::eUndefined).setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    attachments[5].setFormat(vk::Format::eR16G16B16A16Sfloat).setSamples(vk::SampleCountFlagBits::e1).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore).setStencilLoadOp(vk::AttachmentLoadOp::eDontCare).setStencilStoreOp(vk::AttachmentStoreOp::eDontCare).setInitialLayout(vk::ImageLayout::eUndefined).setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    attachments[6].setFormat(swapchain.format()).setSamples(vk::SampleCountFlagBits::e1).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore).setStencilLoadOp(vk::AttachmentLoadOp::eDontCare).setStencilStoreOp(vk::AttachmentStoreOp::eDontCare).setInitialLayout(vk::ImageLayout::eUndefined).setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    attachments[7].setFormat(depthFormat).setSamples(vk::SampleCountFlagBits::e1).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eDontCare).setStencilLoadOp(vk::AttachmentLoadOp::eDontCare).setStencilStoreOp(vk::AttachmentStoreOp::eDontCare).setInitialLayout(vk::ImageLayout::eUndefined).setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    std::vector<vk::AttachmentReference> colorAttachmentRefs[9];
    colorAttachmentRefs[0].resize(3);
    colorAttachmentRefs[0][0].setAttachment(0).setLayout(vk::ImageLayout::eColorAttachmentOptimal);
    colorAttachmentRefs[0][1].setAttachment(1).setLayout(vk::ImageLayout::eColorAttachmentOptimal);
    colorAttachmentRefs[0][2].setAttachment(2).setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    colorAttachmentRefs[1].resize(2);
    colorAttachmentRefs[1][0].setAttachment(3).setLayout(vk::ImageLayout::eColorAttachmentOptimal);
    colorAttachmentRefs[1][1].setAttachment(4).setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    colorAttachmentRefs[2].resize(1);
    colorAttachmentRefs[2][0].setAttachment(5).setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    colorAttachmentRefs[3].resize(1);
    colorAttachmentRefs[3][0].setAttachment(4).setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    colorAttachmentRefs[4].resize(1);
    colorAttachmentRefs[4][0].setAttachment(6).setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    colorAttachmentRefs[5].resize(3);
    colorAttachmentRefs[5][0].setAttachment(0).setLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    colorAttachmentRefs[5][1].setAttachment(1).setLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    colorAttachmentRefs[5][2].setAttachment(2).setLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    colorAttachmentRefs[6].resize(1);
    colorAttachmentRefs[6][0].setAttachment(4).setLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    colorAttachmentRefs[7].resize(1);
    colorAttachmentRefs[7][0].setAttachment(5).setLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    colorAttachmentRefs[8].resize(2);
    colorAttachmentRefs[8][0].setAttachment(3).setLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    colorAttachmentRefs[8][1].setAttachment(4).setLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    vk::AttachmentReference depthStencilRef;
    depthStencilRef.setAttachment(7)
        .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    std::vector<vk::SubpassDescription> subpasses(5);
    subpasses[0].setColorAttachments(colorAttachmentRefs[0]).setPDepthStencilAttachment(&depthStencilRef);
    subpasses[1].setColorAttachments(colorAttachmentRefs[1]).setInputAttachments(colorAttachmentRefs[5]);
    subpasses[2].setColorAttachments(colorAttachmentRefs[2]).setInputAttachments(colorAttachmentRefs[6]);
    subpasses[3].setColorAttachments(colorAttachmentRefs[3]).setInputAttachments(colorAttachmentRefs[7]);
    subpasses[4].setColorAttachments(colorAttachmentRefs[4]).setInputAttachments(colorAttachmentRefs[8]);

    std::vector<vk::SubpassDependency> dependencies;
    {
        vk::SubpassDependency &info = dependencies.emplace_back();
        info.setSrcSubpass(vk::SubpassExternal).setDstSubpass(0).setSrcStageMask(vk::PipelineStageFlagBits::eFragmentShader).setSrcAccessMask(vk::AccessFlagBits::eNone).setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput).setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);
    }
    {
        vk::SubpassDependency &info = dependencies.emplace_back();
        info.setSrcSubpass(0).setDstSubpass(1).setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput).setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite).setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader).setDstAccessMask(vk::AccessFlagBits::eInputAttachmentRead);
    }
    {
        vk::SubpassDependency &info = dependencies.emplace_back();
        info.setSrcSubpass(1).setDstSubpass(2).setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput).setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite).setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader).setDstAccessMask(vk::AccessFlagBits::eShaderRead);
    }
    {
        vk::SubpassDependency &info = dependencies.emplace_back();
        info.setSrcSubpass(2).setDstSubpass(3).setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput).setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite).setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader).setDstAccessMask(vk::AccessFlagBits::eShaderRead);
    }
    {
        vk::SubpassDependency &info = dependencies.emplace_back();
        info.setSrcSubpass(3).setDstSubpass(4).setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput).setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite).setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader).setDstAccessMask(vk::AccessFlagBits::eInputAttachmentRead);
    }
    {
        vk::SubpassDependency &info = dependencies.emplace_back();
        info.setSrcSubpass(1).setDstSubpass(4).setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput).setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite).setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader).setDstAccessMask(vk::AccessFlagBits::eInputAttachmentRead);
    }

    vk::RenderPassCreateInfo createInfo;
    createInfo.setAttachments(attachments)
        .setDependencies(dependencies)
        .setSubpasses(subpasses);

    const Vkbase::RenderPass &renderPass = *(new Vkbase::RenderPass("mainWindow", "0", createInfo));

    // --------------------------------
    vk::Extent2D extent = swapchain.extent();
    for (uint32_t i = 0; i < swapchain.imageNames().size(); ++i)
    {
        Vkbase::Image &positionAttachment = *(new Vkbase::Image("Position_" + std::to_string(i), "0", extent.width, extent.height, 1, vk::Format::eR16G16B16A16Sfloat, vk::ImageType::e2D, vk::ImageViewType::e2D, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment));
        Vkbase::Image &normalAttachment = *(new Vkbase::Image("Normal_" + std::to_string(i), "0", extent.width, extent.height, 1, vk::Format::eR16G16B16A16Sfloat, vk::ImageType::e2D, vk::ImageViewType::e2D, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment));
        Vkbase::Image &albedoSpecAttachment = *(new Vkbase::Image("AlbedoSpec_" + std::to_string(i), "0", extent.width, extent.height, 1, vk::Format::eR16G16B16A16Sfloat, vk::ImageType::e2D, vk::ImageViewType::e2D, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment));
        Vkbase::Image &originalColorAttachment = *(new Vkbase::Image("OriginalColor_" + std::to_string(i), "0", extent.width, extent.height, 1, vk::Format::eR16G16B16A16Sfloat, vk::ImageType::e2D, vk::ImageViewType::e2D, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment));
        Vkbase::Image &blurColorAttachment1 = *(new Vkbase::Image("BlurColor1_" + std::to_string(i), "0", extent.width, extent.height, 1, vk::Format::eR16G16B16A16Sfloat, vk::ImageType::e2D, vk::ImageViewType::e2D, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eInputAttachment));
        Vkbase::Image &blurColorAttachment2 = *(new Vkbase::Image("BlurColor2_" + std::to_string(i), "0", extent.width, extent.height, 1, vk::Format::eR16G16B16A16Sfloat, vk::ImageType::e2D, vk::ImageViewType::e2D, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eInputAttachment));
        Vkbase::Image &depthImage = *(new Vkbase::Image("Depth_" + std::to_string(i), "0", extent.width, extent.height, 1, depthFormat, vk::ImageType::e2D, vk::ImageViewType::e2D, vk::ImageUsageFlagBits::eDepthStencilAttachment));
        renderPass.createFramebuffer("mainWindow_" + std::to_string(i), {positionAttachment.name(), normalAttachment.name(), albedoSpecAttachment.name(), originalColorAttachment.name(), blurColorAttachment1.name(), blurColorAttachment2.name(), swapchain.imageNames()[i], depthImage.name()}, extent.width, extent.height);
    }

    vk::DescriptorImageInfo imageInfo;
    imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
        .setSampler(sampler.sampler());
    // {
    //     std::vector<vk::DescriptorImageInfo> imageInfos(swapchain.imageNames().size(), imageInfo);
    //     for (uint32_t i = 0; i < swapchain.imageNames().size(); ++i)
    //         imageInfos[i].setImageView(dynamic_cast<const Vkbase::Image *>(_resourceManager.resource(Vkbase::ResourceType::Image, "BlurColor1_" + std::to_string(i)))->view());
    //     descriptorSets.writeSets("BlurSampler1", 0, {}, imageInfos, swapchain.imageNames().size());
    // }
    {
        std::vector<vk::DescriptorImageInfo> imageInfos(swapchain.imageNames().size(), imageInfo);
        for (uint32_t i = 0; i < swapchain.imageNames().size(); ++i)
            imageInfos[i].setImageView(dynamic_cast<const Vkbase::Image *>(_resourceManager.resource(Vkbase::ResourceType::Image, "Cloud"))->view());
        descriptorSets.writeSets("BlurSampler1", 0, {}, imageInfos, swapchain.imageNames().size());
    }
    {
        std::vector<vk::DescriptorImageInfo> imageInfos(swapchain.imageNames().size(), imageInfo);
        for (uint32_t i = 0; i < swapchain.imageNames().size(); ++i)
            imageInfos[i].setImageView(dynamic_cast<const Vkbase::Image *>(_resourceManager.resource(Vkbase::ResourceType::Image, "BlurColor2_" + std::to_string(i)))->view());
        descriptorSets.writeSets("BlurSampler2", 0, {}, imageInfos, swapchain.imageNames().size());
    }
    imageInfo.setSampler(nullptr);
    {
        std::vector<vk::DescriptorImageInfo> imageInfos(swapchain.imageNames().size(), imageInfo);
        for (uint32_t i = 0; i < swapchain.imageNames().size(); ++i)
            imageInfos[i].setImageView(dynamic_cast<const Vkbase::Image *>(_resourceManager.resource(Vkbase::ResourceType::Image, "Position_" + std::to_string(i)))->view());
        descriptorSets.writeSets("G_BufferInputAttachments", 0, {}, imageInfos, swapchain.imageNames().size());
    }
    {
        std::vector<vk::DescriptorImageInfo> imageInfos(swapchain.imageNames().size(), imageInfo);
        for (uint32_t i = 0; i < swapchain.imageNames().size(); ++i)
            imageInfos[i].setImageView(dynamic_cast<const Vkbase::Image *>(_resourceManager.resource(Vkbase::ResourceType::Image, "Normal_" + std::to_string(i)))->view());
        descriptorSets.writeSets("G_BufferInputAttachments", 1, {}, imageInfos, swapchain.imageNames().size());
    }
    {
        std::vector<vk::DescriptorImageInfo> imageInfos(swapchain.imageNames().size(), imageInfo);
        for (uint32_t i = 0; i < swapchain.imageNames().size(); ++i)
            imageInfos[i].setImageView(dynamic_cast<const Vkbase::Image *>(_resourceManager.resource(Vkbase::ResourceType::Image, "AlbedoSpec_" + std::to_string(i)))->view());
        descriptorSets.writeSets("G_BufferInputAttachments", 2, {}, imageInfos, swapchain.imageNames().size());
    }

    {
        std::vector<vk::DescriptorImageInfo> imageInfos(swapchain.imageNames().size(), imageInfo);
        for (uint32_t i = 0; i < swapchain.imageNames().size(); ++i)
            imageInfos[i].setImageView(dynamic_cast<const Vkbase::Image *>(_resourceManager.resource(Vkbase::ResourceType::Image, "OriginalColor_" + std::to_string(i)))->view());
        descriptorSets.writeSets("BlendInputAttachments", 0, {}, imageInfos, swapchain.imageNames().size());
    }
    {
        std::vector<vk::DescriptorImageInfo> imageInfos(swapchain.imageNames().size(), imageInfo);
        for (uint32_t i = 0; i < swapchain.imageNames().size(); ++i)
            imageInfos[i].setImageView(dynamic_cast<const Vkbase::Image *>(_resourceManager.resource(Vkbase::ResourceType::Image, "BlurColor1_" + std::to_string(i)))->view());
        descriptorSets.writeSets("BlendInputAttachments", 1, {}, imageInfos, swapchain.imageNames().size());
    }

    // --------------------------------
    std::vector<Vkbase::ShaderInfo> shaderInfos = {Vkbase::ShaderInfo("", "main", vk::ShaderStageFlagBits::eVertex),
                                                   Vkbase::ShaderInfo("", "main", vk::ShaderStageFlagBits::eFragment)};

    Vkbase::VertexInfo modelVertexInfo(ModelData::Vertex::attributeDescriptions(), {ModelData::Vertex::bindingDescription()});
    Vkbase::VertexInfo screenVertexInfo(VertexData::attributeDescriptions(), {VertexData::bindingDescription()});

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;

    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.setBlendEnable(vk::False)
        .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

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

    renderInfo.blendAttachments.push_back(colorBlendAttachment);
    descriptorSetLayouts = {descriptorSets.layout("G_BufferInputAttachments")};
    shaderInfos[0].filename = "shader/bin/baseShaderVert.spv";
    shaderInfos[1].filename = "shader/bin/baseShaderFrag.spv";
    renderInfo.rasterizationStateInfo.setCullMode(vk::CullModeFlagBits::eNone);
    renderInfo.subpass = 1;
    renderPass.createPipeline("light", Vkbase::PipelineCreateInfo(shaderInfos, screenVertexInfo, descriptorSetLayouts, &renderInfo));

    std::vector<vk::DescriptorSetLayout> layout = (*Modelbase::Model::models().begin())->descriptorSetLayout(0);
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
    Vkbase::DescriptorSets *pDescriptorSets = new Vkbase::DescriptorSets("MainDescriptorSets", "0");
    const Vkbase::Swapchain &swapchain = *dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow"));
    pDescriptorSets->addDescriptorSetCreateConfig("Camera", {{vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex}}, Vkbase::RenderDelegator::maxFlightCount());
    pDescriptorSets->addDescriptorSetCreateConfig("G_BufferInputAttachments", {{vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment}, {vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment}, {vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment}}, swapchain.images().size());
    pDescriptorSets->addDescriptorSetCreateConfig("BlurSampler1", {{vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment}}, swapchain.images().size());
    pDescriptorSets->addDescriptorSetCreateConfig("BlurSampler2", {{vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment}}, swapchain.images().size());
    pDescriptorSets->addDescriptorSetCreateConfig("BlendInputAttachments", {{vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment}, {vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment}}, swapchain.images().size());
    pDescriptorSets->init();

    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.setOffset(0)
        .setRange(sizeof(UniformBufferData));

    {
        std::vector<vk::DescriptorBufferInfo> bufferInfos(Vkbase::RenderDelegator::maxFlightCount(), bufferInfo);
        for (int i = 0; i < Vkbase::RenderDelegator::maxFlightCount(); ++i)
            bufferInfos[i].setBuffer(dynamic_cast<Vkbase::Buffer *>(_resourceManager.resource(Vkbase::ResourceType::Buffer, "UBO" + std::to_string(i)))->buffer());
        pDescriptorSets->writeSets("Camera", 0, bufferInfos, {}, Vkbase::RenderDelegator::maxFlightCount());
    }
}

void Render::createRenderDelegator()
{
    const Vkbase::Swapchain &swapchain = *dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow"));
    const Vkbase::Swapchain &swapchain1 = *dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow1"));

    _pRenderDelegator = new Vkbase::RenderDelegator("mainRender", "0", {swapchain.name()}, "Graphics0");
    _pRenderDelegator->setCommandRecordFunc([this](const vk::CommandBuffer &commandBuffer, uint32_t imageIndex, uint32_t currentFrame)
                                            { this->recordCommand(commandBuffer, imageIndex, currentFrame); });
    _pRenderDelegator->setRenderPassCreateFunc([this]()
                                               { this->createRenderPass(); });
}

void Render::draw()
{
    glfwPollEvents();
    processInputs();
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
    _resourceManager.resource(Vkbase::ResourceType::Device, "0")->destroy();
}

void Render::recordCommand(const vk::CommandBuffer &commandBuffer, uint32_t imageIndex, uint32_t currentFrame)
{
    updateUniformBuffer(_resourceManager, currentFrame);

    const Vkbase::RenderPass &renderPass = *dynamic_cast<const Vkbase::RenderPass *>(_resourceManager.resource(Vkbase::ResourceType::RenderPass, "mainWindow"));
    const Vkbase::Swapchain &swapchain = *dynamic_cast<const Vkbase::Swapchain *>(_resourceManager.resource(Vkbase::ResourceType::Swapchain, "mainWindow"));
    const Vkbase::Pipeline &g_bufferPipeline = *dynamic_cast<const Vkbase::Pipeline *>(_resourceManager.resource(Vkbase::ResourceType::Pipeline, "g_buffer"));
    const Vkbase::Pipeline &lightPipeline = *dynamic_cast<const Vkbase::Pipeline *>(_resourceManager.resource(Vkbase::ResourceType::Pipeline, "light"));
    const Vkbase::Pipeline &pipeline1 = *dynamic_cast<const Vkbase::Pipeline *>(_resourceManager.resource(Vkbase::ResourceType::Pipeline, "Inversted_Hull"));
    const Vkbase::Pipeline &blendPipeline = *dynamic_cast<const Vkbase::Pipeline *>(_resourceManager.resource(Vkbase::ResourceType::Pipeline, "blend"));
    const Vkbase::Pipeline &blur_hPipeline = *dynamic_cast<const Vkbase::Pipeline *>(_resourceManager.resource(Vkbase::ResourceType::Pipeline, "blur_h"));
    const Vkbase::Pipeline &blur_vPipeline = *dynamic_cast<const Vkbase::Pipeline *>(_resourceManager.resource(Vkbase::ResourceType::Pipeline, "blur_v"));
    const Vkbase::Buffer &screenVertexBuffer = *dynamic_cast<const Vkbase::Buffer *>(_resourceManager.resource(Vkbase::ResourceType::Buffer, "Vertex"));
    const Vkbase::DescriptorSets &descriptorSets = *dynamic_cast<const Vkbase::DescriptorSets *>(_resourceManager.resource(Vkbase::ResourceType::DescriptorSets, "MainDescriptorSets"));

    std::vector<vk::ClearValue> clearValues = {vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}), vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}), vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}), vk::ClearValue().setColor({1.0f, 1.0f, 1.0f, 1.0f}), vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}), vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}), vk::ClearValue().setColor({0.0f, 0.0f, 0.0f, 1.0f}), vk::ClearValue().setDepthStencil({1.0f, 0})};

    vk::Extent2D extent = swapchain.extent();
    renderPass.begin(commandBuffer, *dynamic_cast<const Vkbase::Framebuffer *>(_resourceManager.resource(Vkbase::ResourceType::Framebuffer, "mainWindow_" + std::to_string(imageIndex))), clearValues, extent);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, g_bufferPipeline.pipeline());
    for (Modelbase::Model *pModel : Modelbase::Model::models())
    {
        pModel->updateUniformBuffers(0, currentFrame, _camera);
        pModel->draw(currentFrame, commandBuffer, g_bufferPipeline, 0);
    }

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, lightPipeline.pipeline());
    commandBuffer.bindVertexBuffers(0, {screenVertexBuffer.buffer()}, {0});
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, lightPipeline.layout(), 0, descriptorSets.sets("G_BufferInputAttachments")[imageIndex], {});
    commandBuffer.draw(6, 1, 0, 0);

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, blur_hPipeline.pipeline());
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, blur_hPipeline.layout(), 0, descriptorSets.sets("BlurSampler1")[imageIndex], {});
    commandBuffer.bindVertexBuffers(0, {screenVertexBuffer.buffer()}, {0});
    commandBuffer.draw(6, 1, 0, 0);

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, blur_vPipeline.pipeline());
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, blur_vPipeline.layout(), 0, descriptorSets.sets("BlurSampler2")[imageIndex], {});
    commandBuffer.bindVertexBuffers(0, {screenVertexBuffer.buffer()}, {0});
    commandBuffer.draw(6, 1, 0, 0);

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, blendPipeline.pipeline());
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, blendPipeline.layout(), 0, descriptorSets.sets("BlendInputAttachments")[imageIndex], {});
    commandBuffer.bindVertexBuffers(0, {screenVertexBuffer.buffer()}, {0});
    commandBuffer.draw(6, 1, 0, 0);

    renderPass.end(commandBuffer);
}

Camera &Render::camera()
{
    return _camera;
}

void Render::processInputs()
{
    _lastTime = _currentTime;
    _currentTime = glfwGetTime();
    _deltaTime = _currentTime - _lastTime;
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
}

bool Render::shouldEndApplication()
{
    return _resourceManager.resources().count(Vkbase::ResourceType::Window);
}