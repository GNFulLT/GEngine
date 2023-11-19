#include "engine/gproject.h"

GProject::GProject(std::string projectPath,std::string name, std::string defaultNamespace)
{
	m_projectPath = projectPath;
	m_projectName = name;
	m_defaultNamespace = defaultNamespace;
}

const char* GProject::get_project_path() const noexcept
{
	return m_projectPath.c_str();
}

const char* GProject::get_project_name() const noexcept
{
	return m_projectName.c_str();
}

const char* GProject::get_project_namespace() const noexcept
{
	return m_defaultNamespace.c_str();
}
