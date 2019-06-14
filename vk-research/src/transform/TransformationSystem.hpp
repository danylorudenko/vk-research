#pragma once

#include <cstdint>
#include <vector>
#include <glm\vec3.hpp>
#include "..\class_features\NonCopyable.hpp"

namespace Render
{

class UniformBufferWriterProxy;

}

namespace Transform
{

struct TransformComponent;


struct TransformSystemCameraData
{
    glm::vec3 cameraPos; 
    glm::vec3 cameraEuler;
    float cameraFowDegrees;
    float width;
    float height;
};

class TransformationSystem
    : public NonCopyable
{
public:
    TransformationSystem();
    TransformationSystem(TransformationSystem&& rhs);
    TransformationSystem& operator=(TransformationSystem&& rhs);

    ~TransformationSystem();

    TransformComponent* CreateTransformComponent(TransformComponent* parent, Render::UniformBufferWriterProxy* uniformProxy);
    void Update(std::uint32_t context, TransformSystemCameraData& cameraData);


private:
    std::vector<TransformComponent*> components_;
};

}