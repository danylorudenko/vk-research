#include "TransformationSystem.hpp"

#include "TansformComponent.hpp"
#include <utility>
#include <glm\gtc\matrix_transform.hpp>

namespace Transform
{

TransformationSystem::TransformationSystem() = default;

TransformationSystem::TransformationSystem(TransformationSystem&& rhs)
{
    operator=(std::move(rhs));
}

TransformationSystem& TransformationSystem::operator=(TransformationSystem&& rhs)
{
    std::swap(components_, rhs.components_);

    return *this;
}

TransformationSystem::~TransformationSystem() = default;

void TransformationSystem::Update(glm::vec3 const& cameraPos, glm::vec3 const& cameraEuler, float cameraFowDegrees)
{
    glm::mat4 matrix;

    std::uint64_t const componentsCount = static_cast<std::uint64_t>(components_.size());
    for (std::uint64_t i = 0; i < componentsCount; ++i) {
        TransformComponent* component = components_[i];
        
        
    }
}

TransformComponent* TransformationSystem::CreateTransformComponent(TransformComponent* parent, Render::UniformBufferWriterProxy* uniformProxy)
{
    TransformComponent* component = new TransformComponent;
    component->parent_ = parent;
    component->position_ = glm::vec3(0.0f, 0.0f, 0.0f);
    component->orientation_ = glm::vec3(0.0f, 0.0f, 1.0f);
    component->scale_ = glm::vec3(1.0f, 1.0f, 1.0f);
    if (uniformProxy != nullptr)
        component->uniformProxy_ = *uniformProxy;

    components_.emplace_back(component);

    return component;
}

}