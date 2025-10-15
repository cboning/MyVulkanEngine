#include "CubeOutline.h"
#include "../../Camera/Camera.h"
#include "../../Data.h"
#include "../../JsonConfigReader/JsonConfigReader.h"
#include "../../Vkbase/Vkbase.h"

CubeOutline::CubeOutline(const std::string &name, const Object &object) : Entity(name, false, object) // static false, no dynamic physics
{
    init();
}

CubeOutline::~CubeOutline()
{
}

void CubeOutline::init()
{
    json config = JsonConfigReader::load("config/cubeOutline.json");
    for (uint32_t i = 0; i < dynamic_cast<Vkbase::Swapchain *>(Vkbase::ResourceBase::resourceManager().resource(
                                                                   Vkbase::ResourceType::Swapchain, config["descriptorSets"]["sets"][0]["swapchainName"]))
                                 ->imageNames()
                                 .size();
         ++i)
    {
        _ubos.push_back(createResource<Vkbase::Buffer>(name() + "_CubeOutline_UBO_" + std::to_string(i), "Device",
                                                                                       sizeof(CubeUniformBufferData), vk::BufferUsageFlagBits::eUniformBuffer));
    }

    Vkbase::DescriptorSets &descriptorSets = *(createResource<Vkbase::DescriptorSets>(name() + "_CubeOutline", "Device"));

    config["descriptorSets"]["write"][0]["detail"]["bufferInfo"]["bufferName"] = name() + "_CubeOutline_UBO";
    descriptorSets.addDescriptorSetCreateConfigWithJson(config["descriptorSets"]["sets"]);
    descriptorSets.init();
    descriptorSets.writeSetsWithJson(config["descriptorSets"]["write"]);

    // 构建线框顶点
    GeometryVertexData cubeVertices[] = {{{-0.5f, -0.5f, 0.5f}, {}},  {{0.5f, -0.5f, 0.5f}, {}},  {{0.5f, 0.5f, 0.5f}, {}},  {{-0.5f, 0.5f, 0.5f}, {}},
                                         {{-0.5f, -0.5f, -0.5f}, {}}, {{0.5f, -0.5f, -0.5f}, {}}, {{0.5f, 0.5f, -0.5f}, {}}, {{-0.5f, 0.5f, -0.5f}, {}}};

    // 12 条边
    uint32_t cubeIndices[] = {
        0, 1, 1, 2, 2, 3, 3, 0, // front face
        4, 5, 5, 6, 6, 7, 7, 4, // back face
        0, 4, 1, 5, 2, 6, 3, 7  // sides
    };

    createResource<Vkbase::Buffer>("CubeVertex_" + name(), "Device", sizeof(GeometryVertexData) * 8,
                                                                   vk::BufferUsageFlagBits::eVertexBuffer, cubeVertices);

    createResource<Vkbase::Buffer>("CubeIndices_" + name(), "Device", sizeof(uint32_t) * 24,
                                                                   vk::BufferUsageFlagBits::eIndexBuffer, cubeIndices);
}

void CubeOutline::draw(Vkbase::CommandBuffer *pCommandBuffer, uint32_t frameIndex, const std::string &pipelineName, const std::string &uboName) const
{
    // 获取 pipeline
    auto *pPipeline = dynamic_cast<Vkbase::Pipeline *>(
        Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Pipeline, pipelineName));

    // 绑定 pipeline
    pCommandBuffer->bindPipeline(pPipeline);

    // 获取 descriptor sets
    auto *pDescriptorSets = dynamic_cast<Vkbase::DescriptorSets *>(
        Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::DescriptorSets, name() + "_CubeOutline"));

    // 绑定 descriptor sets
    pCommandBuffer->bindDescriptorSets(
        0,
        {{pDescriptorSets, {uboName, frameIndex}}}, // first = DescriptorSets*, second = {uboName, frameIndex}
        {}                                          // dynamicOffsets
    );

    // 顶点缓冲
    auto *pVertexBuffer = dynamic_cast<Vkbase::Buffer *>(
        Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Buffer, "CubeVertex_" + name()));

    // 索引缓冲
    auto *pIndexBuffer = dynamic_cast<Vkbase::Buffer *>(
        Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Buffer, "CubeIndices_" + name()));

    // 绑定顶点与索引缓冲
    pCommandBuffer->bindVertexBuffers(0, {pVertexBuffer}, {0});
    pCommandBuffer->bindIndexBuffer(pIndexBuffer, 0, vk::IndexType::eUint32);

    // 绘制线框立方体（24个索引）
    pCommandBuffer->commandBuffer().drawIndexed(24, 1, 0, 0, 0);
}


void CubeOutline::updateUBO(const Camera &camera, uint32_t index, const glm::mat4 &mat, const std::string &uboName) const
{
    CubeUniformBufferData ubo;
    ubo.model = object().matModel();
    ubo.proj = camera.perspective();
    ubo.view = camera.view();
    ubo.lightSpaceMatrix = mat;
    ubo.color = _color;

    dynamic_cast<Vkbase::Buffer *>(
        Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Buffer, name() + "_CubeOutline_" + uboName + "_" + std::to_string(index)))
        ->updateBufferData(&ubo);
}

std::vector<vk::DescriptorSetLayout> CubeOutline::descriptorSetLayouts()
{
    return {
        dynamic_cast<Vkbase::DescriptorSets *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::DescriptorSets, name() + "_CubeOutline"))
            ->layout("UBO")};
}
