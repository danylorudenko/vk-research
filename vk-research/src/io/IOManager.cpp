#include "IOManager.hpp"
#include "..\memory\ByteBuffer.hpp"

#include <fstream>
#include <iostream>
#include <utility>

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