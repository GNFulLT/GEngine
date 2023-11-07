#ifndef TRANSFORM_COMPONENT_H
#define TRANSFORM_COMPONENT_H

#include <glm/glm.hpp>
#include "gobject/gobject_defs.h"

class TransformComponent
{
public:
	TransformComponent();


	glm::mat4& get_global_transform() noexcept;
	const glm::mat4& get_global_transform() const noexcept;

	glm::mat4& get_local_transform() noexcept;
	const glm::mat4& get_local_transform() const noexcept;

	GPROPERTY(NAME=localTransform)
	glm::mat4 m_localTransform = glm::mat4(1.f);

	GPROPERTY(NAME=globalTransform)
	glm::mat4 m_globalTransform = glm::mat4(1.f);
};
#endif // TRANSFORM_COMPONENT_H