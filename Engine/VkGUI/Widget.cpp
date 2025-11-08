#include "Widget.h"
#include "../Resources/EmptyTextureResource.h"
#include "../Resources/ResourceManager.h"
#include "../Resources/SamplerResource.h"
#include "../Vkbase/Buffer.h"
#include "../Vkbase/DescriptorSets.h"
#include "../Vkbase/Image.h"
#include "../Vkbase/VkResourceManager.h"
#include <glm/gtc/matrix_transform.hpp>

namespace VkGUI
{
MouseState Widget::mouseState() const { return _mouseState; }

void Widget::onUpdate() {}

void Widget::updateRenderRect()
{
    if (auto sp = _superWidget.lock())
    {
        auto parentRect = sp->_renderRect;
        glm::u32vec2 topLeft = {std::max(_rect.x + parentRect.x, parentRect.x), std::max(_rect.y + parentRect.y, parentRect.y)};
        glm::u32vec2 bottomRight = {std::min(_rect.x + _rect.z + parentRect.x, parentRect.x + parentRect.z),
                                    std::min(_rect.y + _rect.w + parentRect.y, parentRect.y + parentRect.w)};
        _renderRect = glm::u32vec4(topLeft, bottomRight - topLeft);
    }
    else
        _renderRect = glm::max(_rect, glm::u32vec4(0));

    for (auto &subWidget : _subWidgets)
        subWidget->updateRenderRect();
}

void Widget::buildMesh()
{
    static std::vector<Vertex> vertices = {{{0.0f, 0.0f, 0.0f, 0.0f}}, {{1.0f, 0.0f, 1.0f, 0.0f}}, {{1.0f, 1.0f, 1.0f, 1.0f}}, {{0.0f, 1.0f, 0.0f, 1.0f}}};

    static std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};
    if (auto p = _superWidget.lock())
        _mesh = p->_mesh;
    else
        _mesh = std::make_shared<Vkbase::Mesh<Vertex>>(std::string(""), deviceName(), vertices, indices, std::vector<std::vector<std::string>>{});
}

void Widget::processMouse(glm::vec2 pos, MouseEventType eventType)
{
    if (!inRect(pos))
    {
        if (eventType == MouseEventType::Down)
            return;

        if (eventType == MouseEventType::Move && _mouseState == MouseState::Above)
            _mouseState = MouseState::Normal;
        else if (eventType == MouseEventType::Up && _mouseState == MouseState::Clicked)
            _mouseState = MouseState::Normal;
    }
    else
    {
        if (eventType == MouseEventType::Move && _mouseState == MouseState::Normal)
            _mouseState = MouseState::Above;
        else if (eventType == MouseEventType::Down)
            _mouseState = MouseState::Clicked;
        else if (eventType == MouseEventType::Up && _mouseState == MouseState::Clicked)
        {
            _trigger = true;
            _mouseState = MouseState::Above;
        }
    }

    for (auto it = _subWidgets.rbegin(); it != _subWidgets.rend(); ++it)
    {
        if (eventType == MouseEventType::Down)
        {
            if ((*it)->inRect(pos))
            {
                (*it)->processMouse(pos, eventType);
                break;
            }
        }
        else
            (*it)->processMouse(pos, eventType);
    }
}

bool Widget::inRect(glm::vec2 pos)
{
    return !(pos.x < _renderRect.x || pos.y < _renderRect.y || pos.x >= _renderRect.x + _renderRect.z || pos.y >= _renderRect.y + _renderRect.w);
}

void Widget::init()
{
    updateRenderRect();
    delegatorInit();
    buildMesh();
}

Widget::Widget(WidgetType type, const std::string &deviceName, const std::string &textureName)
    : RenderObjectDelegator(deviceName, 0, 0),
      _vertUBOName(createResource<Vkbase::Buffer>("", deviceName, sizeof(WidgetUBOData), vk::BufferUsageFlagBits::eUniformBuffer).lock()->name()),
      _fragUBOName(createResource<Vkbase::Buffer>("", deviceName, sizeof(WidgetUBOFragData), vk::BufferUsageFlagBits::eUniformBuffer).lock()->name()),
      _type(type), _textureName(textureName.empty() ? deviceName + "_" + "Empty" : textureName)
{
}

Widget::~Widget()
{
    for (auto &w : _subWidgets)
        w->_superWidget.reset();
    _subWidgets.clear();
    _commandShouldRecordFunc();
}

void Widget::destroy()
{
    if (auto p = _superWidget.lock())
    {
        auto it = std::find_if(p->_subWidgets.begin(), p->_subWidgets.end(), [this](const std::shared_ptr<Widget> &w) { return w.get() == this; });
        if (it != p->_subWidgets.end())
            p->_subWidgets.erase(it);
    }
}

void Widget::setColor(const glm::vec4 &color) { _color = color; }

void Widget::setRect(const glm::u32vec4 &rect)
{
    _rect = rect;
    updateRenderRect();
    _model = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(_renderRect.x, _renderRect.y, 0.0f)), glm::vec3(rect.z, rect.w, 1.0f));
}

const glm::u32vec4 &Widget::rect() const { return _rect; }

