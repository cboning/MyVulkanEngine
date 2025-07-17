#pragma once
#include <GLFW/glfw3.h>

#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace Vkbase {
enum class ResourceType {
  Unknown,
  Window,
  Swapchain,
  Device,
  Image,
  Buffer,
  Pipeline,
  Framebuffer,
  CommandPool,
  RenderPass,
  Sampler,
  DescriptorSets,
  RenderDelegator
};

class ResourceBase;

typedef std::unordered_map<ResourceType,
                           std::unordered_map<std::string, ResourceBase *>>
    ResourceSet;

inline std::string toString(ResourceType type) {
  switch (type) {
    case ResourceType::Unknown:
      return "Unknown";
    case ResourceType::Window:
      return "Window";
    case ResourceType::Swapchain:
      return "Swapchain";
    case ResourceType::Device:
      return "Device";
    case ResourceType::Image:
      return "Image";
    case ResourceType::Buffer:
      return "Buffer";
    case ResourceType::Pipeline:
      return "Pipeline";
    case ResourceType::Framebuffer:
      return "Framebuffer";
    case ResourceType::CommandPool:
      return "CommandPool";
    case ResourceType::RenderPass:
      return "RenderPass";
    case ResourceType::Sampler:
      return "Sampler";
    case ResourceType::DescriptorSets:
      return "DescriptorSets";
    case ResourceType::RenderDelegator:
      return "RenderDelegator";
  }
}
class ResourceManager {
 private:
  vk::Instance _instance;
  ResourceSet _pResources;

  void createInstance(
      std::vector<const char *> layers = {"VK_LAYER_KHRONOS_validation"},
      std::vector<const char *> extensions =
          {VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
           "VK_EXT_metal_surface", "VK_KHR_surface"},
      std::string appName = "Vulkan");

 public:
  ResourceManager();
  ~ResourceManager();
  void addResource(ResourceBase *pResource);
  const ResourceSet &resources() const;
  ResourceBase *resource(ResourceType type, std::string name) const;
  void remove(ResourceType type, std::string name);
  const vk::Instance &instance() const;
  void renameResource(const ResourceBase *pResource,
                      const std::string &newName);
};
}  // namespace Vkbase