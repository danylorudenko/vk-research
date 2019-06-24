#pragma once

#include <vector>
#include <cstdint>

namespace Data
{

enum TextureChannelVariations : std::uint32_t
{
    TEXTURE_VARIATION_INVALID,
    TEXTURE_VARIATION_GRAY,
    TEXTURE_VARIATION_GRAY_ALPHA,
    TEXTURE_VARIATION_RGB,
    TEXTURE_VARIATION_RGBA
};

struct Texture2D
{
    std::vector<std::uint8_t> textureData_;
    TextureChannelVariations textureChannelVariations_;
    std::uint32_t width_;
    std::uint32_t height_;
};

}