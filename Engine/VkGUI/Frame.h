#pragma once
#include "Widget.h"
#include <glm/glm.hpp>
#include <memory>

namespace VkGUI
{
class Frame : public Widget
{
    friend class Widget;

public:
    virtual ~Frame() = default;

protected:
    Frame(const std::string &deviceName);
};
} // namespace VkGUI