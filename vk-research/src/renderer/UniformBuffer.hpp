#pragma once

#include <vk_interface\ProxyHandles.hpp>

namespace Render
{

struct UniformBuffer
{
    VKW::ProxyBufferHandle proxyBufferViewHandle_;
};

}