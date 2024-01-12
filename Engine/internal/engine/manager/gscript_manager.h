#ifndef GSCRIPT_MANAGER_H
#define GSCRIPT_MANAGER_H

#include "engine/manager/igscript_manager.h"
#include "engine/plugin/igscript_space.h"
#include <unordered_map>
#include <unordered_set>

class GScriptManager : public IGScriptManager
{
public:
	//X TODO NEEDS TO BE THREAD SAFE 
	GScriptManager();
	~GScriptManager();
	std::expected<IGScriptSpace*, GSCRIPT_SPACE_LOAD_ERROR> load_script_space(std::filesystem::path path) override;

	bool register_script(const GNFScriptRegisterArgs* args) override;
	bool register_script_space(const GNFScriptSpaceRegisterArgs* args) override;
	
	bool unregister_script(const std::string& path) override;

	const std::vector<IGScriptSpace*>* get_loaded_script_spaces() const noexcept override;
private:
	std::unordered_map<std::string, IGScriptSpace*> m_scriptSpaceMap;
	std::unordered_map<std::string, IGScriptSpace*> m_scriptSpaceDllMap;

	std::vector<IGScriptSpace*> m_scriptSpaces;
	std::unordered_map<std::string,void*> m_libMaps;
	IGScriptSpace* m_currentStartedSpace;
	const char* m_currentDllPath = nullptr;
};

#endif // GSCRIPT_MANAGER_H