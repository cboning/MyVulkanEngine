#include "Frame.h"
#include "../Vkbase/Mesh.h"
#include "../Vkbase/VkResourceManager.h"
#include <memory>

namespace VkGUI
{
Frame::Frame(const std::string &deviceName) : Widget(WidgetType::Frame, deviceName) {}
} // namespace VkGUI