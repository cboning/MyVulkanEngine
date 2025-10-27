#pragma once
#include "VkResourcesDelegator.h"

class Camera;
namespace Vkbase
{
class CommandBuffer;
class DescriptorSets;
class RenderObjectDelegator : public VkResourcesDelegator
{
public:
    RenderObjectDelegator(const std::string &deviceName, const Camera &camera, uint32_t flightFrameCount, vk::DeviceSize uboSize);
    RenderObjectDelegator(const std::string &deviceName, uint32_t flightFrameCount, vk::DeviceSize uboSize);
    virtual ~RenderObjectDelegator() = default;
    void draw(CommandBuffer *pCommandBuffer, const std::string &renderPassName, const std::string &pipelineName, uint32_t imageIndex,
              uint32_t frameIndex) const;
    void update(uint32_t frameIndex) const;

protected:
    uint32_t flightFrameCount() const;
    void delegatorInit();

    void createUBOs(vk::DeviceSize size);
    void writeUBODescriptorSets(const std::string &name, uint32_t binding) const;
    void writeUBODescriptorSets(const std::string &name, uint32_t binding, uint32_t firstIndex, uint32_t secondIndex) const;
    void updateUBO(uint32_t frameIndex, const void *pData) const;
    virtual void addDescriptorSetsConfig(DescriptorSets &descriptorSets) = 0;
    virtual void writeDescriptorSets(DescriptorSets &descriptorSets) = 0;
    virtual void onUpdateUBO(uint32_t frameIndex) const = 0;
    virtual void onDraw(CommandBuffer *pCommandBuffer, const std::string &renderPassName, const std::string &pipelineName, uint32_t imageIndex,
                        uint32_t frameIndex) const = 0;
    const Camera &camera() const;
    const std::string &descriptorSetsName() const;
    const std::vector<std::string> &uboNames() const;
    const std::string &deviceName() const;

private:
    uint32_t _flightFrameCount;
    const std::string _descriptorSetsName;
    const std::string _deviceName;
    const Camera *const _pCamera = nullptr;
    std::vector<std::string> _uboNames;
};
} // namespace Vkbase