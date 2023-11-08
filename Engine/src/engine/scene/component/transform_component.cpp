#include "internal/engine/scene/component/transform_component.h"
#include "engine/scene/serializable_component.h"
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include "engine/rendering/scene/scene.h"

TransformComponent::TransformComponent(Scene* scene, uint32_t nodeID)
{

	p_scene = scene;
	m_nodeID = nodeID;
}

glm::mat4& TransformComponent::get_global_transform() noexcept
{
	return m_globalTransform;
}

const glm::mat4& TransformComponent::get_global_transform() const noexcept
{
	return m_globalTransform;
}

void TransformComponent::set_global_transform(const glm::mat4& tr) noexcept
{
	m_globalTransform = tr;
}

glm::mat4 TransformComponent::get_local_transform() noexcept
{
	auto transform = glm::mat4(1.f);
	transform = glm::translate(transform,m_position);
	transform = glm::scale(transform, m_scale);
	transform = transform * glm::toMat4(m_rotation);
	return transform;
}

void TransformComponent::set_local_transform(const glm::mat4& tr) noexcept
{
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(tr, m_scale, m_rotation, m_position, skew, perspective);
}

const glm::vec3& TransformComponent::scale_getter()
{
	return m_scale;
}

void TransformComponent::scale_setter(const glm::vec3& scale)
{
	m_scale = scale;
	p_scene->mark_as_changed(m_nodeID);
}

const glm::vec3& TransformComponent::position_getter()
{
	return m_position;
}

void TransformComponent::position_setter(const glm::vec3& getter)
{
	m_position = getter;
	p_scene->mark_as_changed(m_nodeID);
}
