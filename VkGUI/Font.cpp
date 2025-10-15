#include "Font.h"
#include "../Vkbase/Buffer.h"
#include "../Vkbase/DescriptorSets.h"
#include <glm/gtc/matrix_transform.hpp>

Font::Font(const std::string &deviceName, const std::string &filename)
    : _deviceName(deviceName), _sampler(*(createResource<Vkbase::Sampler>("", deviceName))),
      _descriptorSets(*(createResource<Vkbase::DescriptorSets>("Text", deviceName)))
{
    _descriptorSets.addDescriptorSetCreateConfig("Character", {{vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment}}, 128);
    _descriptorSets.init();

    std::pair<vk::DescriptorImageInfo, Vkbase::Image *> imageInfo;
    imageInfo.first.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setSampler(_sampler.sampler());
    std::vector<std::pair<vk::DescriptorImageInfo, Vkbase::Image *>> imageInfos(128, imageInfo);

    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
#ifdef DEBUG
        std::cout << "[Error] Could not init FreeType Library" << std::endl;
#endif
    }

    FT_Face face;
    if (FT_New_Face(ft, filename.c_str(), 0, &face))
    {
#ifdef DEBUG
        std::cout << "[Error] Failed to load font" << std::endl;
#endif
    }
    FT_Set_Pixel_Sizes(face, 0, 32);
    for (GLubyte i = 0; i < 128; ++i)
    {
        auto [it, inserted] = _characters.try_emplace(i, deviceName, face, i);
        imageInfos[i].second = &it->second.image();
    }

    _descriptorSets.writeSets("Character", 0, {}, imageInfos, 128);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

const std::string &Font::deviceName() const { return _deviceName; }

const std::unordered_map<FT_ULong, Font::Character> &Font::characters() const { return _characters; }

std::pair<Vkbase::DescriptorSets *, std::pair<std::string, uint32_t>> Font::set(FT_ULong character) const
{
    return {&_descriptorSets, {"Character", character}};
}

const vk::DescriptorSetLayout &Font::layout() const { return _descriptorSets.layout("Character"); }

void Font::addProjectiveDescriptorSet(const std::string &descriptorSetsName)
{
    Vkbase::DescriptorSets &descriptorSets =
        *(dynamic_cast<Vkbase::DescriptorSets *>(Vkbase::DescriptorSets::resourceManager().resource(Vkbase::ResourceType::DescriptorSets, descriptorSetsName)));

    descriptorSets.addDescriptorSetCreateConfig("FontProjective", {{vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex}}, 1);
}

void Font::writeProjectiveDescriptorSet(const std::string &descriptorSetsName, const std::string &deviceName)
{
    Vkbase::DescriptorSets &descriptorSets =
        *(dynamic_cast<Vkbase::DescriptorSets *>(Vkbase::DescriptorSets::resourceManager().resource(Vkbase::ResourceType::DescriptorSets, descriptorSetsName)));

    std::pair<vk::DescriptorBufferInfo, Vkbase::Buffer *> bufferInfo;
    bufferInfo.first.setOffset(0).setRange(sizeof(UniformBufferData));

    _pProjectiveUniformBuffer = std::make_unique<ProjectiveUniformBuffer>(deviceName);

    bufferInfo.second = _pProjectiveUniformBuffer->getUBO();
    descriptorSets.writeSets("FontProjective", 0, {bufferInfo}, {}, 1);
}

void Font::setScreenSize(const glm::ivec2 &screenSize)
{
    UniformBufferData uniformBufferData;
    uniformBufferData.projection = glm::ortho(0.0f, (float)screenSize.x, 0.0f, (float)screenSize.y);
    uniformBufferData.projection[1][1] *= -1;
    Vkbase::Buffer &projectiveUniformBuffer =
        *(dynamic_cast<Vkbase::Buffer *>(Vkbase::DescriptorSets::resourceManager().resource(Vkbase::ResourceType::Buffer, "FontProjectiveUniformBuffer")));
    projectiveUniformBuffer.updateBufferData(&uniformBufferData);
}

const vk::DescriptorSetLayout &Font::projectiveLayout(const std::string &descriptorSetsName)
{
    Vkbase::DescriptorSets &descriptorSets =
        *(dynamic_cast<Vkbase::DescriptorSets *>(Vkbase::DescriptorSets::resourceManager().resource(Vkbase::ResourceType::DescriptorSets, descriptorSetsName)));
    return descriptorSets.layout("FontProjective");
}
std::pair<Vkbase::DescriptorSets *, std::pair<std::string, uint32_t>> Font::projectiveSet(const std::string &descriptorSetsName)
{
    Vkbase::DescriptorSets &descriptorSets =
        *(dynamic_cast<Vkbase::DescriptorSets *>(Vkbase::DescriptorSets::resourceManager().resource(Vkbase::ResourceType::DescriptorSets, descriptorSetsName)));
    return {&descriptorSets, {"FontProjective", 0}};
}

Font::ProjectiveUniformBuffer::ProjectiveUniformBuffer(const std::string &deviceName)
{
    _pUBO = createResource<Vkbase::Buffer>("FontProjectiveUniformBuffer", deviceName, sizeof(UniformBufferData), vk::BufferUsageFlagBits::eUniformBuffer);
}

Vkbase::Buffer *Font::ProjectiveUniformBuffer::getUBO() { return _pUBO; }
