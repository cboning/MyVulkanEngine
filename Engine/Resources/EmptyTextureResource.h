#pragma once

#include "../Vkbase/VkResourcesDelegator.h"
#include "ResourceBase.h"

namespace Vkbase
{
    class Image;
}


namespace Resources
{
class EmptyTextureResource : public ResourceBase, public Vkbase::VkResourcesDelegator
{
    friend class ResourceManager;

public:
    Vkbase::Image *texture() const;


private:
    std::string _textureName;

    EmptyTextureResource(const std::string &deviceName);
    ~EmptyTextureResource() override;

    static std::string getNameByArgument(const std::string &deviceName);
};
} // namespace Resources
