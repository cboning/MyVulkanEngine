#pragma once
#include <any>
#include <json.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.hpp>

using json = nlohmann::json;
namespace Vkbase
{
class CommandBuffer;
}

class RenderObjectManager;

class RenderPassManager
{
private:
    struct RenderPassInfo
    {
        std::string name;
        std::vector<std::vector<std::string>> pipelineNames;
        std::vector<vk::ClearValue> clearValues;

    };
    std::vector<RenderPassInfo> _renderPasses;

    static std::vector<RenderPassInfo> processConfig(const json &config);

public:
    RenderPassManager(const json &config);
    void draw(Vkbase::CommandBuffer *pCommandBuffer, RenderObjectManager *pObjects, uint32_t imageIndex, uint32_t frameIndex) const;
    std::vector<vk::ClearValue> &clearValue(const std::string &renderPassName);
};