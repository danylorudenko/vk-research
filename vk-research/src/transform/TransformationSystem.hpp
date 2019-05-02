#pragma once

#include <vector>
#include <glm\fwd.hpp>
#include "..\class_features\NonCopyable.hpp"

namespace Render
{

class UniformBufferWriterProxy;

}

namespace Transform
{

struct TransformComponent;


class TransformationSystem
    : public NonCopyable
{
public:
    TransformationSystem();
    TransformationSystem(TransformationSystem&& rhs);
    TransformationSystem& operator=(TransformationSystem&& rhs);

    ~TransformationSystem();

    void Update(glm::vec3 const& cameraPos, glm::vec3 const& cameraEuler, float cameraFowDegrees);

    TransformComponent* CreateTransformComponent(TransformComponent* parent, Render::UniformBufferWriterProxy* uniformProxy);

private:
    std::vector<TransformComponent*> components_;
};

}