#include "internal/engine/plugin/gscript_object_1_0.h"

GScriptObject_1_0::GScriptObject_1_0(IGScriptSpace* boundedScriptSpace, const std::string& name, GNFScriptClassConstructor scriptCtor, GNFScriptClassDestructor scriptDtor)
{
	m_scriptName = name;
	m_scriptSpace = boundedScriptSpace;
	m_version.version_minor = 0;
	m_version.version_major = 1;

	m_scriptCtor = scriptCtor;
	m_scriptDtor = scriptDtor;

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
}

IGScript* GScriptObject_1_0::create_script()
{
	auto ptr =  ((*m_scriptCtor)(nullptr));
	return (IGScript*)ptr;
}

void GScriptObject_1_0::destroy_script(IGScript* script)
{
	(m_scriptDtor)((pGNFScriptObject)script);
}
