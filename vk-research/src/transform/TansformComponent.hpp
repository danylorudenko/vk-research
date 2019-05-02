#pragma once

#include <glm\glm.hpp>
#include "..\renderer\UniformBufferWriterProxy.hpp"

namespace Transform
{

struct TransformComponent
{
    TransformComponent* parent_;

    glm::vec3 position_;
    glm::vec3 orientation_;
    glm::vec3 scale_;

    Render::UniformBufferWriterProxy uniformProxy_;
};

}