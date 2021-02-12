#pragma once

#include <glm\glm.hpp>

namespace Transform
{

struct TransformComponent
{
    TransformComponent* parent_;

    glm::vec3 position_;
    glm::vec3 orientation_;
    glm::vec3 scale_;

    //Render::UniformBufferWriterProxy uniformProxy_;
};

}