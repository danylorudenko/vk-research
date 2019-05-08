#pragma once

#include <vector>
#include <cstdint>

namespace Data
{

enum ModelVertexContentFlags : std::uint32_t
{
    MODEL_VERTEX_CONTENT_POSITION = 1,
    MODEL_VERTEX_CONTENT_NORMAL = 1 << 1,
    MODEL_VERTEX_CONTENT_TANGENT = 1 << 2,
    MODEL_VERTEX_CONTENT_BITANGENT = 1 << 3,
    MODEL_VERTEX_CONTENT_UV = 1 << 4
};

struct ModelMesh
{
    std::vector<std::uint8_t> vertexData_;
    std::vector<std::uint8_t> indexData_;

    std::uint32_t vertexContentFlags_;
};

}