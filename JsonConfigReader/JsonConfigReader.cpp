#include "JsonConfigReader.h"
#include "../Vkbase/Swapchain.h"
#include <fstream>
#include <functional>
#include <iostream>

aiTextureType JsonConfigReader::getTextureTypeWithString(const std::string &textureType)
{
    static const std::unordered_map<std::string, aiTextureType> typeMap = {{"NONE", aiTextureType_NONE},
                                                                           {"DIFFUSE", aiTextureType_DIFFUSE},
                                                                           {"SPECULAR", aiTextureType_SPECULAR},
                                                                           {"AMBIENT", aiTextureType_AMBIENT},
                                                                           {"EMISSIVE", aiTextureType_EMISSIVE},
                                                                           {"HEIGHT", aiTextureType_HEIGHT},
                                                                           {"NORMALS", aiTextureType_NORMALS},
                                                                           {"SHININESS", aiTextureType_SHININESS},
                                                                           {"OPACITY", aiTextureType_OPACITY},
                                                                           {"DISPLACEMENT", aiTextureType_DISPLACEMENT},
                                                                           {"LIGHTMAP", aiTextureType_LIGHTMAP},
                                                                           {"REFLECTION", aiTextureType_REFLECTION},
                                                                           {"BASE_COLOR", aiTextureType_BASE_COLOR},
                                                                           {"NORMAL_CAMERA", aiTextureType_NORMAL_CAMERA},
                                                                           {"EMISSION_COLOR", aiTextureType_EMISSION_COLOR},
                                                                           {"METALNESS", aiTextureType_METALNESS},
                                                                           {"DIFFUSE_ROUGHNESS", aiTextureType_DIFFUSE_ROUGHNESS},
                                                                           {"AMBIENT_OCCLUSION", aiTextureType_AMBIENT_OCCLUSION},
                                                                           {"SHEEN", aiTextureType_SHEEN},
                                                                           {"CLEARCOAT", aiTextureType_CLEARCOAT},
                                                                           {"TRANSMISSION", aiTextureType_TRANSMISSION},
                                                                           {"MAYA_BASE", aiTextureType_MAYA_BASE},
                                                                           {"MAYA_SPECULAR", aiTextureType_MAYA_SPECULAR},
                                                                           {"MAYA_SPECULAR_COLOR", aiTextureType_MAYA_SPECULAR_COLOR},
                                                                           {"MAYA_SPECULAR_ROUGHNESS", aiTextureType_MAYA_SPECULAR_ROUGHNESS},
                                                                           {"ANISOTROPY", aiTextureType_ANISOTROPY},
                                                                           {"GLTF_METALLIC_ROUGHNESS", aiTextureType_GLTF_METALLIC_ROUGHNESS},
                                                                           {"UNKNOWN", aiTextureType_UNKNOWN}};

    auto it = typeMap.find(textureType);
    if (it != typeMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown texture type: " + textureType);
}

vk::Format JsonConfigReader::getFormatWithJson(const std::string &format, const std::string &swapchainName, vk::Format depthFormat)
{
    static const std::unordered_map<std::string, vk::Format> formatMap = {
        {"Undefined", vk::Format::eUndefined},
        {"R4G4UnormPack8", vk::Format::eR4G4UnormPack8},
        {"R4G4B4A4UnormPack16", vk::Format::eR4G4B4A4UnormPack16},
        {"B4G4R4A4UnormPack16", vk::Format::eB4G4R4A4UnormPack16},
        {"R5G6B5UnormPack16", vk::Format::eR5G6B5UnormPack16},
        {"B5G6R5UnormPack16", vk::Format::eB5G6R5UnormPack16},
        {"R5G5B5A1UnormPack16", vk::Format::eR5G5B5A1UnormPack16},
        {"B5G5R5A1UnormPack16", vk::Format::eB5G5R5A1UnormPack16},
        {"A1R5G5B5UnormPack16", vk::Format::eA1R5G5B5UnormPack16},
        {"R8Unorm", vk::Format::eR8Unorm},
        {"R8Snorm", vk::Format::eR8Snorm},
        {"R8Uscaled", vk::Format::eR8Uscaled},
        {"R8Sscaled", vk::Format::eR8Sscaled},
        {"R8Uint", vk::Format::eR8Uint},
        {"R8Sint", vk::Format::eR8Sint},
        {"R8Srgb", vk::Format::eR8Srgb},
        {"R8G8Unorm", vk::Format::eR8G8Unorm},
        {"R8G8Snorm", vk::Format::eR8G8Snorm},
        {"R8G8Uscaled", vk::Format::eR8G8Uscaled},
        {"R8G8Sscaled", vk::Format::eR8G8Sscaled},
        {"R8G8Uint", vk::Format::eR8G8Uint},
        {"R8G8Sint", vk::Format::eR8G8Sint},
        {"R8G8Srgb", vk::Format::eR8G8Srgb},
        {"R8G8B8Unorm", vk::Format::eR8G8B8Unorm},
        {"R8G8B8Snorm", vk::Format::eR8G8B8Snorm},
        {"R8G8B8Uscaled", vk::Format::eR8G8B8Uscaled},
        {"R8G8B8Sscaled", vk::Format::eR8G8B8Sscaled},
        {"R8G8B8Uint", vk::Format::eR8G8B8Uint},
        {"R8G8B8Sint", vk::Format::eR8G8B8Sint},
        {"R8G8B8Srgb", vk::Format::eR8G8B8Srgb},
        {"B8G8R8Unorm", vk::Format::eB8G8R8Unorm},
        {"B8G8R8Snorm", vk::Format::eB8G8R8Snorm},
        {"B8G8R8Uscaled", vk::Format::eB8G8R8Uscaled},
        {"B8G8R8Sscaled", vk::Format::eB8G8R8Sscaled},
        {"B8G8R8Uint", vk::Format::eB8G8R8Uint},
        {"B8G8R8Sint", vk::Format::eB8G8R8Sint},
        {"B8G8R8Srgb", vk::Format::eB8G8R8Srgb},
        {"R8G8B8A8Unorm", vk::Format::eR8G8B8A8Unorm},
        {"R8G8B8A8Snorm", vk::Format::eR8G8B8A8Snorm},
        {"R8G8B8A8Uscaled", vk::Format::eR8G8B8A8Uscaled},
        {"R8G8B8A8Sscaled", vk::Format::eR8G8B8A8Sscaled},
        {"R8G8B8A8Uint", vk::Format::eR8G8B8A8Uint},
        {"R8G8B8A8Sint", vk::Format::eR8G8B8A8Sint},
        {"R8G8B8A8Srgb", vk::Format::eR8G8B8A8Srgb},
        {"B8G8R8A8Unorm", vk::Format::eB8G8R8A8Unorm},
        {"B8G8R8A8Snorm", vk::Format::eB8G8R8A8Snorm},
        {"B8G8R8A8Uscaled", vk::Format::eB8G8R8A8Uscaled},
        {"B8G8R8A8Sscaled", vk::Format::eB8G8R8A8Sscaled},
        {"B8G8R8A8Uint", vk::Format::eB8G8R8A8Uint},
        {"B8G8R8A8Sint", vk::Format::eB8G8R8A8Sint},
        {"B8G8R8A8Srgb", vk::Format::eB8G8R8A8Srgb},
        {"A8B8G8R8UnormPack32", vk::Format::eA8B8G8R8UnormPack32},
        {"A8B8G8R8SnormPack32", vk::Format::eA8B8G8R8SnormPack32},
        {"A8B8G8R8UscaledPack32", vk::Format::eA8B8G8R8UscaledPack32},
        {"A8B8G8R8SscaledPack32", vk::Format::eA8B8G8R8SscaledPack32},
        {"A8B8G8R8UintPack32", vk::Format::eA8B8G8R8UintPack32},
        {"A8B8G8R8SintPack32", vk::Format::eA8B8G8R8SintPack32},
        {"A8B8G8R8SrgbPack32", vk::Format::eA8B8G8R8SrgbPack32},
        {"A2R10G10B10UnormPack32", vk::Format::eA2R10G10B10UnormPack32},
        {"A2R10G10B10SnormPack32", vk::Format::eA2R10G10B10SnormPack32},
        {"A2R10G10B10UscaledPack32", vk::Format::eA2R10G10B10UscaledPack32},
        {"A2R10G10B10SscaledPack32", vk::Format::eA2R10G10B10SscaledPack32},
        {"A2R10G10B10UintPack32", vk::Format::eA2R10G10B10UintPack32},
        {"A2R10G10B10SintPack32", vk::Format::eA2R10G10B10SintPack32},
        {"A2B10G10R10UnormPack32", vk::Format::eA2B10G10R10UnormPack32},
        {"A2B10G10R10SnormPack32", vk::Format::eA2B10G10R10SnormPack32},
        {"A2B10G10R10UscaledPack32", vk::Format::eA2B10G10R10UscaledPack32},
        {"A2B10G10R10SscaledPack32", vk::Format::eA2B10G10R10SscaledPack32},
        {"A2B10G10R10UintPack32", vk::Format::eA2B10G10R10UintPack32},
        {"A2B10G10R10SintPack32", vk::Format::eA2B10G10R10SintPack32},
        {"R16Unorm", vk::Format::eR16Unorm},
        {"R16Snorm", vk::Format::eR16Snorm},
        {"R16Uscaled", vk::Format::eR16Uscaled},
        {"R16Sscaled", vk::Format::eR16Sscaled},
        {"R16Uint", vk::Format::eR16Uint},
        {"R16Sint", vk::Format::eR16Sint},
        {"R16Sfloat", vk::Format::eR16Sfloat},
        {"R16G16Unorm", vk::Format::eR16G16Unorm},
        {"R16G16Snorm", vk::Format::eR16G16Snorm},
        {"R16G16Uscaled", vk::Format::eR16G16Uscaled},
        {"R16G16Sscaled", vk::Format::eR16G16Sscaled},
        {"R16G16Uint", vk::Format::eR16G16Uint},
        {"R16G16Sint", vk::Format::eR16G16Sint},
        {"R16G16Sfloat", vk::Format::eR16G16Sfloat},
        {"R16G16B16Unorm", vk::Format::eR16G16B16Unorm},
        {"R16G16B16Snorm", vk::Format::eR16G16B16Snorm},
        {"R16G16B16Uscaled", vk::Format::eR16G16B16Uscaled},
        {"R16G16B16Sscaled", vk::Format::eR16G16B16Sscaled},
        {"R16G16B16Uint", vk::Format::eR16G16B16Uint},
        {"R16G16B16Sint", vk::Format::eR16G16B16Sint},
        {"R16G16B16Sfloat", vk::Format::eR16G16B16Sfloat},
        {"R16G16B16A16Unorm", vk::Format::eR16G16B16A16Unorm},
        {"R16G16B16A16Snorm", vk::Format::eR16G16B16A16Snorm},
        {"R16G16B16A16Uscaled", vk::Format::eR16G16B16A16Uscaled},
        {"R16G16B16A16Sscaled", vk::Format::eR16G16B16A16Sscaled},
        {"R16G16B16A16Uint", vk::Format::eR16G16B16A16Uint},
        {"R16G16B16A16Sint", vk::Format::eR16G16B16A16Sint},
        {"R16G16B16A16Sfloat", vk::Format::eR16G16B16A16Sfloat},
        {"R32Uint", vk::Format::eR32Uint},
        {"R32Sint", vk::Format::eR32Sint},
        {"R32Sfloat", vk::Format::eR32Sfloat},
        {"R32G32Uint", vk::Format::eR32G32Uint},
        {"R32G32Sint", vk::Format::eR32G32Sint},
        {"R32G32Sfloat", vk::Format::eR32G32Sfloat},
        {"R32G32B32Uint", vk::Format::eR32G32B32Uint},
        {"R32G32B32Sint", vk::Format::eR32G32B32Sint},
        {"R32G32B32Sfloat", vk::Format::eR32G32B32Sfloat},
        {"R32G32B32A32Uint", vk::Format::eR32G32B32A32Uint},
        {"R32G32B32A32Sint", vk::Format::eR32G32B32A32Sint},
        {"R32G32B32A32Sfloat", vk::Format::eR32G32B32A32Sfloat},
        {"R64Uint", vk::Format::eR64Uint},
        {"R64Sint", vk::Format::eR64Sint},
        {"R64Sfloat", vk::Format::eR64Sfloat},
        {"R64G64Uint", vk::Format::eR64G64Uint},
        {"R64G64Sint", vk::Format::eR64G64Sint},
        {"R64G64Sfloat", vk::Format::eR64G64Sfloat},
        {"R64G64B64Uint", vk::Format::eR64G64B64Uint},
        {"R64G64B64Sint", vk::Format::eR64G64B64Sint},
        {"R64G64B64Sfloat", vk::Format::eR64G64B64Sfloat},
        {"R64G64B64A64Uint", vk::Format::eR64G64B64A64Uint},
        {"R64G64B64A64Sint", vk::Format::eR64G64B64A64Sint},
        {"R64G64B64A64Sfloat", vk::Format::eR64G64B64A64Sfloat},
        {"B10G11R11UfloatPack32", vk::Format::eB10G11R11UfloatPack32},
        {"E5B9G9R9UfloatPack32", vk::Format::eE5B9G9R9UfloatPack32},
        {"D16Unorm", vk::Format::eD16Unorm},
        {"X8D24UnormPack32", vk::Format::eX8D24UnormPack32},
        {"D32Sfloat", vk::Format::eD32Sfloat},
        {"S8Uint", vk::Format::eS8Uint},
        {"D16UnormS8Uint", vk::Format::eD16UnormS8Uint},
        {"D24UnormS8Uint", vk::Format::eD24UnormS8Uint},
        {"D32SfloatS8Uint", vk::Format::eD32SfloatS8Uint},
        {"Bc1RgbUnormBlock", vk::Format::eBc1RgbUnormBlock},
        {"Bc1RgbSrgbBlock", vk::Format::eBc1RgbSrgbBlock},
        {"Bc1RgbaUnormBlock", vk::Format::eBc1RgbaUnormBlock},
        {"Bc1RgbaSrgbBlock", vk::Format::eBc1RgbaSrgbBlock},
        {"Bc2UnormBlock", vk::Format::eBc2UnormBlock},
        {"Bc2SrgbBlock", vk::Format::eBc2SrgbBlock},
        {"Bc3UnormBlock", vk::Format::eBc3UnormBlock},
        {"Bc3SrgbBlock", vk::Format::eBc3SrgbBlock},
        {"Bc4UnormBlock", vk::Format::eBc4UnormBlock},
        {"Bc4SnormBlock", vk::Format::eBc4SnormBlock},
        {"Bc5UnormBlock", vk::Format::eBc5UnormBlock},
        {"Bc5SnormBlock", vk::Format::eBc5SnormBlock},
        {"Bc6HUfloatBlock", vk::Format::eBc6HUfloatBlock},
        {"Bc6HSfloatBlock", vk::Format::eBc6HSfloatBlock},
        {"Bc7UnormBlock", vk::Format::eBc7UnormBlock},
        {"Bc7SrgbBlock", vk::Format::eBc7SrgbBlock},
        {"Etc2R8G8B8UnormBlock", vk::Format::eEtc2R8G8B8UnormBlock},
        {"Etc2R8G8B8SrgbBlock", vk::Format::eEtc2R8G8B8SrgbBlock},
        {"Etc2R8G8B8A1UnormBlock", vk::Format::eEtc2R8G8B8A1UnormBlock},
        {"Etc2R8G8B8A1SrgbBlock", vk::Format::eEtc2R8G8B8A1SrgbBlock},
        {"Etc2R8G8B8A8UnormBlock", vk::Format::eEtc2R8G8B8A8UnormBlock},
        {"Etc2R8G8B8A8SrgbBlock", vk::Format::eEtc2R8G8B8A8SrgbBlock},
        {"EacR11UnormBlock", vk::Format::eEacR11UnormBlock},
        {"EacR11SnormBlock", vk::Format::eEacR11SnormBlock},
        {"EacR11G11UnormBlock", vk::Format::eEacR11G11UnormBlock},
        {"EacR11G11SnormBlock", vk::Format::eEacR11G11SnormBlock},
        {"Astc4x4UnormBlock", vk::Format::eAstc4x4UnormBlock},
        {"Astc4x4SrgbBlock", vk::Format::eAstc4x4SrgbBlock},
        {"Astc5x4UnormBlock", vk::Format::eAstc5x4UnormBlock},
        {"Astc5x4SrgbBlock", vk::Format::eAstc5x4SrgbBlock},
        {"Astc5x5UnormBlock", vk::Format::eAstc5x5UnormBlock},
        {"Astc5x5SrgbBlock", vk::Format::eAstc5x5SrgbBlock},
        {"Astc6x5UnormBlock", vk::Format::eAstc6x5UnormBlock},
        {"Astc6x5SrgbBlock", vk::Format::eAstc6x5SrgbBlock},
        {"Astc6x6UnormBlock", vk::Format::eAstc6x6UnormBlock},
        {"Astc6x6SrgbBlock", vk::Format::eAstc6x6SrgbBlock},
        {"Astc8x5UnormBlock", vk::Format::eAstc8x5UnormBlock},
        {"Astc8x5SrgbBlock", vk::Format::eAstc8x5SrgbBlock},
        {"Astc8x6UnormBlock", vk::Format::eAstc8x6UnormBlock},
        {"Astc8x6SrgbBlock", vk::Format::eAstc8x6SrgbBlock},
        {"Astc8x8UnormBlock", vk::Format::eAstc8x8UnormBlock},
        {"Astc8x8SrgbBlock", vk::Format::eAstc8x8SrgbBlock},
        {"Astc10x5UnormBlock", vk::Format::eAstc10x5UnormBlock},
        {"Astc10x5SrgbBlock", vk::Format::eAstc10x5SrgbBlock},
        {"Astc10x6UnormBlock", vk::Format::eAstc10x6UnormBlock},
        {"Astc10x6SrgbBlock", vk::Format::eAstc10x6SrgbBlock},
        {"Astc10x8UnormBlock", vk::Format::eAstc10x8UnormBlock},
        {"Astc10x8SrgbBlock", vk::Format::eAstc10x8SrgbBlock},
        {"Astc10x10UnormBlock", vk::Format::eAstc10x10UnormBlock},
        {"Astc10x10SrgbBlock", vk::Format::eAstc10x10SrgbBlock},
        {"Astc12x10UnormBlock", vk::Format::eAstc12x10UnormBlock},
        {"Astc12x10SrgbBlock", vk::Format::eAstc12x10SrgbBlock},
        {"Astc12x12UnormBlock", vk::Format::eAstc12x12UnormBlock},
        {"Astc12x12SrgbBlock", vk::Format::eAstc12x12SrgbBlock},
        {"G8B8G8R8422Unorm", vk::Format::eG8B8G8R8422Unorm},
        {"G8B8G8R8422UnormKHR", vk::Format::eG8B8G8R8422UnormKHR},
        {"B8G8R8G8422Unorm", vk::Format::eB8G8R8G8422Unorm},
        {"B8G8R8G8422UnormKHR", vk::Format::eB8G8R8G8422UnormKHR},
        {"G8B8R83Plane420Unorm", vk::Format::eG8B8R83Plane420Unorm},
        {"G8B8R83Plane420UnormKHR", vk::Format::eG8B8R83Plane420UnormKHR},
        {"G8B8R82Plane420Unorm", vk::Format::eG8B8R82Plane420Unorm},
        {"G8B8R82Plane420UnormKHR", vk::Format::eG8B8R82Plane420UnormKHR},
        {"G8B8R83Plane422Unorm", vk::Format::eG8B8R83Plane422Unorm},
        {"G8B8R83Plane422UnormKHR", vk::Format::eG8B8R83Plane422UnormKHR},
        {"G8B8R82Plane422Unorm", vk::Format::eG8B8R82Plane422Unorm},
        {"G8B8R82Plane422UnormKHR", vk::Format::eG8B8R82Plane422UnormKHR},
        {"G8B8R83Plane444Unorm", vk::Format::eG8B8R83Plane444Unorm},
        {"G8B8R83Plane444UnormKHR", vk::Format::eG8B8R83Plane444UnormKHR},
        {"R10X6UnormPack16", vk::Format::eR10X6UnormPack16},
        {"R10X6UnormPack16KHR", vk::Format::eR10X6UnormPack16KHR},
        {"R10X6G10X6Unorm2Pack16", vk::Format::eR10X6G10X6Unorm2Pack16},
        {"R10X6G10X6Unorm2Pack16KHR", vk::Format::eR10X6G10X6Unorm2Pack16KHR},
        {"R10X6G10X6B10X6A10X6Unorm4Pack16", vk::Format::eR10X6G10X6B10X6A10X6Unorm4Pack16},
        {"R10X6G10X6B10X6A10X6Unorm4Pack16KHR", vk::Format::eR10X6G10X6B10X6A10X6Unorm4Pack16KHR},
        {"G10X6B10X6G10X6R10X6422Unorm4Pack16", vk::Format::eG10X6B10X6G10X6R10X6422Unorm4Pack16},
        {"G10X6B10X6G10X6R10X6422Unorm4Pack16KHR", vk::Format::eG10X6B10X6G10X6R10X6422Unorm4Pack16KHR},
        {"B10X6G10X6R10X6G10X6422Unorm4Pack16", vk::Format::eB10X6G10X6R10X6G10X6422Unorm4Pack16},
        {"B10X6G10X6R10X6G10X6422Unorm4Pack16KHR", vk::Format::eB10X6G10X6R10X6G10X6422Unorm4Pack16KHR},
        {"G10X6B10X6R10X63Plane420Unorm3Pack16", vk::Format::eG10X6B10X6R10X63Plane420Unorm3Pack16},
        {"G10X6B10X6R10X63Plane420Unorm3Pack16KHR", vk::Format::eG10X6B10X6R10X63Plane420Unorm3Pack16KHR},
        {"G10X6B10X6R10X62Plane420Unorm3Pack16", vk::Format::eG10X6B10X6R10X62Plane420Unorm3Pack16},
        {"G10X6B10X6R10X62Plane420Unorm3Pack16KHR", vk::Format::eG10X6B10X6R10X62Plane420Unorm3Pack16KHR},
        {"G10X6B10X6R10X63Plane422Unorm3Pack16", vk::Format::eG10X6B10X6R10X63Plane422Unorm3Pack16},
        {"G10X6B10X6R10X63Plane422Unorm3Pack16KHR", vk::Format::eG10X6B10X6R10X63Plane422Unorm3Pack16KHR},
        {"G10X6B10X6R10X62Plane422Unorm3Pack16", vk::Format::eG10X6B10X6R10X62Plane422Unorm3Pack16},
        {"G10X6B10X6R10X62Plane422Unorm3Pack16KHR", vk::Format::eG10X6B10X6R10X62Plane422Unorm3Pack16KHR},
        {"G10X6B10X6R10X63Plane444Unorm3Pack16", vk::Format::eG10X6B10X6R10X63Plane444Unorm3Pack16},
        {"G10X6B10X6R10X63Plane444Unorm3Pack16KHR", vk::Format::eG10X6B10X6R10X63Plane444Unorm3Pack16KHR},
        {"R12X4UnormPack16", vk::Format::eR12X4UnormPack16},
        {"R12X4UnormPack16KHR", vk::Format::eR12X4UnormPack16KHR},
        {"R12X4G12X4Unorm2Pack16", vk::Format::eR12X4G12X4Unorm2Pack16},
        {"R12X4G12X4Unorm2Pack16KHR", vk::Format::eR12X4G12X4Unorm2Pack16KHR},
        {"R12X4G12X4B12X4A12X4Unorm4Pack16", vk::Format::eR12X4G12X4B12X4A12X4Unorm4Pack16},
        {"R12X4G12X4B12X4A12X4Unorm4Pack16KHR", vk::Format::eR12X4G12X4B12X4A12X4Unorm4Pack16KHR},
        {"G12X4B12X4G12X4R12X4422Unorm4Pack16", vk::Format::eG12X4B12X4G12X4R12X4422Unorm4Pack16},
        {"G12X4B12X4G12X4R12X4422Unorm4Pack16KHR", vk::Format::eG12X4B12X4G12X4R12X4422Unorm4Pack16KHR},
        {"B12X4G12X4R12X4G12X4422Unorm4Pack16", vk::Format::eB12X4G12X4R12X4G12X4422Unorm4Pack16},
        {"B12X4G12X4R12X4G12X4422Unorm4Pack16KHR", vk::Format::eB12X4G12X4R12X4G12X4422Unorm4Pack16KHR},
        {"G12X4B12X4R12X43Plane420Unorm3Pack16", vk::Format::eG12X4B12X4R12X43Plane420Unorm3Pack16},
        {"G12X4B12X4R12X43Plane420Unorm3Pack16KHR", vk::Format::eG12X4B12X4R12X43Plane420Unorm3Pack16KHR},
        {"G12X4B12X4R12X42Plane420Unorm3Pack16", vk::Format::eG12X4B12X4R12X42Plane420Unorm3Pack16},
        {"G12X4B12X4R12X42Plane420Unorm3Pack16KHR", vk::Format::eG12X4B12X4R12X42Plane420Unorm3Pack16KHR},
        {"G12X4B12X4R12X43Plane422Unorm3Pack16", vk::Format::eG12X4B12X4R12X43Plane422Unorm3Pack16},
        {"G12X4B12X4R12X43Plane422Unorm3Pack16KHR", vk::Format::eG12X4B12X4R12X43Plane422Unorm3Pack16KHR},
        {"G12X4B12X4R12X42Plane422Unorm3Pack16", vk::Format::eG12X4B12X4R12X42Plane422Unorm3Pack16},
        {"G12X4B12X4R12X42Plane422Unorm3Pack16KHR", vk::Format::eG12X4B12X4R12X42Plane422Unorm3Pack16KHR},
        {"G12X4B12X4R12X43Plane444Unorm3Pack16", vk::Format::eG12X4B12X4R12X43Plane444Unorm3Pack16},
        {"G12X4B12X4R12X43Plane444Unorm3Pack16KHR", vk::Format::eG12X4B12X4R12X43Plane444Unorm3Pack16KHR},
        {"G16B16G16R16422Unorm", vk::Format::eG16B16G16R16422Unorm},
        {"G16B16G16R16422UnormKHR", vk::Format::eG16B16G16R16422UnormKHR},
        {"B16G16R16G16422Unorm", vk::Format::eB16G16R16G16422Unorm},
        {"B16G16R16G16422UnormKHR", vk::Format::eB16G16R16G16422UnormKHR},
        {"G16B16R163Plane420Unorm", vk::Format::eG16B16R163Plane420Unorm},
        {"G16B16R163Plane420UnormKHR", vk::Format::eG16B16R163Plane420UnormKHR},
        {"G16B16R162Plane420Unorm", vk::Format::eG16B16R162Plane420Unorm},
        {"G16B16R162Plane420UnormKHR", vk::Format::eG16B16R162Plane420UnormKHR},
        {"G16B16R163Plane422Unorm", vk::Format::eG16B16R163Plane422Unorm},
        {"G16B16R163Plane422UnormKHR", vk::Format::eG16B16R163Plane422UnormKHR},
        {"G16B16R162Plane422Unorm", vk::Format::eG16B16R162Plane422Unorm},
        {"G16B16R162Plane422UnormKHR", vk::Format::eG16B16R162Plane422UnormKHR},
        {"G16B16R163Plane444Unorm", vk::Format::eG16B16R163Plane444Unorm},
        {"G16B16R163Plane444UnormKHR", vk::Format::eG16B16R163Plane444UnormKHR},
        {"G8B8R82Plane444Unorm", vk::Format::eG8B8R82Plane444Unorm},
        {"G8B8R82Plane444UnormEXT", vk::Format::eG8B8R82Plane444UnormEXT},
        {"G10X6B10X6R10X62Plane444Unorm3Pack16", vk::Format::eG10X6B10X6R10X62Plane444Unorm3Pack16},
        {"G10X6B10X6R10X62Plane444Unorm3Pack16EXT", vk::Format::eG10X6B10X6R10X62Plane444Unorm3Pack16EXT},
        {"G12X4B12X4R12X42Plane444Unorm3Pack16", vk::Format::eG12X4B12X4R12X42Plane444Unorm3Pack16},
        {"G12X4B12X4R12X42Plane444Unorm3Pack16EXT", vk::Format::eG12X4B12X4R12X42Plane444Unorm3Pack16EXT},
        {"G16B16R162Plane444Unorm", vk::Format::eG16B16R162Plane444Unorm},
        {"G16B16R162Plane444UnormEXT", vk::Format::eG16B16R162Plane444UnormEXT},
        {"A4R4G4B4UnormPack16", vk::Format::eA4R4G4B4UnormPack16},
        {"A4R4G4B4UnormPack16EXT", vk::Format::eA4R4G4B4UnormPack16EXT},
        {"A4B4G4R4UnormPack16", vk::Format::eA4B4G4R4UnormPack16},
        {"A4B4G4R4UnormPack16EXT", vk::Format::eA4B4G4R4UnormPack16EXT},
        {"Astc4x4SfloatBlock", vk::Format::eAstc4x4SfloatBlock},
        {"Astc4x4SfloatBlockEXT", vk::Format::eAstc4x4SfloatBlockEXT},
        {"Astc5x4SfloatBlock", vk::Format::eAstc5x4SfloatBlock},
        {"Astc5x4SfloatBlockEXT", vk::Format::eAstc5x4SfloatBlockEXT},
        {"Astc5x5SfloatBlock", vk::Format::eAstc5x5SfloatBlock},
        {"Astc5x5SfloatBlockEXT", vk::Format::eAstc5x5SfloatBlockEXT},
        {"Astc6x5SfloatBlock", vk::Format::eAstc6x5SfloatBlock},
        {"Astc6x5SfloatBlockEXT", vk::Format::eAstc6x5SfloatBlockEXT},
        {"Astc6x6SfloatBlock", vk::Format::eAstc6x6SfloatBlock},
        {"Astc6x6SfloatBlockEXT", vk::Format::eAstc6x6SfloatBlockEXT},
        {"Astc8x5SfloatBlock", vk::Format::eAstc8x5SfloatBlock},
        {"Astc8x5SfloatBlockEXT", vk::Format::eAstc8x5SfloatBlockEXT},
        {"Astc8x6SfloatBlock", vk::Format::eAstc8x6SfloatBlock},
        {"Astc8x6SfloatBlockEXT", vk::Format::eAstc8x6SfloatBlockEXT},
        {"Astc8x8SfloatBlock", vk::Format::eAstc8x8SfloatBlock},
        {"Astc8x8SfloatBlockEXT", vk::Format::eAstc8x8SfloatBlockEXT},
        {"Astc10x5SfloatBlock", vk::Format::eAstc10x5SfloatBlock},
        {"Astc10x5SfloatBlockEXT", vk::Format::eAstc10x5SfloatBlockEXT},
        {"Astc10x6SfloatBlock", vk::Format::eAstc10x6SfloatBlock},
        {"Astc10x6SfloatBlockEXT", vk::Format::eAstc10x6SfloatBlockEXT},
        {"Astc10x8SfloatBlock", vk::Format::eAstc10x8SfloatBlock},
        {"Astc10x8SfloatBlockEXT", vk::Format::eAstc10x8SfloatBlockEXT},
        {"Astc10x10SfloatBlock", vk::Format::eAstc10x10SfloatBlock},
        {"Astc10x10SfloatBlockEXT", vk::Format::eAstc10x10SfloatBlockEXT},
        {"Astc12x10SfloatBlock", vk::Format::eAstc12x10SfloatBlock},
        {"Astc12x10SfloatBlockEXT", vk::Format::eAstc12x10SfloatBlockEXT},
        {"Astc12x12SfloatBlock", vk::Format::eAstc12x12SfloatBlock},
        {"Astc12x12SfloatBlockEXT", vk::Format::eAstc12x12SfloatBlockEXT},
        {"Pvrtc12BppUnormBlockIMG", vk::Format::ePvrtc12BppUnormBlockIMG},
        {"Pvrtc14BppUnormBlockIMG", vk::Format::ePvrtc14BppUnormBlockIMG},
        {"Pvrtc22BppUnormBlockIMG", vk::Format::ePvrtc22BppUnormBlockIMG},
        {"Pvrtc24BppUnormBlockIMG", vk::Format::ePvrtc24BppUnormBlockIMG},
        {"Pvrtc12BppSrgbBlockIMG", vk::Format::ePvrtc12BppSrgbBlockIMG},
        {"Pvrtc14BppSrgbBlockIMG", vk::Format::ePvrtc14BppSrgbBlockIMG},
        {"Pvrtc22BppSrgbBlockIMG", vk::Format::ePvrtc22BppSrgbBlockIMG},
        {"Pvrtc24BppSrgbBlockIMG", vk::Format::ePvrtc24BppSrgbBlockIMG},
        {"R16G16Sfixed5NV", vk::Format::eR16G16Sfixed5NV},
        {"R16G16S105NV", vk::Format::eR16G16S105NV},
        {"A1B5G5R5UnormPack16KHR", vk::Format::eA1B5G5R5UnormPack16KHR},
        {"A8UnormKHR", vk::Format::eA8UnormKHR}};

    auto it = formatMap.find(format);
    if (it != formatMap.end())
        return it->second;
    else
    {
        if (format == "SWAPCHAIN_FORMAT")
            return dynamic_cast<Vkbase::Swapchain *>(Vkbase::ResourceBase::resourceManager().resource(Vkbase::ResourceType::Swapchain, swapchainName))
                ->format();
        else if (format == "DEPTH_FORMAT")
            return depthFormat;
        else
            throw std::invalid_argument("Unknown Format: " + format);
    }
}

vk::ImageLayout JsonConfigReader::getImageLayoutWithJson(const std::string &imageLayout)
{
    static const std::unordered_map<std::string, vk::ImageLayout> imageLayoutMap = {
        {"Undefined", vk::ImageLayout::eUndefined},
        {"General", vk::ImageLayout::eGeneral},
        {"ColorAttachmentOptimal", vk::ImageLayout::eColorAttachmentOptimal},
        {"DepthStencilAttachmentOptimal", vk::ImageLayout::eDepthStencilAttachmentOptimal},
        {"DepthStencilReadOnlyOptimal", vk::ImageLayout::eDepthStencilReadOnlyOptimal},
        {"ShaderReadOnlyOptimal", vk::ImageLayout::eShaderReadOnlyOptimal},
        {"TransferSrcOptimal", vk::ImageLayout::eTransferSrcOptimal},
        {"TransferDstOptimal", vk::ImageLayout::eTransferDstOptimal},
        {"Preinitialized", vk::ImageLayout::ePreinitialized},
        {"DepthReadOnlyStencilAttachmentOptimal", vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal},
        {"DepthReadOnlyStencilAttachmentOptimalKHR", vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimalKHR},
        {"DepthAttachmentStencilReadOnlyOptimal", vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal},
        {"DepthAttachmentStencilReadOnlyOptimalKHR", vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimalKHR},
        {"DepthAttachmentOptimal", vk::ImageLayout::eDepthAttachmentOptimal},
        {"DepthAttachmentOptimalKHR", vk::ImageLayout::eDepthAttachmentOptimalKHR},
        {"DepthReadOnlyOptimal", vk::ImageLayout::eDepthReadOnlyOptimal},
        {"DepthReadOnlyOptimalKHR", vk::ImageLayout::eDepthReadOnlyOptimalKHR},
        {"StencilAttachmentOptimal", vk::ImageLayout::eStencilAttachmentOptimal},
        {"StencilAttachmentOptimalKHR", vk::ImageLayout::eStencilAttachmentOptimalKHR},
        {"StencilReadOnlyOptimal", vk::ImageLayout::eStencilReadOnlyOptimal},
        {"StencilReadOnlyOptimalKHR", vk::ImageLayout::eStencilReadOnlyOptimalKHR},
        {"ReadOnlyOptimal", vk::ImageLayout::eReadOnlyOptimal},
        {"ReadOnlyOptimalKHR", vk::ImageLayout::eReadOnlyOptimalKHR},
        {"AttachmentOptimal", vk::ImageLayout::eAttachmentOptimal},
        {"AttachmentOptimalKHR", vk::ImageLayout::eAttachmentOptimalKHR},
        {"PresentSrcKHR", vk::ImageLayout::ePresentSrcKHR},
        {"VideoDecodeDstKHR", vk::ImageLayout::eVideoDecodeDstKHR},
        {"VideoDecodeSrcKHR", vk::ImageLayout::eVideoDecodeSrcKHR},
        {"VideoDecodeDpbKHR", vk::ImageLayout::eVideoDecodeDpbKHR},
        {"SharedPresentKHR", vk::ImageLayout::eSharedPresentKHR},
        {"FragmentDensityMapOptimalEXT", vk::ImageLayout::eFragmentDensityMapOptimalEXT},
        {"FragmentShadingRateAttachmentOptimalKHR", vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR},
        {"ShadingRateOptimalNV", vk::ImageLayout::eShadingRateOptimalNV},
        {"RenderingLocalReadKHR", vk::ImageLayout::eRenderingLocalReadKHR},
        {"VideoEncodeDstKHR", vk::ImageLayout::eVideoEncodeDstKHR},
        {"VideoEncodeSrcKHR", vk::ImageLayout::eVideoEncodeSrcKHR},
        {"VideoEncodeDpbKHR", vk::ImageLayout::eVideoEncodeDpbKHR},
        {"AttachmentFeedbackLoopOptimalEXT", vk::ImageLayout::eAttachmentFeedbackLoopOptimalEXT}};

    auto it = imageLayoutMap.find(imageLayout);
    if (it != imageLayoutMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown ImageLayout: " + imageLayout);
}

vk::PipelineStageFlagBits JsonConfigReader::getPipelineStageFlagBitsWithJson(const std::string &pipelineStage)
{
    static const std::unordered_map<std::string, vk::PipelineStageFlagBits> pipelineStageMap = {
        {"TopOfPipe", vk::PipelineStageFlagBits::eTopOfPipe},
        {"DrawIndirect", vk::PipelineStageFlagBits::eDrawIndirect},
        {"VertexInput", vk::PipelineStageFlagBits::eVertexInput},
        {"VertexShader", vk::PipelineStageFlagBits::eVertexShader},
        {"TessellationControlShader", vk::PipelineStageFlagBits::eTessellationControlShader},
        {"TessellationEvaluationShader", vk::PipelineStageFlagBits::eTessellationEvaluationShader},
        {"GeometryShader", vk::PipelineStageFlagBits::eGeometryShader},
        {"FragmentShader", vk::PipelineStageFlagBits::eFragmentShader},
        {"EarlyFragmentTests", vk::PipelineStageFlagBits::eEarlyFragmentTests},
        {"LateFragmentTests", vk::PipelineStageFlagBits::eLateFragmentTests},
        {"ColorAttachmentOutput", vk::PipelineStageFlagBits::eColorAttachmentOutput},
        {"ComputeShader", vk::PipelineStageFlagBits::eComputeShader},
        {"Transfer", vk::PipelineStageFlagBits::eTransfer},
        {"BottomOfPipe", vk::PipelineStageFlagBits::eBottomOfPipe},
        {"Host", vk::PipelineStageFlagBits::eHost},
        {"AllGraphics", vk::PipelineStageFlagBits::eAllGraphics},
        {"AllCommands", vk::PipelineStageFlagBits::eAllCommands},
        {"None", vk::PipelineStageFlagBits::eNone},
        {"NoneKHR", vk::PipelineStageFlagBits::eNoneKHR},
        {"TransformFeedbackEXT", vk::PipelineStageFlagBits::eTransformFeedbackEXT},
        {"ConditionalRenderingEXT", vk::PipelineStageFlagBits::eConditionalRenderingEXT},
        {"AccelerationStructureBuildKHR", vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR},
        {"AccelerationStructureBuildNV", vk::PipelineStageFlagBits::eAccelerationStructureBuildNV},
        {"RayTracingShaderKHR", vk::PipelineStageFlagBits::eRayTracingShaderKHR},
        {"RayTracingShaderNV", vk::PipelineStageFlagBits::eRayTracingShaderNV},
        {"FragmentDensityProcessEXT", vk::PipelineStageFlagBits::eFragmentDensityProcessEXT},
        {"FragmentShadingRateAttachmentKHR", vk::PipelineStageFlagBits::eFragmentShadingRateAttachmentKHR},
        {"ShadingRateImageNV", vk::PipelineStageFlagBits::eShadingRateImageNV},
        {"CommandPreprocessNV", vk::PipelineStageFlagBits::eCommandPreprocessNV},
        {"CommandPreprocessEXT", vk::PipelineStageFlagBits::eCommandPreprocessEXT},
        {"TaskShaderEXT", vk::PipelineStageFlagBits::eTaskShaderEXT},
        {"TaskShaderNV", vk::PipelineStageFlagBits::eTaskShaderNV},
        {"MeshShaderEXT", vk::PipelineStageFlagBits::eMeshShaderEXT},
        {"MeshShaderNV", vk::PipelineStageFlagBits::eMeshShaderNV}};

    auto it = pipelineStageMap.find(pipelineStage);
    if (it != pipelineStageMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown PipelineStageFlagBit: " + pipelineStage);
}

vk::SampleCountFlagBits JsonConfigReader::getSampleCountFlagBitsWithJson(uint32_t sampleCount) { return (vk::SampleCountFlagBits)sampleCount; }

vk::AttachmentLoadOp JsonConfigReader::getAttachmentLoadOpWithJson(const std::string &loadOp)
{
    static const std::unordered_map<std::string, vk::AttachmentLoadOp> loadOpMap = {{"Load", vk::AttachmentLoadOp::eLoad},
                                                                                    {"Clear", vk::AttachmentLoadOp::eClear},
                                                                                    {"DontCare", vk::AttachmentLoadOp::eDontCare},
                                                                                    {"NoneKHR", vk::AttachmentLoadOp::eNoneKHR},
                                                                                    {"NoneEXT", vk::AttachmentLoadOp::eNoneEXT}};
    auto it = loadOpMap.find(loadOp);
    if (it != loadOpMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown AttachmentLoadOp: " + loadOp);
}

vk::AttachmentStoreOp JsonConfigReader::getAttachmentStoreOpWithJson(const std::string &loadOp)
{
    static const std::unordered_map<std::string, vk::AttachmentStoreOp> loadOpMap = {
        {"Store", vk::AttachmentStoreOp::eStore},     {"DontCare", vk::AttachmentStoreOp::eDontCare}, {"None", vk::AttachmentStoreOp::eNone},
        {"NoneEXT", vk::AttachmentStoreOp::eNoneEXT}, {"NoneKHR", vk::AttachmentStoreOp::eNoneKHR},   {"NoneQCOM", vk::AttachmentStoreOp::eNoneQCOM},
    };
    auto it = loadOpMap.find(loadOp);
    if (it != loadOpMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown AttachmentStoreOp: " + loadOp);
}

vk::AccessFlagBits JsonConfigReader::getAccessFlagBitsWithJson(const std::string &access)
{
    static const std::unordered_map<std::string, vk::AccessFlagBits> accessMap = {
        {"IndirectCommandRead", vk::AccessFlagBits::eIndirectCommandRead},
        {"IndexRead", vk::AccessFlagBits::eIndexRead},
        {"VertexAttributeRead", vk::AccessFlagBits::eVertexAttributeRead},
        {"UniformRead", vk::AccessFlagBits::eUniformRead},
        {"InputAttachmentRead", vk::AccessFlagBits::eInputAttachmentRead},
        {"ShaderRead", vk::AccessFlagBits::eShaderRead},
        {"ShaderWrite", vk::AccessFlagBits::eShaderWrite},
        {"ColorAttachmentRead", vk::AccessFlagBits::eColorAttachmentRead},
        {"ColorAttachmentWrite", vk::AccessFlagBits::eColorAttachmentWrite},
        {"DepthStencilAttachmentRead", vk::AccessFlagBits::eDepthStencilAttachmentRead},
        {"DepthStencilAttachmentWrite", vk::AccessFlagBits::eDepthStencilAttachmentWrite},
        {"TransferRead", vk::AccessFlagBits::eTransferRead},
        {"TransferWrite", vk::AccessFlagBits::eTransferWrite},
        {"HostRead", vk::AccessFlagBits::eHostRead},
        {"HostWrite", vk::AccessFlagBits::eHostWrite},
        {"MemoryRead", vk::AccessFlagBits::eMemoryRead},
        {"MemoryWrite", vk::AccessFlagBits::eMemoryWrite},
        {"None", vk::AccessFlagBits::eNone},
        {"NoneKHR", vk::AccessFlagBits::eNoneKHR},
        {"TransformFeedbackWriteEXT", vk::AccessFlagBits::eTransformFeedbackWriteEXT},
        {"TransformFeedbackCounterReadEXT", vk::AccessFlagBits::eTransformFeedbackCounterReadEXT},
        {"TransformFeedbackCounterWriteEXT", vk::AccessFlagBits::eTransformFeedbackCounterWriteEXT},
        {"ConditionalRenderingReadEXT", vk::AccessFlagBits::eConditionalRenderingReadEXT},
        {"ColorAttachmentReadNoncoherentEXT", vk::AccessFlagBits::eColorAttachmentReadNoncoherentEXT},
        {"AccelerationStructureReadKHR", vk::AccessFlagBits::eAccelerationStructureReadKHR},
        {"AccelerationStructureReadNV", vk::AccessFlagBits::eAccelerationStructureReadNV},
        {"AccelerationStructureWriteKHR", vk::AccessFlagBits::eAccelerationStructureWriteKHR},
        {"AccelerationStructureWriteNV", vk::AccessFlagBits::eAccelerationStructureWriteNV},
        {"FragmentDensityMapReadEXT", vk::AccessFlagBits::eFragmentDensityMapReadEXT},
        {"FragmentShadingRateAttachmentReadKHR", vk::AccessFlagBits::eFragmentShadingRateAttachmentReadKHR},
        {"ShadingRateImageReadNV", vk::AccessFlagBits::eShadingRateImageReadNV},
        {"CommandPreprocessReadNV", vk::AccessFlagBits::eCommandPreprocessReadNV},
        {"CommandPreprocessReadEXT", vk::AccessFlagBits::eCommandPreprocessReadEXT},
        {"CommandPreprocessWriteNV", vk::AccessFlagBits::eCommandPreprocessWriteNV},
        {"CommandPreprocessWriteEXT", vk::AccessFlagBits::eCommandPreprocessWriteEXT}};

    auto it = accessMap.find(access);
    if (it != accessMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown AccessFlagBit: " + access);
}

vk::PipelineStageFlags JsonConfigReader::getPipelineStageFlagsWithJson(const std::vector<std::string> &pipelineStageFlags)
{
    vk::PipelineStageFlags flags;
    for (const std::string &pipelineStageFlagBit : pipelineStageFlags)
        flags |= getPipelineStageFlagBitsWithJson(pipelineStageFlagBit);
    return flags;
}

vk::AccessFlags JsonConfigReader::getAccessFlagsWithJson(const std::vector<std::string> &accessFlags)
{
    vk::AccessFlags flags;
    for (const std::string &accessFlagBit : accessFlags)
        flags |= getAccessFlagBitsWithJson(accessFlagBit);
    return flags;
}

std::vector<vk::AttachmentDescription> JsonConfigReader::getAttachmentsWithJson(const json &config, const std::string &swapchainName, vk::Format depthFormat)
{
    const json &attachmentsJson = config["attachments"];

    std::vector<vk::AttachmentDescription> attachments(attachmentsJson.size());

    for (uint32_t i = 0; i < attachments.size(); ++i)
    {
        const json &attachmentJson = attachmentsJson[i];
        vk::AttachmentDescription &attachment = attachments[i];

        attachment.setFormat(getFormatWithJson(attachmentJson["format"], swapchainName, depthFormat))
            .setSamples(getSampleCountFlagBitsWithJson(attachmentJson["samples"]))
            .setLoadOp(getAttachmentLoadOpWithJson(attachmentJson["loadOp"]))
            .setStoreOp(getAttachmentStoreOpWithJson(attachmentJson["storeOp"]))
            .setStencilLoadOp(getAttachmentLoadOpWithJson(attachmentJson["stencilLoadOp"]))
            .setStencilStoreOp(getAttachmentStoreOpWithJson(attachmentJson["stencilStoreOp"]))
            .setInitialLayout(getImageLayoutWithJson(attachmentJson["initialLayout"]))
            .setFinalLayout(getImageLayoutWithJson(attachmentJson["finalLayout"]));
    }
    return attachments;
}

std::vector<std::vector<vk::AttachmentReference>> JsonConfigReader::getAttachmentRefsWithJson(const json &config)
{
    try
    {
        const json &attachmentRefSsJson = config["attachmentReferences"];

        std::vector<std::vector<vk::AttachmentReference>> attachmentRefSs(attachmentRefSsJson.size());

        for (uint32_t i = 0; i < attachmentRefSs.size(); ++i)
        {
            const json &attachmentRefsJson = attachmentRefSsJson[i];
            std::vector<vk::AttachmentReference> &attachmentRefs = attachmentRefSs[i];
            attachmentRefs.resize(attachmentRefsJson.size());

            for (uint32_t j = 0; j < attachmentRefs.size(); ++j)
            {
                const json &attachmentRefJson = attachmentRefsJson[j];

                vk::AttachmentReference &attachmentRef = attachmentRefs[j];

                attachmentRef.setAttachment(attachmentRefJson["attachment"]).setLayout(getImageLayoutWithJson(attachmentRefJson["layout"]));
            }
        }
        return attachmentRefSs;
    }
    catch (const char *msg)
    {
        throw std::runtime_error(std::string("Config Format Error: ") + msg);
    }
}

std::vector<vk::SubpassDescription> JsonConfigReader::getSubpassesWithJson(const json &config,
                                                                           const std::vector<std::vector<vk::AttachmentReference>> &attachmentRefs)
{
    const json &subpassesJson = config["subpasses"];
    std::vector<vk::SubpassDescription> subpasses(subpassesJson.size());
    for (uint32_t i = 0; i < subpasses.size(); ++i)
    {
        const json &subpassJson = subpassesJson[i];
        vk::SubpassDescription &subpass = subpasses[i];
        auto it = subpassJson.find("colorAttachments");
        if (it != subpassJson.end())
            subpass.setColorAttachments(attachmentRefs[it.value()]);

        it = subpassJson.find("inputAttachments");
        if (it != subpassJson.end())
            subpass.setInputAttachments(attachmentRefs[it.value()]);

        it = subpassJson.find("depthStencilAttachment");
        if (it != subpassJson.end())
            subpass.setPDepthStencilAttachment(&attachmentRefs[it.value()][0]);
    }
    return subpasses;
}

std::vector<vk::SubpassDependency> JsonConfigReader::getSubpassDependenciesWithJson(const json &config)
{
    const json &subpassDependenciesJson = config["subpassDependencies"];
    std::vector<vk::SubpassDependency> subpassDependencies(subpassDependenciesJson.size());

    for (uint32_t i = 0; i < subpassDependencies.size(); ++i)
    {
        const json &subpassDependencyJson = subpassDependenciesJson[i];
        vk::SubpassDependency &subpassDependency = subpassDependencies[i];
        subpassDependency.setSrcSubpass(subpassDependencyJson["srcSubpass"])
            .setDstSubpass(subpassDependencyJson["dstSubpass"])
            .setSrcStageMask(getPipelineStageFlagsWithJson(subpassDependencyJson["srcStageMask"]))
            .setSrcAccessMask(getAccessFlagsWithJson(subpassDependencyJson["srcAccessMask"]))
            .setDstStageMask(getPipelineStageFlagsWithJson(subpassDependencyJson["dstStageMask"]))
            .setDstAccessMask(getAccessFlagsWithJson(subpassDependencyJson["dstAccessMask"]));
    }

    return subpassDependencies;
}

vk::RenderPassCreateInfo JsonConfigReader::getRenderPassCreateInfo(const std::vector<vk::AttachmentDescription> &attachments,
                                                                   const std::vector<vk::SubpassDescription> &subpasses,
                                                                   const std::vector<vk::SubpassDependency> &subpassDependencies)
{
    return vk::RenderPassCreateInfo().setAttachments(attachments).setSubpasses(subpasses).setDependencies(subpassDependencies);
}

vk::ImageType JsonConfigReader::getImageTypeWithJson(const std::string &imageType)
{
    static const std::unordered_map<std::string, vk::ImageType> imageTypeMap = {
        {"1D", vk::ImageType::e1D}, {"2D", vk::ImageType::e2D}, {"3D", vk::ImageType::e3D}};

    auto it = imageTypeMap.find(imageType);
    if (it != imageTypeMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown imageType: " + imageType);
}

vk::ImageViewType JsonConfigReader::getImageViewTypeWithJson(const std::string &imageViewType)
{
    static const std::unordered_map<std::string, vk::ImageViewType> imageViewTypeMap = {{"1D", vk::ImageViewType::e1D},
                                                                                        {"2D", vk::ImageViewType::e2D},
                                                                                        {"3D", vk::ImageViewType::e3D},
                                                                                        {"Cube", vk::ImageViewType::eCube},
                                                                                        {"1DArray", vk::ImageViewType::e1DArray},
                                                                                        {"2DArray", vk::ImageViewType::e2DArray},
                                                                                        {"CubeArray", vk::ImageViewType::eCubeArray}};

    auto it = imageViewTypeMap.find(imageViewType);
    if (it != imageViewTypeMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown imageViewType: " + imageViewType);
}

vk::ImageUsageFlagBits JsonConfigReader::getImageUsageFlagBitsWithJson(const std::string &imageUsageFlagBits)
{
    static const std::unordered_map<std::string, vk::ImageUsageFlagBits> imageUsageFlagBitsMap = {
        {"TransferSrc", vk::ImageUsageFlagBits::eTransferSrc},
        {"TransferDst", vk::ImageUsageFlagBits::eTransferDst},
        {"Sampled", vk::ImageUsageFlagBits::eSampled},
        {"Storage", vk::ImageUsageFlagBits::eStorage},
        {"ColorAttachment", vk::ImageUsageFlagBits::eColorAttachment},
        {"DepthStencilAttachment", vk::ImageUsageFlagBits::eDepthStencilAttachment},
        {"TransientAttachment", vk::ImageUsageFlagBits::eTransientAttachment},
        {"InputAttachment", vk::ImageUsageFlagBits::eInputAttachment},
        {"VideoDecodeDstKHR", vk::ImageUsageFlagBits::eVideoDecodeDstKHR},
        {"VideoDecodeSrcKHR", vk::ImageUsageFlagBits::eVideoDecodeSrcKHR},
        {"VideoDecodeDpbKHR", vk::ImageUsageFlagBits::eVideoDecodeDpbKHR},
        {"FragmentDensityMapEXT", vk::ImageUsageFlagBits::eFragmentDensityMapEXT},
        {"FragmentShadingRateAttachmentKHR", vk::ImageUsageFlagBits::eFragmentShadingRateAttachmentKHR},
        {"ShadingRateImageNV", vk::ImageUsageFlagBits::eShadingRateImageNV},
        {"HostTransferEXT", vk::ImageUsageFlagBits::eHostTransferEXT},
        {"VideoEncodeDstKHR", vk::ImageUsageFlagBits::eVideoEncodeDstKHR},
        {"VideoEncodeSrcKHR", vk::ImageUsageFlagBits::eVideoEncodeSrcKHR},
        {"VideoEncodeDpbKHR", vk::ImageUsageFlagBits::eVideoEncodeDpbKHR},
        {"AttachmentFeedbackLoopEXT", vk::ImageUsageFlagBits::eAttachmentFeedbackLoopEXT},
        {"InvocationMaskHUAWEI", vk::ImageUsageFlagBits::eInvocationMaskHUAWEI},
        {"SampleWeightQCOM", vk::ImageUsageFlagBits::eSampleWeightQCOM},
        {"SampleBlockMatchQCOM", vk::ImageUsageFlagBits::eSampleBlockMatchQCOM}};

    auto it = imageUsageFlagBitsMap.find(imageUsageFlagBits);
    if (it != imageUsageFlagBitsMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown imageType: " + imageUsageFlagBits);
}

vk::ImageUsageFlags JsonConfigReader::getImageUsageFlagsWithJson(const json &imageUsageFlags)
{
    vk::ImageUsageFlags flags;
    for (const json &imageUsageFlagBits : imageUsageFlags)
        flags |= getImageUsageFlagBitsWithJson(imageUsageFlagBits);
    return flags;
}

vk::PipelineInputAssemblyStateCreateInfo JsonConfigReader::getPipelineInputAssemblyStateCreateInfoWithJson(const json &config)
{
    vk::PipelineInputAssemblyStateCreateInfo createInfo;
    static const std::unordered_map<std::string, std::function<void(vk::PipelineInputAssemblyStateCreateInfo &, const json &)>> attributeLoadMap = {
        {"primitiveRestartEnable", [](vk::PipelineInputAssemblyStateCreateInfo &createInfo, const json &config)
         { createInfo.setPrimitiveRestartEnable(JsonConfigReader::getBooleanWithJson(config)); }},
        {"topology", [](vk::PipelineInputAssemblyStateCreateInfo &createInfo, const json &config)
         { createInfo.setTopology(JsonConfigReader::getPrimitiveTopologyWithJson(config)); }}};

    for (auto it = config.begin(); it != config.end(); ++it)
    {
        if (!attributeLoadMap.count(it.key()))
        {
            std::cout << "[ Warning ] Config Loader: Accept a unknown attribute: " << it.key() << std::endl;
            continue;
        }

        attributeLoadMap.at(it.key())(createInfo, it.value());
    }
    return createInfo;
}

vk::PipelineTessellationStateCreateInfo JsonConfigReader::getPipelineTessellationStateCreateInfoWithJson(const json &config)
{
    vk::PipelineTessellationStateCreateInfo createInfo;
    static const std::unordered_map<std::string, std::function<void(vk::PipelineTessellationStateCreateInfo &, const json &)>> attributeLoadMap = {
        {"patchControlPoints", [](vk::PipelineTessellationStateCreateInfo &createInfo, const json &config) { createInfo.setPatchControlPoints(config); }}};

    for (auto it = config.begin(); it != config.end(); ++it)
    {
        if (!attributeLoadMap.count(it.key()))
        {
            std::cout << "[ Warning ] Config Loader: Accept a unknown attribute: " << it.key() << std::endl;
            continue;
        }

        attributeLoadMap.at(it.key())(createInfo, it.value());
    }
    return createInfo;
}

std::vector<vk::Rect2D> JsonConfigReader::getScissorsWithJson(const json &config)
{
    std::vector<vk::Rect2D> scissors;
    scissors.reserve(config.size());
    for (const json &scissorJson : config)
        scissors.push_back(getScissorWithJson(scissorJson));
    return scissors;
}

vk::Rect2D JsonConfigReader::getScissorWithJson(const json &config)
{
    vk::Rect2D scissor;
    static const std::unordered_map<std::string, std::function<void(vk::Rect2D &, const json &)>> attributeLoadMap = {
        {"extent", [](vk::Rect2D &scissor, const json &config) { scissor.setExtent(getExtent2DWithJson(config)); }},
        {"offset", [](vk::Rect2D &scissor, const json &config) { scissor.setOffset(getOffset2DWithJson(config)); }}};

    for (auto it = config.begin(); it != config.end(); ++it)
    {
        if (!attributeLoadMap.count(it.key()))
        {
            std::cout << "[ Warning ] Config Loader: Accept a unknown attribute: " << it.key() << std::endl;
            continue;
        }

        attributeLoadMap.at(it.key())(scissor, it.value());
    }
    return scissor;
}

vk::Extent2D JsonConfigReader::getExtent2DWithJson(const json &config) { return vk::Extent2D().setWidth(config["width"]).setHeight(config["height"]); }

vk::Offset2D JsonConfigReader::getOffset2DWithJson(const json &config) { return vk::Offset2D().setX(config["x"]).setY(config["y"]); }

std::vector<vk::Viewport> JsonConfigReader::getViewportsWithJson(const json &config)
{
    std::vector<vk::Viewport> viewports;
    viewports.reserve(config.size());

    for (const json &viewportJson : config)
        viewports.push_back(getViewportWithJson(viewportJson));

    return viewports;
}

vk::Viewport JsonConfigReader::getViewportWithJson(const json &config)
{
    vk::Viewport viewport;
    static const std::unordered_map<std::string, std::function<void(vk::Viewport &, const json &)>> attributeLoadMap = {
        {"width", [](vk::Viewport &viewport, const json &config) { viewport.setWidth(config); }},
        {"height", [](vk::Viewport &viewport, const json &config) { viewport.setHeight(config); }},
        {"x", [](vk::Viewport &viewport, const json &config) { viewport.setX(config); }},
        {"y", [](vk::Viewport &viewport, const json &config) { viewport.setY(config); }},
        {"maxDepth", [](vk::Viewport &viewport, const json &config) { viewport.setMaxDepth(config); }},
        {"minDepth", [](vk::Viewport &viewport, const json &config) { viewport.setMinDepth(config); }}};

    for (auto it = config.begin(); it != config.end(); ++it)
    {
        if (!attributeLoadMap.count(it.key()))
        {
            std::cout << "[ Warning ] Config Loader: Accept a unknown attribute: " << it.key() << std::endl;
            continue;
        }

        attributeLoadMap.at(it.key())(viewport, it.value());
    }
    return viewport;
}

vk::PipelineRasterizationStateCreateInfo JsonConfigReader::getPipelineRasterizationStateCreateInfoWithJson(const json &config)
{
    vk::PipelineRasterizationStateCreateInfo createInfo;
    static const std::unordered_map<std::string, std::function<void(vk::PipelineRasterizationStateCreateInfo &, const json &)>> attributeLoadMap = {
        {"cullMode",
         [](vk::PipelineRasterizationStateCreateInfo &createInfo, const json &config) { createInfo.setCullMode(getCullModeFlagsWithJson(config)); }},
        {"depthBiasClamp", [](vk::PipelineRasterizationStateCreateInfo &createInfo, const json &config) { createInfo.setDepthBiasClamp(config); }},
        {"depthBiasConstantFactor",
         [](vk::PipelineRasterizationStateCreateInfo &createInfo, const json &config) { createInfo.setDepthBiasConstantFactor(config); }},
        {"depthBiasEnable",
         [](vk::PipelineRasterizationStateCreateInfo &createInfo, const json &config) { createInfo.setDepthBiasEnable(getBooleanWithJson(config)); }},
        {"depthBiasSlopeFactor", [](vk::PipelineRasterizationStateCreateInfo &createInfo, const json &config) { createInfo.setDepthBiasSlopeFactor(config); }},
        {"depthClampEnable",
         [](vk::PipelineRasterizationStateCreateInfo &createInfo, const json &config) { createInfo.setDepthClampEnable(getBooleanWithJson(config)); }},
        {"frontFace", [](vk::PipelineRasterizationStateCreateInfo &createInfo, const json &config) { createInfo.setFrontFace(getFrontFaceWithJson(config)); }},
        {"lineWidth", [](vk::PipelineRasterizationStateCreateInfo &createInfo, const json &config) { createInfo.setLineWidth(config); }},
        {"polygonMode",
         [](vk::PipelineRasterizationStateCreateInfo &createInfo, const json &config) { createInfo.setPolygonMode(getPolygonModeWithJson(config)); }},
        {"rasterizerDiscardEnable",
         [](vk::PipelineRasterizationStateCreateInfo &createInfo, const json &config) { createInfo.setRasterizerDiscardEnable(getBooleanWithJson(config)); }}};

    for (auto it = config.begin(); it != config.end(); ++it)
    {
        if (!attributeLoadMap.count(it.key()))
        {
            std::cout << "[ Warning ] Config Loader: Accept a unknown attribute: " << it.key() << std::endl;
            continue;
        }

        attributeLoadMap.at(it.key())(createInfo, it.value());
    }
    return createInfo;
}

vk::PipelineMultisampleStateCreateInfo JsonConfigReader::getPipelineMultisampleStateCreateInfo(const json &config)
{

    vk::PipelineMultisampleStateCreateInfo createInfo;
    static const std::unordered_map<std::string, std::function<void(vk::PipelineMultisampleStateCreateInfo &, const json &)>> attributeLoadMap = {
        {"alphaToCoverageEnable",
         [](vk::PipelineMultisampleStateCreateInfo &createInfo, const json &config) { createInfo.setAlphaToCoverageEnable(getBooleanWithJson(config)); }},
        {"alphaToOneEnable",
         [](vk::PipelineMultisampleStateCreateInfo &createInfo, const json &config) { createInfo.setAlphaToOneEnable(getBooleanWithJson(config)); }},
        {"minSampleShading", [](vk::PipelineMultisampleStateCreateInfo &createInfo, const json &config) { createInfo.setMinSampleShading(config); }},
        {"rasterizationSamples", [](vk::PipelineMultisampleStateCreateInfo &createInfo, const json &config)
         { createInfo.setRasterizationSamples(getSampleCountFlagBitsWithJson(config)); }},
        {"sampleShadingEnable",
         [](vk::PipelineMultisampleStateCreateInfo &createInfo, const json &config) { createInfo.setSampleShadingEnable(getBooleanWithJson(config)); }}

    };

    for (auto it = config.begin(); it != config.end(); ++it)
    {
        if (!attributeLoadMap.count(it.key()))
        {
            std::cout << "[ Warning ] Config Loader: Accept a unknown attribute: " << it.key() << std::endl;
            continue;
        }

        attributeLoadMap.at(it.key())(createInfo, it.value());
    }
    return createInfo;
}

vk::CullModeFlags JsonConfigReader::getCullModeFlagsWithJson(const json &config)
{
    vk::CullModeFlags flags;
    for (const json &cullModeFlagBits : config)
        flags |= getCullModeFlagBitsWithJson(cullModeFlagBits);
    return flags;
}
vk::CullModeFlagBits JsonConfigReader::getCullModeFlagBitsWithJson(const json &config)
{
    static const std::unordered_map<std::string, vk::CullModeFlagBits> cullModeFlagBitsMap = {{"None", vk::CullModeFlagBits::eNone},
                                                                                              {"Front", vk::CullModeFlagBits::eFront},
                                                                                              {"Back", vk::CullModeFlagBits::eBack},
                                                                                              {"FrontAndBack", vk::CullModeFlagBits::eFrontAndBack}};

    auto it = cullModeFlagBitsMap.find(config);
    if (it != cullModeFlagBitsMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown cullModeFlagBits: " + std::string(config));
}

vk::FrontFace JsonConfigReader::getFrontFaceWithJson(const json &config)
{
    static const std::unordered_map<std::string, vk::FrontFace> frontFaceMap = {{"CounterClockwise", vk::FrontFace::eCounterClockwise},
                                                                                {"Clockwise", vk::FrontFace::eClockwise}};

    auto it = frontFaceMap.find(config);
    if (it != frontFaceMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown frontFace: " + std::string(config));
}

vk::Bool32 JsonConfigReader::getBooleanWithJson(const json &config)
{
    if (!config.is_boolean())
        throw std::runtime_error("Tring to convert a non-boolean value.");
    return config ? vk::True : vk::False;
}

vk::PrimitiveTopology JsonConfigReader::getPrimitiveTopologyWithJson(const json &primitiveTopology)
{
    static const std::unordered_map<std::string, vk::PrimitiveTopology> primitiveTopologyMap = {
        {"PointList", vk::PrimitiveTopology::ePointList},
        {"LineList", vk::PrimitiveTopology::eLineList},
        {"LineStrip", vk::PrimitiveTopology::eLineStrip},
        {"TriangleList", vk::PrimitiveTopology::eTriangleList},
        {"TriangleStrip", vk::PrimitiveTopology::eTriangleStrip},
        {"TriangleFan", vk::PrimitiveTopology::eTriangleFan},
        {"LineListWithAdjacency", vk::PrimitiveTopology::eLineListWithAdjacency},
        {"LineStripWithAdjacency", vk::PrimitiveTopology::eLineStripWithAdjacency},
        {"TriangleListWithAdjacency", vk::PrimitiveTopology::eTriangleListWithAdjacency},
        {"TriangleStripWithAdjacency", vk::PrimitiveTopology::eTriangleStripWithAdjacency},
        {"PatchList", vk::PrimitiveTopology::ePatchList}};

    auto it = primitiveTopologyMap.find(primitiveTopology);
    if (it != primitiveTopologyMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown primitiveTopology: " + std::string(primitiveTopology));
}

vk::PolygonMode JsonConfigReader::getPolygonModeWithJson(const json &config)
{
    static const std::unordered_map<std::string, vk::PolygonMode> polygonModeMap = {{"Fill", vk::PolygonMode::eFill},
                                                                                    {"Line", vk::PolygonMode::eLine},
                                                                                    {"Point", vk::PolygonMode::ePoint},
                                                                                    {"FillRectangleNV", vk::PolygonMode::eFillRectangleNV}};

    auto it = polygonModeMap.find(config);
    if (it != polygonModeMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown polygonMode: " + std::string(config));
}

vk::PipelineDepthStencilStateCreateInfo JsonConfigReader::getPipelineDepthStencilStateCreateInfoWithJson(const json &config)
{
    vk::PipelineDepthStencilStateCreateInfo createInfo;
    static const std::unordered_map<std::string, std::function<void(vk::PipelineDepthStencilStateCreateInfo &, const json &)>> attributeLoadMap = {
        {"back",
         [](vk::PipelineDepthStencilStateCreateInfo &createInfo, const json &config) { createInfo.setBack(getStencilOpStateWithJson(config["back"])); }},
        {"depthBoundsTestEnable",
         [](vk::PipelineDepthStencilStateCreateInfo &createInfo, const json &config) { createInfo.setDepthBoundsTestEnable(getBooleanWithJson(config)); }},
        {"depthCompareOp",
         [](vk::PipelineDepthStencilStateCreateInfo &createInfo, const json &config) { createInfo.setDepthCompareOp(getCompareOpWithJson(config)); }},
        {"depthTestEnable",
         [](vk::PipelineDepthStencilStateCreateInfo &createInfo, const json &config) { createInfo.setDepthTestEnable(getBooleanWithJson(config)); }},
        {"depthWriteEnable",
         [](vk::PipelineDepthStencilStateCreateInfo &createInfo, const json &config) { createInfo.setDepthWriteEnable(getBooleanWithJson(config)); }},
        {"flags", [](vk::PipelineDepthStencilStateCreateInfo &createInfo, const json &config)
         { createInfo.setFlags(getPipelineDepthStencilStateCreateInfoFlagsWithJson(config)); }},
        {"front", [](vk::PipelineDepthStencilStateCreateInfo &createInfo, const json &config) { createInfo.setFront(getStencilOpStateWithJson(config)); }},
        {"maxDepthBounds", [](vk::PipelineDepthStencilStateCreateInfo &createInfo, const json &config) { createInfo.setMaxDepthBounds(config); }},
        {"minDepthBounds", [](vk::PipelineDepthStencilStateCreateInfo &createInfo, const json &config) { createInfo.setMinDepthBounds(config); }},
        {"minDepthBounds",
         [](vk::PipelineDepthStencilStateCreateInfo &createInfo, const json &config) { createInfo.setStencilTestEnable(getBooleanWithJson(config)); }}};

    for (auto it = config.begin(); it != config.end(); ++it)
    {
        if (!attributeLoadMap.count(it.key()))
        {
            std::cout << "[ Warning ] Config Loader: Accept a unknown attribute: " << it.key() << std::endl;
            continue;
        }

        attributeLoadMap.at(it.key())(createInfo, it.value());
    }
    return createInfo;
}

vk::StencilOpState JsonConfigReader::getStencilOpStateWithJson(const json &config)
{
    vk::StencilOpState stencilOpState;
    static const std::unordered_map<std::string, std::function<void(vk::StencilOpState &, const json &)>> attributeLoadMap = {
        {"compareMask", [](vk::StencilOpState &stencilOpState, const json &config) { stencilOpState.setCompareMask(config); }},
        {"compareOp", [](vk::StencilOpState &stencilOpState, const json &config) { stencilOpState.setCompareOp(getCompareOpWithJson(config)); }},
        {"stencilOp", [](vk::StencilOpState &stencilOpState, const json &config) { stencilOpState.setDepthFailOp(getStencilOpWithJson(config)); }},
        {"failOp", [](vk::StencilOpState &stencilOpState, const json &config) { stencilOpState.setFailOp(getStencilOpWithJson(config)); }},
        {"passOp", [](vk::StencilOpState &stencilOpState, const json &config) { stencilOpState.setPassOp(getStencilOpWithJson(config)); }},
        {"reference", [](vk::StencilOpState &stencilOpState, const json &config) { stencilOpState.setReference(config); }},
        {"writeMask", [](vk::StencilOpState &stencilOpState, const json &config) { stencilOpState.setWriteMask(config); }}

    };

    for (auto it = config.begin(); it != config.end(); ++it)
    {
        if (!attributeLoadMap.count(it.key()))
        {
            std::cout << "[ Warning ] Config Loader: Accept a unknown attribute: " << it.key() << std::endl;
            continue;
        }

        attributeLoadMap.at(it.key())(stencilOpState, it.value());
    }
    return stencilOpState;
}

vk::CompareOp JsonConfigReader::getCompareOpWithJson(const json &config)
{
    static const std::unordered_map<std::string, vk::CompareOp> compareOpMap = {{"Never", vk::CompareOp::eNever},
                                                                                {"Less", vk::CompareOp::eLess},
                                                                                {"Equal", vk::CompareOp::eEqual},
                                                                                {"LessOrEqual", vk::CompareOp::eLessOrEqual},
                                                                                {"Greater", vk::CompareOp::eGreater},
                                                                                {"NotEqual", vk::CompareOp::eNotEqual},
                                                                                {"GreaterOrEqual", vk::CompareOp::eGreaterOrEqual},
                                                                                {"Always", vk::CompareOp::eAlways}};

    auto it = compareOpMap.find(config);
    if (it != compareOpMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown compareOp: " + std::string(config));
}

vk::StencilOp JsonConfigReader::getStencilOpWithJson(const json &config)
{
    static const std::unordered_map<std::string, vk::StencilOp> stencilOpMap = {{"Keep", vk::StencilOp::eKeep},
                                                                                {"Zero", vk::StencilOp::eZero},
                                                                                {"Replace", vk::StencilOp::eReplace},
                                                                                {"IncrementAndClamp", vk::StencilOp::eIncrementAndClamp},
                                                                                {"DecrementAndClamp", vk::StencilOp::eDecrementAndClamp},
                                                                                {"Invert", vk::StencilOp::eInvert},
                                                                                {"IncrementAndWrap", vk::StencilOp::eIncrementAndWrap},
                                                                                {"DecrementAndWrap", vk::StencilOp::eDecrementAndWrap}};

    auto it = stencilOpMap.find(config);
    if (it != stencilOpMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown stencilOp: " + std::string(config));
}

vk::PipelineDepthStencilStateCreateFlags JsonConfigReader::getPipelineDepthStencilStateCreateInfoFlagsWithJson(const json &config)
{
    vk::PipelineDepthStencilStateCreateFlags flags;
    for (const json &pipelineDepthStencilStateCreateInfoFlagBits : config)
        flags |= getPipelineDepthStencilStateCreateInfoFlagBitsWithJson(pipelineDepthStencilStateCreateInfoFlagBits);
    return flags;
}

vk::PipelineDepthStencilStateCreateFlagBits JsonConfigReader::getPipelineDepthStencilStateCreateInfoFlagBitsWithJson(const json &config)
{
    static const std::unordered_map<std::string, vk::PipelineDepthStencilStateCreateFlagBits> pipelineDepthStencilStateCreateFlagBitsMap = {
        {"RasterizationOrderAttachmentDepthAccessEXT", vk::PipelineDepthStencilStateCreateFlagBits::eRasterizationOrderAttachmentDepthAccessEXT},
        {"RasterizationOrderAttachmentDepthAccessARM", vk::PipelineDepthStencilStateCreateFlagBits::eRasterizationOrderAttachmentDepthAccessARM},
        {"RasterizationOrderAttachmentStencilAccessEXT", vk::PipelineDepthStencilStateCreateFlagBits::eRasterizationOrderAttachmentStencilAccessEXT},
        {"RasterizationOrderAttachmentStencilAccessARM", vk::PipelineDepthStencilStateCreateFlagBits::eRasterizationOrderAttachmentStencilAccessARM}};

    auto it = pipelineDepthStencilStateCreateFlagBitsMap.find(config);
    if (it != pipelineDepthStencilStateCreateFlagBitsMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown pipelineDepthStencilStateCreateFlagBits: " + std::string(config));
}

vk::PipelineColorBlendStateCreateInfo JsonConfigReader::getPipelineColorBlendStateCreateInfoWithJson(const json &config)
{
    vk::PipelineColorBlendStateCreateInfo createInfo;
    static const std::unordered_map<std::string, std::function<void(vk::PipelineColorBlendStateCreateInfo &, const json &)>> attributeLoadMap = {
        {"blendConstants", [](vk::PipelineColorBlendStateCreateInfo &createInfo, const json &config)
         { createInfo.setBlendConstants({config[0], config[1], config[2], config[3]}); }},
        {"flags", [](vk::PipelineColorBlendStateCreateInfo &createInfo, const json &config)
         { createInfo.setFlags(getPipelineColorBlendStateCreateFlagsWithJson(config)); }},
        {"logicOp", [](vk::PipelineColorBlendStateCreateInfo &createInfo, const json &config) { createInfo.setLogicOp(getLogicOp(config)); }},
        {"logicOpEnable",
         [](vk::PipelineColorBlendStateCreateInfo &createInfo, const json &config) { createInfo.setLogicOpEnable(getBooleanWithJson(config)); }},
        {"blendAttachments", [](vk::PipelineColorBlendStateCreateInfo &, const json &) {}}};

    for (auto it = config.begin(); it != config.end(); ++it)
    {
        if (!attributeLoadMap.count(it.key()))
        {
            std::cout << "[ Warning ] Config Loader: Accept a unknown attribute: " << it.key() << std::endl;
            continue;
        }

        attributeLoadMap.at(it.key())(createInfo, it.value());
    }
    return createInfo;
}

vk::PipelineColorBlendStateCreateFlags JsonConfigReader::getPipelineColorBlendStateCreateFlagsWithJson(const json &config)
{
    vk::PipelineColorBlendStateCreateFlags flags;
    for (const json &PipelineColorBlendStateCreateFlagBits : config)
        flags |= getPipelineColorBlendStateCreateFlagBitsWithJson(PipelineColorBlendStateCreateFlagBits);
    return flags;
}

vk::PipelineColorBlendStateCreateFlagBits JsonConfigReader::getPipelineColorBlendStateCreateFlagBitsWithJson(const json &config)
{
    static const std::unordered_map<std::string, vk::PipelineColorBlendStateCreateFlagBits> pipelineColorBlendStateCreateFlagBitsMap = {
        {"RasterizationOrderAttachmentAccessEXT", vk::PipelineColorBlendStateCreateFlagBits::eRasterizationOrderAttachmentAccessEXT},
        {"RasterizationOrderAttachmentAccessARM", vk::PipelineColorBlendStateCreateFlagBits::eRasterizationOrderAttachmentAccessARM}};

    auto it = pipelineColorBlendStateCreateFlagBitsMap.find(config);
    if (it != pipelineColorBlendStateCreateFlagBitsMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown pipelineColorBlendStateCreateFlagBits: " + std::string(config));
}

vk::LogicOp JsonConfigReader::getLogicOp(const json &config)
{
    static const std::unordered_map<std::string, vk::LogicOp> logicOpMap = {{"Clear", vk::LogicOp::eClear},
                                                                            {"And", vk::LogicOp::eAnd},
                                                                            {"AndReverse", vk::LogicOp::eAndReverse},
                                                                            {"Copy", vk::LogicOp::eCopy},
                                                                            {"AndInverted", vk::LogicOp::eAndInverted},
                                                                            {"NoOp", vk::LogicOp::eNoOp},
                                                                            {"Xor", vk::LogicOp::eXor},
                                                                            {"Or", vk::LogicOp::eOr},
                                                                            {"Nor", vk::LogicOp::eNor},
                                                                            {"Equivalent", vk::LogicOp::eEquivalent},
                                                                            {"Invert", vk::LogicOp::eInvert},
                                                                            {"OrReverse", vk::LogicOp::eOrReverse},
                                                                            {"CopyInverted", vk::LogicOp::eCopyInverted},
                                                                            {"OrInverted", vk::LogicOp::eOrInverted},
                                                                            {"Nand", vk::LogicOp::eNand},
                                                                            {"Set", vk::LogicOp::eSet}};

    auto it = logicOpMap.find(config);
    if (it != logicOpMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown logicOp: " + std::string(config));
}

std::vector<vk::PipelineColorBlendAttachmentState> JsonConfigReader::getPipelineColorBlendAttachmentStatesWithJson(const json &config)
{
    std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachmentStates;
    colorBlendAttachmentStates.reserve(config.size());

    for (const json &colorBlendAttachmentStateJson : config)
        colorBlendAttachmentStates.push_back(getPipelineColorBlendAttachmentStateWithJson(colorBlendAttachmentStateJson));

    return colorBlendAttachmentStates;
}

vk::PipelineColorBlendAttachmentState JsonConfigReader::getPipelineColorBlendAttachmentStateWithJson(const json &config)
{
    vk::PipelineColorBlendAttachmentState colorBlendAttachmentState;
    static const std::unordered_map<std::string, std::function<void(vk::PipelineColorBlendAttachmentState &, const json &)>> attributeLoadMap = {
        {"alphaBlendOp", [](vk::PipelineColorBlendAttachmentState &colorBlendAttachmentState, const json &config)
         { colorBlendAttachmentState.setAlphaBlendOp(getBlendOpWithJson(config)); }},
        {"blendEnable", [](vk::PipelineColorBlendAttachmentState &colorBlendAttachmentState, const json &config)
         { colorBlendAttachmentState.setBlendEnable(getBooleanWithJson(config)); }},
        {"colorBlendOp", [](vk::PipelineColorBlendAttachmentState &colorBlendAttachmentState, const json &config)
         { colorBlendAttachmentState.setColorBlendOp(getBlendOpWithJson(config)); }},
        {"colorWriteMask", [](vk::PipelineColorBlendAttachmentState &colorBlendAttachmentState, const json &config)
         { colorBlendAttachmentState.setColorWriteMask(getColorComponentFlagsWithJson(config)); }},
        {"dstAlphaBlendFactor", [](vk::PipelineColorBlendAttachmentState &colorBlendAttachmentState, const json &config)
         { colorBlendAttachmentState.setDstAlphaBlendFactor(getBlendFactorWithJson(config)); }},
        {"dstColorBlendFactor", [](vk::PipelineColorBlendAttachmentState &colorBlendAttachmentState, const json &config)
         { colorBlendAttachmentState.setDstColorBlendFactor(getBlendFactorWithJson(config)); }},
        {"srcAlphaBlendFactor", [](vk::PipelineColorBlendAttachmentState &colorBlendAttachmentState, const json &config)
         { colorBlendAttachmentState.setSrcAlphaBlendFactor(getBlendFactorWithJson(config)); }},
        {"srcColorBlendFactor", [](vk::PipelineColorBlendAttachmentState &colorBlendAttachmentState, const json &config)
         { colorBlendAttachmentState.setSrcColorBlendFactor(getBlendFactorWithJson(config)); }}};

    for (auto it = config.begin(); it != config.end(); ++it)
    {
        if (!attributeLoadMap.count(it.key()))
        {
            std::cout << "[ Warning ] Config Loader: Accept a unknown attribute: " << it.key() << std::endl;
            continue;
        }

        attributeLoadMap.at(it.key())(colorBlendAttachmentState, it.value());
    }
    return colorBlendAttachmentState;
}

vk::BlendOp JsonConfigReader::getBlendOpWithJson(const json &config)
{
    static const std::unordered_map<std::string, vk::BlendOp> blendOpMap = {{"Add", vk::BlendOp::eAdd},
                                                                            {"Subtract", vk::BlendOp::eSubtract},
                                                                            {"ReverseSubtract", vk::BlendOp::eReverseSubtract},
                                                                            {"Min", vk::BlendOp::eMin},
                                                                            {"Max", vk::BlendOp::eMax},
                                                                            {"ZeroEXT", vk::BlendOp::eZeroEXT},
                                                                            {"SrcEXT", vk::BlendOp::eSrcEXT},
                                                                            {"DstEXT", vk::BlendOp::eDstEXT},
                                                                            {"SrcOverEXT", vk::BlendOp::eSrcOverEXT},
                                                                            {"DstOverEXT", vk::BlendOp::eDstOverEXT},
                                                                            {"SrcInEXT", vk::BlendOp::eSrcInEXT},
                                                                            {"DstInEXT", vk::BlendOp::eDstInEXT},
                                                                            {"SrcOutEXT", vk::BlendOp::eSrcOutEXT},
                                                                            {"DstOutEXT", vk::BlendOp::eDstOutEXT},
                                                                            {"SrcAtopEXT", vk::BlendOp::eSrcAtopEXT},
                                                                            {"DstAtopEXT", vk::BlendOp::eDstAtopEXT},
                                                                            {"XorEXT", vk::BlendOp::eXorEXT},
                                                                            {"MultiplyEXT", vk::BlendOp::eMultiplyEXT},
                                                                            {"ScreenEXT", vk::BlendOp::eScreenEXT},
                                                                            {"OverlayEXT", vk::BlendOp::eOverlayEXT},
                                                                            {"DarkenEXT", vk::BlendOp::eDarkenEXT},
                                                                            {"LightenEXT", vk::BlendOp::eLightenEXT},
                                                                            {"ColordodgeEXT", vk::BlendOp::eColordodgeEXT},
                                                                            {"ColorburnEXT", vk::BlendOp::eColorburnEXT},
                                                                            {"HardlightEXT", vk::BlendOp::eHardlightEXT},
                                                                            {"SoftlightEXT", vk::BlendOp::eSoftlightEXT},
                                                                            {"DifferenceEXT", vk::BlendOp::eDifferenceEXT},
                                                                            {"ExclusionEXT", vk::BlendOp::eExclusionEXT},
                                                                            {"InvertEXT", vk::BlendOp::eInvertEXT},
                                                                            {"InvertRgbEXT", vk::BlendOp::eInvertRgbEXT},
                                                                            {"LineardodgeEXT", vk::BlendOp::eLineardodgeEXT},
                                                                            {"LinearburnEXT", vk::BlendOp::eLinearburnEXT},
                                                                            {"VividlightEXT", vk::BlendOp::eVividlightEXT},
                                                                            {"LinearlightEXT", vk::BlendOp::eLinearlightEXT},
                                                                            {"PinlightEXT", vk::BlendOp::ePinlightEXT},
                                                                            {"HardmixEXT", vk::BlendOp::eHardmixEXT},
                                                                            {"HslHueEXT", vk::BlendOp::eHslHueEXT},
                                                                            {"HslSaturationEXT", vk::BlendOp::eHslSaturationEXT},
                                                                            {"HslColorEXT", vk::BlendOp::eHslColorEXT},
                                                                            {"HslLuminosityEXT", vk::BlendOp::eHslLuminosityEXT},
                                                                            {"PlusEXT", vk::BlendOp::ePlusEXT},
                                                                            {"PlusClampedEXT", vk::BlendOp::ePlusClampedEXT},
                                                                            {"PlusClampedAlphaEXT", vk::BlendOp::ePlusClampedAlphaEXT},
                                                                            {"PlusDarkerEXT", vk::BlendOp::ePlusDarkerEXT},
                                                                            {"MinusEXT", vk::BlendOp::eMinusEXT},
                                                                            {"MinusClampedEXT", vk::BlendOp::eMinusClampedEXT},
                                                                            {"ContrastEXT", vk::BlendOp::eContrastEXT},
                                                                            {"InvertOvgEXT", vk::BlendOp::eInvertOvgEXT},
                                                                            {"RedEXT", vk::BlendOp::eRedEXT},
                                                                            {"GreenEXT", vk::BlendOp::eGreenEXT},
                                                                            {"BlueEXT", vk::BlendOp::eBlueEXT}};

    auto it = blendOpMap.find(config);
    if (it != blendOpMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown blendOp: " + std::string(config));
}

vk::ColorComponentFlags JsonConfigReader::getColorComponentFlagsWithJson(const json &config)
{
    vk::ColorComponentFlags flags;
    for (const json &colorComponentFlagBits : config)
        flags |= getColorComponentFlagBitsWithJson(colorComponentFlagBits);
    return flags;
}

vk::ColorComponentFlagBits JsonConfigReader::getColorComponentFlagBitsWithJson(const json &config)
{
    static const std::unordered_map<std::string, vk::ColorComponentFlagBits> colorComponentFlagBitsMap = {{"R", vk::ColorComponentFlagBits::eR},
                                                                                                          {"G", vk::ColorComponentFlagBits::eG},
                                                                                                          {"B", vk::ColorComponentFlagBits::eB},
                                                                                                          {"A", vk::ColorComponentFlagBits::eA}};

    auto it = colorComponentFlagBitsMap.find(config);
    if (it != colorComponentFlagBitsMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown colorComponentFlagBits: " + std::string(config));
}

vk::BlendFactor JsonConfigReader::getBlendFactorWithJson(const json &config)
{
    static const std::unordered_map<std::string, vk::BlendFactor> blendFactorMap = {{"Zero", vk::BlendFactor::eZero},
                                                                                    {"One", vk::BlendFactor::eOne},
                                                                                    {"SrcColor", vk::BlendFactor::eSrcColor},
                                                                                    {"OneMinusSrcColor", vk::BlendFactor::eOneMinusSrcColor},
                                                                                    {"DstColor", vk::BlendFactor::eDstColor},
                                                                                    {"OneMinusDstColor", vk::BlendFactor::eOneMinusDstColor},
                                                                                    {"SrcAlpha", vk::BlendFactor::eSrcAlpha},
                                                                                    {"OneMinusSrcAlpha", vk::BlendFactor::eOneMinusSrcAlpha},
                                                                                    {"DstAlpha", vk::BlendFactor::eDstAlpha},
                                                                                    {"OneMinusDstAlpha", vk::BlendFactor::eOneMinusDstAlpha},
                                                                                    {"ConstantColor", vk::BlendFactor::eConstantColor},
                                                                                    {"OneMinusConstantColor", vk::BlendFactor::eOneMinusConstantColor},
                                                                                    {"ConstantAlpha", vk::BlendFactor::eConstantAlpha},
                                                                                    {"OneMinusConstantAlpha", vk::BlendFactor::eOneMinusConstantAlpha},
                                                                                    {"SrcAlphaSaturate", vk::BlendFactor::eSrcAlphaSaturate},
                                                                                    {"Src1Color", vk::BlendFactor::eSrc1Color},
                                                                                    {"OneMinusSrc1Color", vk::BlendFactor::eOneMinusSrc1Color},
                                                                                    {"Src1Alpha", vk::BlendFactor::eSrc1Alpha},
                                                                                    {"OneMinusSrc1Alpha", vk::BlendFactor::eOneMinusSrc1Alpha}};

    auto it = blendFactorMap.find(config);
    if (it != blendFactorMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown blendFactor: " + std::string(config));
}

std::vector<vk::DynamicState> JsonConfigReader::getDynamicStatusWithJson(const json &config)
{
    std::vector<vk::DynamicState> dynamicStatus;
    dynamicStatus.reserve(config.size());

    for (const json &dynamicStateJson : config)
        dynamicStatus.push_back(getDynamicStateWithJson(dynamicStateJson));
    return dynamicStatus;
}

vk::DynamicState JsonConfigReader::getDynamicStateWithJson(const json &config)
{
    static const std::unordered_map<std::string, vk::DynamicState> dynamicStateMap = {
        {"Viewport", vk::DynamicState::eViewport},
        {"Scissor", vk::DynamicState::eScissor},
        {"LineWidth", vk::DynamicState::eLineWidth},
        {"DepthBias", vk::DynamicState::eDepthBias},
        {"BlendConstants", vk::DynamicState::eBlendConstants},
        {"DepthBounds", vk::DynamicState::eDepthBounds},
        {"StencilCompareMask", vk::DynamicState::eStencilCompareMask},
        {"StencilWriteMask", vk::DynamicState::eStencilWriteMask},
        {"StencilReference", vk::DynamicState::eStencilReference},
        {"CullMode", vk::DynamicState::eCullMode},
        {"CullModeEXT", vk::DynamicState::eCullModeEXT},
        {"FrontFace", vk::DynamicState::eFrontFace},
        {"FrontFaceEXT", vk::DynamicState::eFrontFaceEXT},
        {"PrimitiveTopology", vk::DynamicState::ePrimitiveTopology},
        {"PrimitiveTopologyEXT", vk::DynamicState::ePrimitiveTopologyEXT},
        {"ViewportWithCount", vk::DynamicState::eViewportWithCount},
        {"ViewportWithCountEXT", vk::DynamicState::eViewportWithCountEXT},
        {"ScissorWithCount", vk::DynamicState::eScissorWithCount},
        {"ScissorWithCountEXT", vk::DynamicState::eScissorWithCountEXT},
        {"VertexInputBindingStride", vk::DynamicState::eVertexInputBindingStride},
        {"VertexInputBindingStrideEXT", vk::DynamicState::eVertexInputBindingStrideEXT},
        {"DepthTestEnable", vk::DynamicState::eDepthTestEnable},
        {"DepthTestEnableEXT", vk::DynamicState::eDepthTestEnableEXT},
        {"DepthWriteEnable", vk::DynamicState::eDepthWriteEnable},
        {"DepthWriteEnableEXT", vk::DynamicState::eDepthWriteEnableEXT},
        {"DepthCompareOp", vk::DynamicState::eDepthCompareOp},
        {"DepthCompareOpEXT", vk::DynamicState::eDepthCompareOpEXT},
        {"DepthBoundsTestEnable", vk::DynamicState::eDepthBoundsTestEnable},
        {"DepthBoundsTestEnableEXT", vk::DynamicState::eDepthBoundsTestEnableEXT},
        {"StencilTestEnable", vk::DynamicState::eStencilTestEnable},
        {"StencilTestEnableEXT", vk::DynamicState::eStencilTestEnableEXT},
        {"StencilOp", vk::DynamicState::eStencilOp},
        {"StencilOpEXT", vk::DynamicState::eStencilOpEXT},
        {"RasterizerDiscardEnable", vk::DynamicState::eRasterizerDiscardEnable},
        {"RasterizerDiscardEnableEXT", vk::DynamicState::eRasterizerDiscardEnableEXT},
        {"DepthBiasEnable", vk::DynamicState::eDepthBiasEnable},
        {"DepthBiasEnableEXT", vk::DynamicState::eDepthBiasEnableEXT},
        {"PrimitiveRestartEnable", vk::DynamicState::ePrimitiveRestartEnable},
        {"PrimitiveRestartEnableEXT", vk::DynamicState::ePrimitiveRestartEnableEXT},
        {"ViewportWScalingNV", vk::DynamicState::eViewportWScalingNV},
        {"DiscardRectangleEXT", vk::DynamicState::eDiscardRectangleEXT},
        {"DiscardRectangleEnableEXT", vk::DynamicState::eDiscardRectangleEnableEXT},
        {"DiscardRectangleModeEXT", vk::DynamicState::eDiscardRectangleModeEXT},
        {"SampleLocationsEXT", vk::DynamicState::eSampleLocationsEXT},
        {"RayTracingPipelineStackSizeKHR", vk::DynamicState::eRayTracingPipelineStackSizeKHR},
        {"ViewportShadingRatePaletteNV", vk::DynamicState::eViewportShadingRatePaletteNV},
        {"ViewportCoarseSampleOrderNV", vk::DynamicState::eViewportCoarseSampleOrderNV},
        {"ExclusiveScissorEnableNV", vk::DynamicState::eExclusiveScissorEnableNV},
        {"ExclusiveScissorNV", vk::DynamicState::eExclusiveScissorNV},
        {"FragmentShadingRateKHR", vk::DynamicState::eFragmentShadingRateKHR},
        {"VertexInputEXT", vk::DynamicState::eVertexInputEXT},
        {"PatchControlPointsEXT", vk::DynamicState::ePatchControlPointsEXT},
        {"LogicOpEXT", vk::DynamicState::eLogicOpEXT},
        {"ColorWriteEnableEXT", vk::DynamicState::eColorWriteEnableEXT},
        {"DepthClampEnableEXT", vk::DynamicState::eDepthClampEnableEXT},
        {"PolygonModeEXT", vk::DynamicState::ePolygonModeEXT},
        {"RasterizationSamplesEXT", vk::DynamicState::eRasterizationSamplesEXT},
        {"SampleMaskEXT", vk::DynamicState::eSampleMaskEXT},
        {"AlphaToCoverageEnableEXT", vk::DynamicState::eAlphaToCoverageEnableEXT},
        {"AlphaToOneEnableEXT", vk::DynamicState::eAlphaToOneEnableEXT},
        {"LogicOpEnableEXT", vk::DynamicState::eLogicOpEnableEXT},
        {"ColorBlendEnableEXT", vk::DynamicState::eColorBlendEnableEXT},
        {"ColorBlendEquationEXT", vk::DynamicState::eColorBlendEquationEXT},
        {"ColorWriteMaskEXT", vk::DynamicState::eColorWriteMaskEXT},
        {"TessellationDomainOriginEXT", vk::DynamicState::eTessellationDomainOriginEXT},
        {"RasterizationStreamEXT", vk::DynamicState::eRasterizationStreamEXT},
        {"ConservativeRasterizationModeEXT", vk::DynamicState::eConservativeRasterizationModeEXT},
        {"ExtraPrimitiveOverestimationSizeEXT", vk::DynamicState::eExtraPrimitiveOverestimationSizeEXT},
        {"DepthClipEnableEXT", vk::DynamicState::eDepthClipEnableEXT},
        {"SampleLocationsEnableEXT", vk::DynamicState::eSampleLocationsEnableEXT},
        {"ColorBlendAdvancedEXT", vk::DynamicState::eColorBlendAdvancedEXT},
        {"ProvokingVertexModeEXT", vk::DynamicState::eProvokingVertexModeEXT},
        {"LineRasterizationModeEXT", vk::DynamicState::eLineRasterizationModeEXT},
        {"LineStippleEnableEXT", vk::DynamicState::eLineStippleEnableEXT},
        {"DepthClipNegativeOneToOneEXT", vk::DynamicState::eDepthClipNegativeOneToOneEXT},
        {"ViewportWScalingEnableNV", vk::DynamicState::eViewportWScalingEnableNV},
        {"ViewportSwizzleNV", vk::DynamicState::eViewportSwizzleNV},
        {"CoverageToColorEnableNV", vk::DynamicState::eCoverageToColorEnableNV},
        {"CoverageToColorLocationNV", vk::DynamicState::eCoverageToColorLocationNV},
        {"CoverageModulationModeNV", vk::DynamicState::eCoverageModulationModeNV},
        {"CoverageModulationTableEnableNV", vk::DynamicState::eCoverageModulationTableEnableNV},
        {"CoverageModulationTableNV", vk::DynamicState::eCoverageModulationTableNV},
        {"ShadingRateImageEnableNV", vk::DynamicState::eShadingRateImageEnableNV},
        {"RepresentativeFragmentTestEnableNV", vk::DynamicState::eRepresentativeFragmentTestEnableNV},
        {"CoverageReductionModeNV", vk::DynamicState::eCoverageReductionModeNV},
        {"AttachmentFeedbackLoopEnableEXT", vk::DynamicState::eAttachmentFeedbackLoopEnableEXT},
        {"LineStippleKHR", vk::DynamicState::eLineStippleKHR},
        {"LineStippleEXT", vk::DynamicState::eLineStippleEXT},
        {"DepthClampRangeEXT", vk::DynamicState::eDepthClampRangeEXT}};

    auto it = dynamicStateMap.find(config);
    if (it != dynamicStateMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown dynamicState: " + std::string(config));
}

std::vector<std::pair<vk::DescriptorType, vk::ShaderStageFlags>> JsonConfigReader::getDescriptorTypeShaderStageWithJson(const json &config)
{
    std::vector<std::pair<vk::DescriptorType, vk::ShaderStageFlags>> descriptorTypeShaderStages;
    descriptorTypeShaderStages.reserve(config.size());
    for (const json &descriptorTypeShaderStageJson : config)
        descriptorTypeShaderStages.push_back({getDescriptorTypeWithJson(descriptorTypeShaderStageJson["descriptorType"]),
                                              getShaderStageFlagsWithJson(descriptorTypeShaderStageJson["shaderStageFlags"])});
    return descriptorTypeShaderStages;
}

vk::ShaderStageFlags JsonConfigReader::getShaderStageFlagsWithJson(const json &config)
{
    vk::ShaderStageFlags flags;
    for (const json &shaderStageFlagBits : config)
        flags |= getShaderStageFlagBitsWithJson(shaderStageFlagBits);
    return flags;
}

vk::DescriptorType JsonConfigReader::getDescriptorTypeWithJson(const json &config)
{
    static const std::unordered_map<std::string, vk::DescriptorType> descriptorTypeMap = {
        {"Sampler", vk::DescriptorType::eSampler},
        {"CombinedImageSampler", vk::DescriptorType::eCombinedImageSampler},
        {"SampledImage", vk::DescriptorType::eSampledImage},
        {"StorageImage", vk::DescriptorType::eStorageImage},
        {"UniformTexelBuffer", vk::DescriptorType::eUniformTexelBuffer},
        {"StorageTexelBuffer", vk::DescriptorType::eStorageTexelBuffer},
        {"UniformBuffer", vk::DescriptorType::eUniformBuffer},
        {"StorageBuffer", vk::DescriptorType::eStorageBuffer},
        {"UniformBufferDynamic", vk::DescriptorType::eUniformBufferDynamic},
        {"StorageBufferDynamic", vk::DescriptorType::eStorageBufferDynamic},
        {"InputAttachment", vk::DescriptorType::eInputAttachment},
        {"InlineUniformBlock", vk::DescriptorType::eInlineUniformBlock},
        {"InlineUniformBlockEXT", vk::DescriptorType::eInlineUniformBlockEXT},
        {"AccelerationStructureKHR", vk::DescriptorType::eAccelerationStructureKHR},
        {"AccelerationStructureNV", vk::DescriptorType::eAccelerationStructureNV},
        {"SampleWeightImageQCOM", vk::DescriptorType::eSampleWeightImageQCOM},
        {"BlockMatchImageQCOM", vk::DescriptorType::eBlockMatchImageQCOM},
        {"MutableEXT", vk::DescriptorType::eMutableEXT},
        {"MutableVALVE", vk::DescriptorType::eMutableVALVE}};

    auto it = descriptorTypeMap.find(config);
    if (it != descriptorTypeMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown descriptorType: " + std::string(config));
}

vk::ShaderStageFlagBits JsonConfigReader::getShaderStageFlagBitsWithJson(const json &config)
{
    static const std::unordered_map<std::string, vk::ShaderStageFlagBits> shaderStageFlagBitsMap = {
        {"Vertex", vk::ShaderStageFlagBits::eVertex},
        {"TessellationControl", vk::ShaderStageFlagBits::eTessellationControl},
        {"TessellationEvaluation", vk::ShaderStageFlagBits::eTessellationEvaluation},
        {"Geometry", vk::ShaderStageFlagBits::eGeometry},
        {"Fragment", vk::ShaderStageFlagBits::eFragment},
        {"Compute", vk::ShaderStageFlagBits::eCompute},
        {"AllGraphics", vk::ShaderStageFlagBits::eAllGraphics},
        {"All", vk::ShaderStageFlagBits::eAll},
        {"RaygenKHR", vk::ShaderStageFlagBits::eRaygenKHR},
        {"RaygenNV", vk::ShaderStageFlagBits::eRaygenNV},
        {"AnyHitKHR", vk::ShaderStageFlagBits::eAnyHitKHR},
        {"AnyHitNV", vk::ShaderStageFlagBits::eAnyHitNV},
        {"ClosestHitKHR", vk::ShaderStageFlagBits::eClosestHitKHR},
        {"ClosestHitNV", vk::ShaderStageFlagBits::eClosestHitNV},
        {"MissKHR", vk::ShaderStageFlagBits::eMissKHR},
        {"MissNV", vk::ShaderStageFlagBits::eMissNV},
        {"IntersectionKHR", vk::ShaderStageFlagBits::eIntersectionKHR},
        {"IntersectionNV", vk::ShaderStageFlagBits::eIntersectionNV},
        {"CallableKHR", vk::ShaderStageFlagBits::eCallableKHR},
        {"CallableNV", vk::ShaderStageFlagBits::eCallableNV},
        {"TaskEXT", vk::ShaderStageFlagBits::eTaskEXT},
        {"TaskNV", vk::ShaderStageFlagBits::eTaskNV},
        {"MeshEXT", vk::ShaderStageFlagBits::eMeshEXT},
        {"MeshNV", vk::ShaderStageFlagBits::eMeshNV},
        {"SubpassShadingHUAWEI", vk::ShaderStageFlagBits::eSubpassShadingHUAWEI},
        {"ClusterCullingHUAWEI", vk::ShaderStageFlagBits::eClusterCullingHUAWEI}};

    auto it = shaderStageFlagBitsMap.find(config);
    if (it != shaderStageFlagBitsMap.end())
        return it->second;
    else
        throw std::invalid_argument("Unknown shaderStageFlagBits: " + std::string(config));
}

bool JsonConfigReader::textInList(const std::string &text, const std::vector<std::string> &list)
{
    return std::find(list.begin(), list.end(), text) != list.end();
}

json JsonConfigReader::load(const std::string &filename)
{
    json config;
    std::ifstream configFile(filename);
    if (!configFile.is_open())
        throw std::runtime_error("Cannot open config file");
    configFile >> config;
    return config;
}