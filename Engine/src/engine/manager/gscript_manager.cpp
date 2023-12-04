#include "internal/engine/manager/gscript_manager.h"
#include <dylib.hpp>
#include "engine/gengine.h"
#include "engine/imanager_table.h"
#include "internal/engine/plugin/gscript_space_1_0.h"


bool register_scripti(const GNFScriptRegisterArgs* args)
{
	auto scriptManager = ((GSharedPtr<IGScriptManager>*)GEngine::get_instance()->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_SCRIPT))->get();
	return scriptManager->register_script(args);
}

bool register_script_spacei(const GNFScriptSpaceRegisterArgs* args)
{
	auto scriptManager = ((GSharedPtr<IGScriptManager>*)GEngine::get_instance()->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_SCRIPT))->get();
	return scriptManager->register_script_space(args);
	
}
GScriptManager::GScriptManager()
{

}

GScriptManager::~GScriptManager()
{
	for (int i = 0; i < m_scriptSpaces.size();i++)
	{
		delete m_scriptSpaces[i];
	}
	for (auto lib : m_libs)
	{
		auto ptr = (dylib*)lib;
		delete ptr;
	}
}

const std::vector<IGScriptSpace*>* GScriptManager::get_loaded_script_spaces() const noexcept
{
	return &m_scriptSpaces;	
}

std::expected<IGScriptSpace*, GSCRIPT_SPACE_LOAD_ERROR> GScriptManager::load_script_space(std::filesystem::path path)
{
	auto pathAsStr = path.filename().string();
	for (auto space : m_scriptSpaces)
	{
		if (strcmp(space->get_dll_name(), pathAsStr.c_str()) == 0)
			return std::unexpected(GSCRIPT_SPACE_LOAD_ERROR_ALREADY_LOADED);
	}
	dylib* lib;
	try
	{
		lib = new dylib(path);
		auto symbolHas = lib->has_symbol(GSCRIPT_REGISTRATION_FUNC_NAME);
		if (!symbolHas)
		{
			delete lib;
			return std::unexpected(GSCRIPT_SPACE_LOAD_ERROR_DLL_ERROR);
		}
		m_currentDllPath = pathAsStr.c_str();
		std::string symbolName = GSCRIPT_REGISTRATION_FUNC_NAME;
		auto registerFunc = lib->get_function<GSCRIPT_REGISTRATION_FUNC_TYPE>(symbolName);
		GNFScriptRegistration registration;
		registration.scriptRegister = &register_scripti;
		registration.scriptSpaceRegister = &register_script_spacei;

		auto prevSpace = m_currentStartedSpace;
		(registerFunc)(&registration);
		auto currSpace = m_currentStartedSpace;

		if (currSpace != prevSpace)
		{
			m_libs.push_back((void*)lib);
			return m_currentStartedSpace;
		}
		return std::unexpected(GSCRIPT_SPACE_LOAD_ERROR_ALREADY_LOADED);
	}
	catch (const std::exception& ex)
	{
		//X LOG
			return std::unexpected(GSCRIPT_SPACE_LOAD_ERROR_DLL_ERROR);
	}
}

bool GScriptManager::register_script(const GNFScriptRegisterArgs* args)
{
	auto vers = m_currentStartedSpace->get_plugin_version();
	if (vers->version_major == 1 && vers->version_minor == 0)
	{
		auto plugin = (GScriptSpace_1_0*)m_currentStartedSpace;
		auto scriptName = std::string(args->scriptName);
		auto obj = plugin->create_script_object(scriptName, args->scriptCtor, args->scriptDtor);
		if (obj != nullptr)
			return true;
	}
	return false;
}

bool GScriptManager::register_script_space(const GNFScriptSpaceRegisterArgs* args)
{
	auto namesSpace = args->scriptNamespace;
	auto vers = args->pluginVersion;
	if (auto mapIter = m_scriptSpaceMap.find(namesSpace); mapIter != m_scriptSpaceMap.end())
	{
		return false;
	}
	if (vers.version_major == 1 && vers.version_minor == 0)
	{
		m_currentStartedSpace = new GScriptSpace_1_0(namesSpace,m_currentDllPath);
		m_scriptSpaceMap.emplace(namesSpace, m_currentStartedSpace);
		m_scriptSpaces.push_back(m_currentStartedSpace);
		return true;
	}
	return false;
}
