#ifndef TRANSFORM_COMPONENT_H
#define TRANSFORM_COMPONENT_H

#include <glm/glm.hpp>


class TransformComponent
{
public:
	TransformComponent();


	glm::mat4& get_global_transform() noexcept;
	const glm::mat4& get_global_transform() const noexcept;

	glm::mat4& get_local_transform() noexcept;
	const glm::mat4& get_local_transform() const noexcept;
private:
	glm::mat4 m_localTransform = glm::mat4(1.f);
	glm::mat4 m_globalTransform = glm::mat4(1.f);
};
#endif // TRANSFORM_COMPONENT_H