#ifndef GSCRIPT_INFO_H
#define GSCRIPT_INFO_H

#include "internal/project/gproject_info.h"
#include <filesystem>

class GScriptInfo
{
public:
	GScriptInfo(GProjectInfo* projectInfo, std::filesystem::path scriptPath,std::string className);
private:
	GProjectInfo* m_boundedProjectInfo;
	std::filesystem::path m_scriptPath;
	std::string m_className;

};

#endif // GSCRIPT_INFO_H