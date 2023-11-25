#include "internal/engine/scene/component/script_component.h"

ScriptComponent::ScriptComponent(Scene* scene, uint32_t nodeID,entt::entity entity, IGScript* script)
{
	m_boundedNodeID = nodeID;
	m_boundedScene = scene;
	m_script = script;
	m_entity = entity;

	m_script->m_entityID = entity;
}

IGScript* ScriptComponent::get_script()
{
	return m_script;
}
