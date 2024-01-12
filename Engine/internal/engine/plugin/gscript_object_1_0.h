#ifndef GSCRIPT_OBJECT_1_0_H
#define GSCRIPT_OBJECT_1_0_H

#include "engine/plugin/igscript_object.h"
#include <string>
#include "engine/plugin/gplugin.h"
#include <vector>

class GScriptObject_1_0 : public IGScriptObject
{
public:
	GScriptObject_1_0(IGScriptSpace* boundedScriptSpace,const std::string& name, GNFScriptClassConstructor scriptCtor, GNFScriptClassDestructor scriptDtor);
	~GScriptObject_1_0();
	virtual const IGScriptSpace* get_bounded_script_space() override;
	virtual const char* get_script_name() override;
	virtual const GNFPluginVersion* get_plugin_version() override;

	virtual void destroy() override;


	virtual IGScriptInstance* create_script() override;

private:
	IGScriptSpace* m_scriptSpace;
	GNFPluginVersion m_version;
	std::string m_scriptName;

	GNFScriptClassConstructor m_scriptCtor;
	GNFScriptClassDestructor m_scriptDtor;

	std::vector<IGScriptInstance*> m_createdScriptInstances;

};

#endif // GSCRIPT_OBJECT_1_0_H