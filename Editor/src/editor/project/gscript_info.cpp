#include "internal/project/gscript_info.h"

GScriptInfo::GScriptInfo(GProjectInfo* projectInfo, std::filesystem::path scriptPath, std::string className)
{
	m_boundedProjectInfo = projectInfo;
	m_scriptPath = scriptPath;
	m_className = className;
}
