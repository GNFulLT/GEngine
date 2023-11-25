#ifndef SCRIPT_COMPONENT_H
#define SCRIPT_COMPONENT_H

#include "engine/scene/component/igscript.h"
#include "gobject/gobject_defs.h"
#include <cstdint>
#include "engine/scene/gentity.h"

class Scene;

class ScriptComponent
{
public:
	ScriptComponent(Scene* scene, uint32_t nodeID,GEntity* entity,IGScript* script);
	ScriptComponent(const ScriptComponent&) = delete;
	ScriptComponent& operator=(const ScriptComponent&) = delete;


	IGScript* get_script();
private:
	uint32_t m_boundedNodeID;
	Scene* m_boundedScene;
	IGScript* m_script;
	GEntity* m_entity;
};

#endif // SCRIPT_COMPONENT_H