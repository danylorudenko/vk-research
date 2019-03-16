#pragma once

#include <string>
#include <cstdint>
#include <limits>

namespace Render
{

using ResourceKey = std::string;
using RenderPassKey = std::string;
using SetLayoutKey = std::string;
using PipelineKey = std::string;
using ShaderKey = std::string;

struct UniformBufferHandle
{
    std::uint64_t id_;
};

}