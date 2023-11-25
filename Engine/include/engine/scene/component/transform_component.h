#ifndef TRANSFORM_COMPONENT_H
#define TRANSFORM_COMPONENT_H

#include <glm/glm.hpp>
#include "gobject/gobject_defs.h"
#include "engine/scene/serializable_component.h"
#include <glm/ext.hpp>
#include "engine/manager/igscene_manager.h"

class Scene;

class TransformComponent : public SerializableComponent<TransformComponent>
{
public:
	TransformComponent(Scene* scene,uint32_t nodeID);
	TransformComponent(const TransformComponent&) = delete;
	TransformComponent& operator=(const TransformComponent&) = delete;

	glm::mat4& get_global_transform() noexcept;
	const glm::mat4& get_global_transform() const noexcept;
	void set_global_transform(const glm::mat4&) noexcept;
	
	glm::mat4 get_local_transform() noexcept;
	//const glm::mat4& get_local_transform() const noexcept;

	void set_local_transform(const glm::mat4& tr) noexcept;

	const glm::vec3& scale_getter();
	void scale_setter(const glm::vec3& scale);

	const glm::vec3& position_getter();

	void position_setter(const glm::vec3& getter);

	glm::mat4 m_globalTransform = glm::mat4(1.f);

	GPROPERTY(NAME = position, GETTER = position_getter, SETTER = position_setter)
	glm::vec3 m_position = glm::vec3(0.f, 0.f, 0.f);

	GPROPERTY(NAME=scale,GETTER=scale_getter,SETTER=scale_setter)
	glm::vec3 m_scale = glm::vec3(1.f, 1.f, 1.f);

	GPROPERTY(NAME = rotation)
	glm::quat m_rotation = glm::quat(0.f,0.f,0.f,0.f);

	
private:
	Scene* p_scene;
	uint32_t m_nodeID;
};
#endif // TRANSFORM_COMPONENT_H