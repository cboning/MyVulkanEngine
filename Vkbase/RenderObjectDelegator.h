#pragma once
#include "ResourcesDelegator.h"
#include <any>

class Camera;
namespace Vkbase
{
class CommandBuffer;
class DescriptorSets;
class RenderObjectDelegator : public ResourcesDelegator
{
public:
    RenderObjectDelegator(const std::string &deviceName, const Camera &camera, uint32_t flightFrameCount, vk::DeviceSize uboSize);
    void draw(CommandBuffer *pCommandBuffer, uint32_t frameIndex, const std::vector<std::any> &args) const;

protected:
    uint32_t flightFrameCount() const;
    void delegatorInit();

    void createUBOs(vk::DeviceSize size);
    void writeUBODescriptorSets(const std::string &name, uint32_t binding) const;
    void writeUBODescriptorSets(const std::string &name, uint32_t binding, uint32_t firstIndex, uint32_t secondIndex) const;
    void updateUBO(uint32_t frameIndex, const void *pData) const;
    virtual void addDescriptorSetsConfig(DescriptorSets &descriptorSets) = 0;
    virtual void writeDescriptorSets(DescriptorSets &descriptorSets) = 0;
    virtual void onUpdateUBO(uint32_t frameIndex, const std::vector<std::any> &args) const = 0;
    virtual void onDraw(CommandBuffer *pCommandBuffer, uint32_t frameIndex, const std::vector<std::any> &args) const = 0;
    const Camera &camera() const;
    const std::string &descriptorSetsName() const;
    const std::vector<std::string> &uboNames() const;
    const std::string &deviceName() const;

private:
    uint32_t _flightFrameCount;
    const std::string _descriptorSetsName;
    const std::string _deviceName;
    const Camera &_camera;
    std::vector<std::string> _uboNames;
};
} // namespace Vkbase