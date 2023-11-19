#include "internal/engine/scene/component/script_component.h"

ScriptComponent::ScriptComponent(Scene* scene, uint32_t nodeID, IGScript* script)
{
	m_boundedNodeID = nodeID;
	m_boundedScene = scene;
	m_script = script;
}

IGScript* ScriptComponent::get_script()
{
	return m_script;
}
