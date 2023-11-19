#ifndef SCRIPT_COMPONENT_H
#define SCRIPT_COMPONENT_H

#include "engine/scene/components/igscript.h"
#include "gobject/gobject_defs.h"
#include <cstdint>

class Scene;

class ScriptComponent
{
public:
	ScriptComponent(Scene* scene, uint32_t nodeID,IGScript* script);
	ScriptComponent(const ScriptComponent&) = delete;
	ScriptComponent& operator=(const ScriptComponent&) = delete;


	IGScript* get_script();
private:
	uint32_t m_boundedNodeID;
	Scene* m_boundedScene;
	IGScript* m_script;
};

#endif // SCRIPT_COMPONENT_H