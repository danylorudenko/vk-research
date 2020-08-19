#pragma once

#include <class_features\NonCopyable.hpp>
#include <data\ModelMesh.hpp>
#include <data\Texture2D.hpp>

class ByteBuffer;

class IOManager
    : public NonCopyable
{
public:
    IOManager();
    IOManager(IOManager&& rhs);
    IOManager& operator=(IOManager&& rhs);

    std::uint64_t ReadFileToBuffer(char const* path, ByteBuffer& buffer);

    Data::ModelMesh ReadModelMesh(char const* path);
    Data::Texture2D ReadTexture2D(char const* path, Data::TextureChannelVariations channels);
    

    ~IOManager();

private:

};