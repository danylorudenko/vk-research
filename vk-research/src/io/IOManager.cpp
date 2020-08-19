#include "IOManager.hpp"

#include <memory\ByteBuffer.hpp>

#include <fstream>
#include <iostream>
#include <utility>

#define STB_IMAGE_IMPLEMENTATION
#include <stb\stb_image.h>

IOManager::IOManager() {}

IOManager::IOManager(IOManager&& rhs) {}

IOManager& IOManager::operator=(IOManager&& rhs)
{
    return *this;
}

IOManager::~IOManager() {}

std::uint64_t IOManager::ReadFileToBuffer(char const* path, ByteBuffer& buffer)
{
    std::ifstream istream{ path, std::ios_base::binary | std::ios_base::beg };
    if (!istream) {
        std::cerr << "Error opening file in path: " << path << std::endl;
        return 0;
    }

    auto const fileSize = istream.seekg(0, std::ios_base::end).tellg();
    if (!istream) {
        std::cerr << "Error measuring file size: " << path << std::endl;
        return 0;
    }

    if (buffer.Size() < static_cast<std::uint64_t>(fileSize)) {
        buffer.Resize(fileSize);
    }

    istream.seekg(0, std::ios_base::beg);
    istream.read(buffer.As<char*>(), static_cast<std::uint64_t>(fileSize));
    istream.close();

    return fileSize;
}

struct ModelHeader
{
    std::uint32_t vertexCount_ = 0;
    std::uint32_t vertexSize_ = 0;
    std::uint32_t indexCount_ = 0;
    std::uint32_t indexSize_ = 0;
    std::uint32_t vertexContentFlags_ = 0;
};

Data::ModelMesh IOManager::ReadModelMesh(char const* path)
{
    std::ifstream istream{ path, std::ios_base::binary | std::ios_base::beg };
    if (!istream) {
        std::cerr << "Error opening ModelMesh in path: " << path << std::endl;
        return Data::ModelMesh{};
    }

    auto const fileSize = istream.seekg(0, std::ios_base::end).tellg();
    if (!istream) {
        std::cerr << "Error measuring file size: " << path << std::endl;
        return Data::ModelMesh();
    }


    ModelHeader modelHeader{};

    istream.seekg(0, std::ios_base::beg);
    istream.read(reinterpret_cast<char*>(&modelHeader), sizeof(modelHeader));

    Data::ModelMesh model;

    std::uint32_t const vertexDataSizeBytes = modelHeader.vertexCount_ * modelHeader.vertexSize_;
    std::uint32_t const indexDataSizeBytes = modelHeader.indexCount_ * modelHeader.indexSize_;

    model.vertexData_.resize(vertexDataSizeBytes);
    model.indexData_.resize(indexDataSizeBytes);
    model.vertexContentFlags_ = modelHeader.vertexContentFlags_;

    istream.read(reinterpret_cast<char*>(model.vertexData_.data()), vertexDataSizeBytes);
    istream.read(reinterpret_cast<char*>(model.indexData_.data()), indexDataSizeBytes);

    istream.close();

    return std::move(model);
}

Data::Texture2D IOManager::ReadTexture2D(char const* path, Data::TextureChannelVariations channelVariations)
{
    Data::Texture2D texture;

    int desiredChannels = 0;
    switch (channelVariations)
    {
    case Data::TEXTURE_VARIATION_GRAY:
        desiredChannels = STBI_grey;
        break;
    case Data::TEXTURE_VARIATION_GRAY_ALPHA:
        desiredChannels = STBI_grey_alpha;
        break;
    case Data::TEXTURE_VARIATION_RGB:
        desiredChannels = STBI_rgb;
        break;
    case Data::TEXTURE_VARIATION_RGBA:
        desiredChannels = STBI_rgb_alpha;
        break;
    default:
        assert(false && "Invalid TextureChannelVariations");
    }

    int x, y, n;
    stbi_uc* textureData = stbi_load(path, &x, &y, &n, desiredChannels);
    if (textureData == NULL) {
        texture.width_ = 0;
        texture.height_ = 0;
        texture.textureChannelVariations_ = Data::TEXTURE_VARIATION_INVALID;
        return std::move(texture);
    }

    std::uint32_t const dataSize = x * y * desiredChannels;
    texture.textureData_.resize(dataSize);
    std::memcpy(texture.textureData_.data(), textureData, dataSize);

    texture.textureChannelVariations_ = channelVariations;
    texture.width_ = x;
    texture.height_ = y;

    stbi_image_free(textureData);

    return std::move(texture);
}