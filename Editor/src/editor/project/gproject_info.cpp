#include "internal/project/gproject_info.h"

GProjectInfo::GProjectInfo(GProject* project)
{
	m_boundedProject = project;
}

GProjectInfo::~GProjectInfo()
{
	for (auto& pair : m_scriptInfoMap)
	{
		delete pair.second;
	}
}

bool GProjectInfo::add_script(std::filesystem::path scriptPath, std::string className)
{
	if (auto p = m_scriptInfoMap.find(scriptPath); p != m_scriptInfoMap.end())
		return false;
	auto inf = new GScriptInfo(this, scriptPath, className);
	m_scriptInfoMap.emplace(scriptPath, inf);
	return true;
}
