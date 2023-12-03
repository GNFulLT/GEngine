#ifndef GSCRIPT_INFO_H
#define GSCRIPT_INFO_H

#include <filesystem>
class GProjectInfo;

class GScriptInfo
{
public:
	GScriptInfo(GProjectInfo* projectInfo, std::filesystem::path scriptPath,std::string className);
	
	std::filesystem::path m_scriptPath;
	std::string m_className;

private:
	GProjectInfo* m_boundedProjectInfo;
	
};

#endif // GSCRIPT_INFO_H