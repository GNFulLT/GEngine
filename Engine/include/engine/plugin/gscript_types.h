#ifndef GSCRIPT_TYPES_H
#define GSCRIPT_TYPES_H

#include "engine/plugin/gapi_general_types.h"

typedef void* pGNFScriptObject;
typedef const char* pGNFScriptName;
typedef const char* pGNFScriptNamespace;

typedef struct GNFScriptSpaceRegisterArgs
{
	pGNFScriptNamespace scriptNamespace;
	GNFPluginVersion pluginVersion;
};

typedef bool(*GNFScriptSpaceRegister)(const GNFScriptSpaceRegisterArgs*);

typedef pGNFScriptObject(*GNFScriptClassConstructor)(pGObject*);
typedef void(*GNFScriptClassDestructor)(pGNFScriptObject);

typedef struct GNFScriptRegisterArgs
{
	pGNFScriptName scriptName;
	GNFPluginVersion pluginVersion;
	GNFScriptClassConstructor scriptCtor;
	GNFScriptClassDestructor scriptDtor;
};

typedef bool(*GNFScriptRegister)(const GNFScriptRegisterArgs*);

typedef struct GNFScriptRegistration
{
	GNFScriptSpaceRegister scriptSpaceRegister;
	GNFScriptRegister scriptRegister;
};

typedef void(GSCRIPT_REGISTRATION_FUNC_TYPE)(const GNFScriptRegistration* registration);
#define GSCRIPT_REGISTRATION_FUNC_NAME "GENGINE_SCRIPT_REGISTRATION"
#define GSCRIPT_REGISTRATION void GENGINE_SCRIPT_REGISTRATION(const GNFScriptRegistration* registration)

#endif // GSCRIPT_TYPES_H