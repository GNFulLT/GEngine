#ifndef SCRIPT_GROUP_COMPONENT_H
#define SCRIPT_GROUP_COMPONENT_H

#include "gobject/gobject_defs.h"
#include "engine/igobject.h"
#include <vector>
#include <string>
#include <unordered_map>
#include "engine/plugin/igscript_object.h"
#include "engine/GEngine_EXPORT.h"

class IGScript;
class GEntity;

class ENGINE_API ScriptGroupComponent : public IGObject
{
	GOBJECT_DEF(ScriptGroupComponent, IGObject)
public:
	ScriptGroupComponent(GEntity* boundedEntity);
	virtual ~ScriptGroupComponent();

	void update(float dt);

	bool try_to_register_script(IGScriptObject* obj);
private:
	GEntity* m_boundedEntity;
	std::unordered_map<std::size_t, IGScript*> m_scriptMap;
	std::vector<IGScript*> m_scripts;
};

#endif // SCRIPT_GROUP_COMPONENT_H