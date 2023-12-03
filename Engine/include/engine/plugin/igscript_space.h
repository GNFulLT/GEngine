#ifndef IGSCRIPT_SPACE_H
#define IGSCRIPT_SPACE_H

#include "engine/plugin/gapi_general_types.h"
#include "engine/plugin/igscript_object.h"

#include <vector>

class IGScriptSpace
{
public:
	virtual ~IGScriptSpace() = default;

	virtual const char* get_script_space_name() = 0;

	virtual const GNFPluginVersion* get_plugin_version() = 0;

	virtual std::vector<IGScriptObject*>* get_loaded_script_objects() = 0;

	virtual IGScriptObject* get_loaded_script_object_by_name(const char* name) = 0;
private:
};

#endif // IGSCRIPT_SPACE_H