#include "Cloud.h"
#include "Engine/Vkbase/Vkbase.h"

Cloud::Cloud() { init(); }

void Cloud::init()
{
    createComputePipeline();
    computeCloudData();
}

void Cloud::createComputePipeline()
{

    // Vkbase::DescriptorSets &descriptorSets = *(createResource<Vkbase::DescriptorSets>("Cloud", "Device"));
    // descriptorSets.addDescriptorSetCreateConfig("Cloud", {{vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute}}, 1);
    // descriptorSets.init();

    // descriptorSets.writeSets("Cloud", 0, {},
    //                          {{vk::DescriptorImageInfo().setImageLayout(vk::ImageLayout::eGeneral),
    //                            createResource<Vkbase::Image>("Cloud", "Device", 32, 32, 32, vk::Format::eR32Sfloat, vk::ImageType::e3D, vk::ImageViewType::e3D,
    //                                                          vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled)}},
    //                          1);

    // std::vector<Vkbase::ShaderInfo> shaderInfo = {{"./shader/bin/cloudComp.spv", "main", vk::ShaderStageFlagBits::eCompute}};
    // Vkbase::PipelineRenderInfo renderInfo;
    // createResource<Vkbase::Pipeline>("Cloud", "Device", "", Vkbase::PipelineCreateInfo{shaderInfo, {}, {descriptorSets.layout("Cloud")}, renderInfo}, true);
}

void Cloud::computeCloudData()
{
    // uint32_t width, height, depth;
    // width = 32;
    // height = 32;
    // depth = 32;
    // Vkbase::DescriptorSets &descriptorSets =
    //     *dynamic_cast<Vkbase::DescriptorSets *>(Vkbase::VkResourceBase::resourceManager().resource(Vkbase::VkResourceType::DescriptorSets, "Cloud"));
    // Vkbase::Pipeline &pipeline =
    //     *dynamic_cast<Vkbase::Pipeline *>(Vkbase::VkResourceBase::resourceManager().resource(Vkbase::VkResourceType::Pipeline, "Cloud"));
    // Vkbase::CommandPool &commandPool = Vkbase::CommandPool::getCommandPool("Device", Vkbase::CommandPoolQueueType::Compute);
    // Vkbase::CommandBuffer *pCommandBuffer = commandPool.allocateOnceCommandBuffer();
    // pCommandBuffer->bindPipeline(&pipeline);
    // pCommandBuffer->bindDescriptorSets(0, {{&descriptorSets, {"Cloud", 0}}}, nullptr);
    // uint32_t groupCountX = (width + 7) / 8;
    // uint32_t groupCountY = (height + 7) / 8;
    // uint32_t groupCountZ = (depth + 7) / 8;
    // pCommandBuffer->commandBuffer().dispatch(groupCountX, groupCountY, groupCountZ);
    // commandPool.endOnceCommandBuffer(pCommandBuffer);
    // pipeline.destroy();

    // dynamic_cast<Vkbase::Image *>(Vkbase::VkResourceBase::resourceManager().resource(Vkbase::VkResourceType::Image, "Cloud"))
    //     ->transitionImageLayout(vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal);
}
