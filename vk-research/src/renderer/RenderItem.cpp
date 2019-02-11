#include "RenderItem.hpp"
#include "Root.hpp"

#include <cstring>
#include <utility>

namespace Render
{

RenderItemUniform::RenderItemUniform()
    : hostBufferSize_{ 0 }
    , hostBuffer_{ nullptr }
{
    name_[0] = '\0';
}

RenderItemUniform::RenderItemUniform(Root* root, char const* name, std::uint32_t dataSize, std::uint8_t const* dataSource)
    : hostBufferSize_{ 0 }
    , hostBuffer_{ nullptr }
{
    std::strcpy(name_, name);

    hostBuffer_ = reinterpret_cast<std::uint8_t*>(malloc(dataSize));
    hostBufferSize_ = dataSize;

    serverBufferHandle_ = root->AcquireUniformBuffer(dataSize);
}

RenderItemUniform::RenderItemUniform(RenderItemUniform&& rhs)
    : RenderItemUniform{}
{
    operator=(std::move(rhs));
}

RenderItemUniform& RenderItemUniform::operator=(RenderItemUniform&& rhs)
{
    std::strcpy(name_, rhs.name_);
    std::swap(hostBufferSize_, rhs.hostBufferSize_);
    std::swap(hostBuffer_, rhs.hostBuffer_);
    std::swap(serverBufferHandle_, rhs.serverBufferHandle_);
}

RenderItemUniform::~RenderItemUniform()
{
    name_[0] = '\0';
    free(hostBuffer_);
}


}