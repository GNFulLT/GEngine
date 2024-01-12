#include "internal/engine/plugin/gscript_instance_1_0.h"
#include "engine/scene/component/igscript.h"
GScriptInstance_1_0::GScriptInstance_1_0(GScriptObject_1_0* owner, GNFScriptClassConstructor scriptCtor, GNFScriptClassDestructor scriptDtor)
{
	m_owner = owner;
	m_script = (IGScript*)(scriptCtor)(nullptr);
	if(m_script != nullptr)
		isValid = true;
	m_scriptDtor = scriptDtor;
}

GScriptInstance_1_0::~GScriptInstance_1_0()
{
	destroy_internal();
}

bool GScriptInstance_1_0::is_valid() const noexcept
{
	return isValid;
}

void GScriptInstance_1_0::update(float dt)
{
	if (m_script != nullptr)
		m_script->update(dt);
	else
	{
		//X TODO : LOG HERE
	}
}

void GScriptInstance_1_0::destroy_internal()
{
	if(m_script != nullptr)
		m_scriptDtor(m_script);
	m_script = nullptr;
	isValid = false;
}

void GScriptInstance_1_0::set_id(GEntity* entity)
{
	if(m_script != nullptr)
		set_script_id(m_script,entity);
}

IGScriptObject* GScriptInstance_1_0::get_script_object() const noexcept
{
	return m_owner;
}

void IGScriptInstance::set_script_id(IGScript* script, GEntity* entity)
{
	script->m_entityID = entity;
}
