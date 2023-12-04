#ifndef IGSCRIPT_MANAGER_H
#define IGSCRIPT_MANAGER_H

#include <filesystem>
#include "engine/plugin/gplugin.h"
#include <vector>
#include <expected>

class IGScriptSpace;

enum GSCRIPT_SPACE_LOAD_ERROR
{
	GSCRIPT_SPACE_LOAD_ERROR_DLL_ERROR,
	GSCRIPT_SPACE_LOAD_ERROR_ALREADY_LOADED
};

class IGScriptManager
{
public:
	virtual std::expected<IGScriptSpace*, GSCRIPT_SPACE_LOAD_ERROR> load_script_space(std::filesystem::path path) = 0;
	virtual bool register_script(const GNFScriptRegisterArgs* args) = 0;
	virtual bool register_script_space(const GNFScriptSpaceRegisterArgs* args) = 0;
	virtual const std::vector<IGScriptSpace*>* get_loaded_script_spaces() const noexcept = 0;
private:
};



#endif // IGSCRIPT_MANAGER_H