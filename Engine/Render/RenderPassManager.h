#pragma once
#include <json.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.hpp>

using json = nlohmann::json;
namespace Vkbase
{
class CommandBuffer;
class CommandPool;
} // namespace Vkbase

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

    std::unordered_map<std::string, std::vector<std::pair<bool, Vkbase::CommandBuffer *>>> _secondaryCommandBuffers;

    static std::vector<RenderPassInfo> processConfig(const json &config);

public:
    RenderPassManager(const json &config);
    ~RenderPassManager();
    void draw(Vkbase::CommandBuffer *pCommandBuffer, RenderObjectManager *pObjects, uint32_t imageIndex, uint32_t frameIndex);
    void registSecondaryBuffer(const std::string &commandPoolName, const std::string &pipelineName, uint32_t frameIndex);
    Vkbase::CommandBuffer *recordSecondaryBuffer(const std::string &commandPoolName, RenderObjectManager *pObjects, const std::string &framebufferName,
                                                 const std::string &renderPassName, const std::string &pipelineName, uint32_t imageIndex, uint32_t frameIndex);
    void shouldRecordFor(const std::string &pipelineName);
    void shouldRecordFor();

    std::vector<vk::ClearValue> &clearValue(const std::string &renderPassName);
};