#ifndef IGSCRIPT_OBJECT_H
#define IGSCRIPT_OBJECT_H

#include "engine/plugin/gapi_general_types.h"

class IGScriptSpace;
class IGScript;

class IGScriptObject
{
public:
	virtual ~IGScriptObject() = default;

	virtual const IGScriptSpace* get_bounded_script_space() = 0;
	
	virtual const char* get_script_name() = 0;

	virtual const GNFPluginVersion* get_plugin_version() = 0;
	
	virtual void destroy() = 0;

	virtual IGScript* create_script() = 0;

	virtual void destroy_script(IGScript* script) = 0;
private: 
};

#endif // IGSCRIPT_OBJECT_H