void Widget::setScreenSize(const glm::u32vec2 &screenSize) { _projection = glm::ortho(0.0f, (float)screenSize.x, 0.0f, (float)screenSize.y); }

void Widget::onDraw(const Vkbase::VkResourceManagerHolder::WeakReference &commandBuffer, const std::string &renderPassName, const std::string &pipelineName,
                    uint32_t imageIndex, uint32_t frameIndex) const
{
    if (!_mesh)
        return;

    _mesh->draw(commandBuffer, {{Vkbase::VkResourceManager::instance().resource(Vkbase::VkResourceType::DescriptorSets, descriptorSetsName()), {"UBO", 0}}});

    for (auto &subWidget : _subWidgets)
        subWidget->draw(commandBuffer, renderPassName, pipelineName, imageIndex, frameIndex);
}

void Widget::onUpdateUBO(uint32_t frameIndex) const
{
    auto vertUBO = Vkbase::VkResourceManager::instance().resource(Vkbase::VkResourceType::Buffer, _vertUBOName);
    auto fragUBO = Vkbase::VkResourceManager::instance().resource(Vkbase::VkResourceType::Buffer, _fragUBOName);
    if (auto p = vertUBO.lock<Vkbase::Buffer>())
        if (auto p1 = fragUBO.lock<Vkbase::Buffer>())
        {
            WidgetUBOData vertUBOData;
            vertUBOData.projection = _projection;
            vertUBOData.model = _model;

            WidgetUBOFragData fragUBOData;
            fragUBOData.color = _color;
            if (_mouseState == MouseState::Above)
                fragUBOData.color *= _aboveColorFactor;
            if (_mouseState == MouseState::Clicked)
                fragUBOData.color *= _clickedColorFactor;

            fragUBOData.rect = _renderRect;
            fragUBOData.type = glm::u32vec4((uint32_t)_type, 0, 0, 0);

            p->updateBufferData(&vertUBOData);
            p1->updateBufferData(&fragUBOData);

            for (auto &subWidget : _subWidgets)
                subWidget->update(frameIndex);
            return;
        }
    std::cerr << "[Warning] The UBO is not exist." << std::endl;
}

void Widget::addDescriptorSetsConfig(const Vkbase::VkResourceManagerHolder::WeakReference &descriptorSets)
{
    if (auto p = descriptorSets.lock<Vkbase::DescriptorSets>())
        p->addDescriptorSetCreateConfig("UBO",
                                        {{vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex},
                                         {vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment},
                                         {vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment}},
                                        1);
}

void Widget::writeDescriptorSets(const Vkbase::VkResourceManagerHolder::WeakReference &descriptorSets)
{
    if (auto p = descriptorSets.lock<Vkbase::DescriptorSets>())
    {
        auto vertUBO = Vkbase::VkResourceManager::instance().resource(Vkbase::VkResourceType::Buffer, _vertUBOName);
        auto fragUBO = Vkbase::VkResourceManager::instance().resource(Vkbase::VkResourceType::Buffer, _fragUBOName);
        vk::DescriptorBufferInfo bufferInfo;

        if (auto p1 = vertUBO.lock<Vkbase::Buffer>())
            bufferInfo.setOffset(0).setRange(p1->size());
        p->writeSets("UBO", 0, {{bufferInfo, vertUBO}}, {}, 1);

        if (auto p1 = fragUBO.lock<Vkbase::Buffer>())
            bufferInfo.setOffset(0).setRange(p1->size());
        p->writeSets("UBO", 1, {{bufferInfo, fragUBO}}, {}, 1);

        auto image = Vkbase::VkResourceManager::instance().resource(Vkbase::VkResourceType::Image, _textureName);
        if (image.lock())
            p->writeSets("UBO", 2, {},
                         {{vk::DescriptorImageInfo()
                               .setSampler(Resources::ResourceManager::instance()
                                               .getResource<Resources::SamplerResource>(deviceName(), Vkbase::Sampler::getDefaultCreateInfo())
                                               .sampler())
                               .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
                           image}},
                         1);
        else
        {
            p->writeSets("UBO", 2, {},
                         {{vk::DescriptorImageInfo()
                               .setSampler(Resources::ResourceManager::instance()
                                               .getResource<Resources::SamplerResource>(deviceName(), Vkbase::Sampler::getDefaultCreateInfo())
                                               .sampler())
                               .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
                           Resources::ResourceManager::instance().getResource<Resources::EmptyTextureResource>(deviceName()).texture()}},
                         1);
        }
    }
}

const vk::DescriptorSetLayout &Widget::descriptorSetsLayout()
{
    if (auto p = Vkbase::VkResourceManager::instance().resource(Vkbase::VkResourceType::DescriptorSets, descriptorSetsName()).lock<Vkbase::DescriptorSets>())
        return p->layout("UBO");
    throw std::runtime_error("The DescriptorSet already destroyed.");
}

void Widget::setCommandShouldRecordFunc(const std::function<void()> &func)
{
    _commandShouldRecordFunc = func;
    for (auto &subWidget : _subWidgets)
        subWidget->setCommandShouldRecordFunc(func);
    func();
}
} // namespace VkGUI