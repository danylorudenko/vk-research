#pragma once

#include <string>
#include <cstdint>

namespace Render
{

using ResourceKey = std::string;
using PassKey = std::string;
using SetLayoutKey = std::string;
using PipelineKey = std::string;
using ShaderKey = std::string;
using MaterialTemplateKey = std::string;
using MaterialKey = std::string;

struct UniformBufferHandle
{
    std::uint64_t id_;
};

}