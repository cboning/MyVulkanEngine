#include "Cloud.h"
#include "Vkbase/Vkbase.h"

Cloud::Cloud()
{
    init();
}

void Cloud::init()
{
    createComputePipeline();
    computeCloudData();
}

void Cloud::createComputePipeline()
{

    Vkbase::DescriptorSets &descriptorSets = *(new Vkbase::DescriptorSets("Cloud", "0"));
    descriptorSets.addDescriptorSetCreateConfig("Cloud", {{vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute}}, 1);
    descriptorSets.init();

    descriptorSets.writeSets("Cloud", 0, {}, {vk::DescriptorImageInfo().setImageLayout(vk::ImageLayout::eGeneral).setImageView((new Vkbase::Image("Cloud", "0", 32, 32, 32, vk::Format::eR32Sfloat, vk::ImageType::e3D, vk::ImageViewType::e3D, vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled))->view())}, 1);

    std::vector<Vkbase::ShaderInfo> shaderInfo = {{"./shader/bin/cloudComp.spv", "main", vk::ShaderStageFlagBits::eCompute}};
    new Vkbase::Pipeline("Cloud", "0", "", {shaderInfo, {}, {descriptorSets.layout("Cloud")}}, true);
}

void Cloud::computeCloudData()
{
    uint32_t width, height, depth;
    width = 32;
    height = 32;
    depth = 32;
    Vkbase::DescriptorSets &descriptorSets = *dynamic_cast<Vkbase::DescriptorSets *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::DescriptorSets, "Cloud"));
    Vkbase::Pipeline &pipeline = *dynamic_cast<Vkbase::Pipeline *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Pipeline, "Cloud"));
    Vkbase::Device &device = *dynamic_cast<Vkbase::Device *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Device, "0"));
    const Vkbase::CommandPool &commandPool = Vkbase::CommandPool::getCommandPool("0", Vkbase::CommandPoolQueueType::Compute);
    vk::CommandBuffer commandBuffer = commandPool.allocateOnceCommandBuffer();
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline.pipeline());
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipeline.layout(), 0, descriptorSets.sets("Cloud")[0], nullptr);
    uint32_t groupCountX = (width + 7) / 8;
    uint32_t groupCountY = (height + 7) / 8;
    uint32_t groupCountZ = (depth + 7) / 8;
    vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
    commandPool.endOnceCommandBuffer(commandBuffer);
    pipeline.destroy();

    dynamic_cast<Vkbase::Image *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Image, "Cloud"))->transitionImageLayout(vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal);
}
