#include "Font.h"
#include "../Vkbase/Buffer.h"
#include "../Vkbase/DescriptorSets.h"
#include <glm/gtc/matrix_transform.hpp>

Font::Font(const std::string &deviceName, const std::string &filename)
    : _deviceName(deviceName), _sampler(*(Vkbase::ResourceBase::resourceManager().create<Vkbase::Sampler>("", deviceName))), _descriptorSets(*(Vkbase::ResourceBase::resourceManager().create<Vkbase::DescriptorSets>("Text", deviceName)))
{
    _descriptorSets.addDescriptorSetCreateConfig("Character", {{vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment}}, 128);
    _descriptorSets.init();

    vk::DescriptorImageInfo imageInfo;
    imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setSampler(_sampler.sampler());
    std::vector<vk::DescriptorImageInfo> imageInfos(128, imageInfo);

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
        Character character(deviceName, face, i);
        _characters.insert(std::pair<GLchar, Character>(i, character));

        imageInfos[i].setImageView(character.image().view());
    }

    _descriptorSets.writeSets("Character", 0, {}, imageInfos, 128);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

Font::~Font()
{
    _descriptorSets.destroy();
    _sampler.destroy();
}

const std::string &Font::deviceName() const { return _deviceName; }

const std::unordered_map<FT_ULong, Font::Character> &Font::characters() const { return _characters; }

const vk::DescriptorSet &Font::set(FT_ULong character) const { return _descriptorSets.sets("Character")[character]; }

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

    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.setOffset(0)
        .setRange(sizeof(UniformBufferData))
        .setBuffer(
            (Vkbase::ResourceBase::resourceManager().create<Vkbase::Buffer>("FontProjectiveUniformBuffer", deviceName, sizeof(UniformBufferData), vk::BufferUsageFlagBits::eUniformBuffer))->buffer());
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
const vk::DescriptorSet &Font::projectiveSet(const std::string &descriptorSetsName)
{
    Vkbase::DescriptorSets &descriptorSets =
        *(dynamic_cast<Vkbase::DescriptorSets *>(Vkbase::DescriptorSets::resourceManager().resource(Vkbase::ResourceType::DescriptorSets, descriptorSetsName)));
    return descriptorSets.sets("FontProjective")[0];
}