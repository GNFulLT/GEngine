#include "internal/engine/scene/component/transform_component.h"

TransformComponent::TransformComponent()
{
}

glm::mat4& TransformComponent::get_global_transform() noexcept
{
	return m_globalTransform;
}

const glm::mat4& TransformComponent::get_global_transform() const noexcept
{
	return m_globalTransform;
}

glm::mat4& TransformComponent::get_local_transform() noexcept
{
	return m_localTransform;
}

const glm::mat4& TransformComponent::get_local_transform() const noexcept
{
	return m_localTransform;
}
