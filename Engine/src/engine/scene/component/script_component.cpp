#include "internal/engine/scene/component/script_component.h"

ScriptComponent::ScriptComponent(Scene* scene, uint32_t nodeID, GEntity* entity, IGScript* script)
{
	m_boundedNodeID = nodeID;
	m_boundedScene = scene;
	m_script = script;
	m_entity = entity;
}

IGScript* ScriptComponent::get_script()
{
	return m_script;
}
