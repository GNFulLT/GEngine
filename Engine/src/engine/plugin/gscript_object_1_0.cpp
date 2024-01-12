#include "internal/engine/plugin/gscript_object_1_0.h"
#include "internal/engine/plugin/gscript_instance_1_0.h"

GScriptObject_1_0::GScriptObject_1_0(IGScriptSpace* boundedScriptSpace, const std::string& name, GNFScriptClassConstructor scriptCtor, GNFScriptClassDestructor scriptDtor)
{
	m_scriptName = name;
	m_scriptSpace = boundedScriptSpace;
	m_version.version_minor = 0;
	m_version.version_major = 1;

	m_scriptCtor = scriptCtor;
	m_scriptDtor = scriptDtor;

}

GScriptObject_1_0::~GScriptObject_1_0()
{
	destroy();
}

const IGScriptSpace* GScriptObject_1_0::get_bounded_script_space()
{
	return m_scriptSpace;
}

const char* GScriptObject_1_0::get_script_name()
{
	return m_scriptName.c_str();
}

const GNFPluginVersion* GScriptObject_1_0::get_plugin_version()
{
	return &m_version;
}

void GScriptObject_1_0::destroy()
{
	for (int i = 0; i < m_createdScriptInstances.size(); i++)
	{
		((GScriptInstance_1_0*)m_createdScriptInstances[i])->destroy_internal();
	}
}

IGScriptInstance* GScriptObject_1_0::create_script()
{
	auto instance = new GScriptInstance_1_0(this, m_scriptCtor, m_scriptDtor);
	if (!instance->is_valid())
	{
		delete instance;
		return nullptr;
	}
	m_createdScriptInstances.push_back(instance);
	return instance;
}

