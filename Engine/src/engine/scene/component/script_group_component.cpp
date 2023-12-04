#include "engine/scene/component/script_group_component.h"
#include "engine/scene/component/igscript.h"

ScriptGroupComponent::ScriptGroupComponent(GEntity* boundedEntity)
{
	m_boundedEntity = boundedEntity;
}

ScriptGroupComponent::~ScriptGroupComponent()
{
	for (auto pair : m_scriptMap)
	{
		auto scriptObj = (IGScriptObject*)pair.first;
		scriptObj->destroy_script(pair.second);
	}
}

void ScriptGroupComponent::update(float dt)
{
	for (auto script : m_scripts)
	{
		script->update(dt);
	}
}

bool ScriptGroupComponent::try_to_register_script(IGScriptObject* obj)
{
	if (auto iter = m_scriptMap.find(std::size_t(obj)); iter !=  m_scriptMap.end())
		return false;
	auto createdScript = obj->create_script();
	if (createdScript == nullptr)
		return false;
	m_scriptMap.emplace(std::size_t(obj), createdScript);
	m_scripts.push_back(createdScript);
	createdScript->m_entityID = m_boundedEntity;
	return true;
}
