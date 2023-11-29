#ifndef GPROJECT_INFO_H
#define GPROJECT_INFO_H

#include "engine/gproject.h"
#include "internal/project/gscript_info.h"
#include <unordered_map>

class GProjectInfo
{
public:
	GProjectInfo(GProject* project);
	~GProjectInfo();
	bool add_script(std::filesystem::path scriptPath,std::string className);
private:
	GProject* m_boundedProject;
	std::unordered_map<std::filesystem::path, GScriptInfo*> m_scriptInfoMap;
};

#endif // GPROJECT_INFO_H