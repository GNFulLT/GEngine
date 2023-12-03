#ifndef GSCRIPT_SPACE_1_0_H
#define GSCRIPT_SPACE_1_0_H

#include "engine/plugin/igscript_space.h"
#include <string>
#include <vector>
#include <unordered_map>
#include "engine/plugin/gplugin.h"

class GScriptSpace_1_0 : public IGScriptSpace
{
public:
	GScriptSpace_1_0(const std::string& spaceName);
	// Inherited via IGScriptSpace
	virtual const char* get_script_space_name() override;
	virtual const GNFPluginVersion* get_plugin_version() override;
	virtual std::vector<IGScriptObject*>* get_loaded_script_objects() override;
	virtual IGScriptObject* get_loaded_script_object_by_name(const char* name) override;

	IGScriptObject* create_script_object(const std::string& name,GNFScriptClassConstructor scriptCtor, GNFScriptClassDestructor scriptDtor);
private:
	GNFScriptClassConstructor m_scriptCtor;
	GNFScriptClassDestructor m_scriptDtor;
	GNFPluginVersion m_version;
	std::string m_scriptSpaceName;
	std::vector<IGScriptObject*> m_loadedScriptObjects;
	std::unordered_map<std::string, IGScriptObject*> m_scriptObjectMap;
	
};

#endif // GSCRIPT_SPACE_1_0_H