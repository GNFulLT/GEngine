#include "internal/engine/plugin/gscript_space_1_0.h"
#include "internal/engine/plugin/gscript_object_1_0.h"

GScriptSpace_1_0::GScriptSpace_1_0(const std::string& spaceName)
{
	m_scriptSpaceName = spaceName;
	m_version.version_minor = 0;
	m_version.version_major = 1;
}

const char* GScriptSpace_1_0::get_script_space_name()
{
	return m_scriptSpaceName.c_str();
}

const GNFPluginVersion* GScriptSpace_1_0::get_plugin_version()
{
	return &m_version;
}

std::vector<IGScriptObject*>* GScriptSpace_1_0::get_loaded_script_objects()
{
	return &m_loadedScriptObjects;
}

IGScriptObject* GScriptSpace_1_0::get_loaded_script_object_by_name(const char* name)
{
	if (auto iter = m_scriptObjectMap.find(name); iter != m_scriptObjectMap.end())
	{
		return iter->second;
	}
	return nullptr;
}

IGScriptObject* GScriptSpace_1_0::create_script_object(const std::string& name, GNFScriptClassConstructor scriptCtor, GNFScriptClassDestructor scriptDtor)
{
	if (auto iter = m_scriptObjectMap.find(name); iter != m_scriptObjectMap.end())
		return iter->second;

	auto obj = new GScriptObject_1_0(this, name, scriptCtor, scriptDtor);

	m_scriptObjectMap.emplace(name, obj);
	m_loadedScriptObjects.push_back(obj);
	return obj;
}
