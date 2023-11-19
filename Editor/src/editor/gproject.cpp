#include "internal/gproject.h"

GProject::GProject(std::string projectPath,std::string name, std::string defaultNamespace)
{
	m_projectPath = projectPath;
	m_projectName = name;
	m_defaultNamespace = defaultNamespace;
}
