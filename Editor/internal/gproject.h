#ifndef GPROJECT_H
#define GPROJECT_H

#include <string>

class GProject
{
public:
	GProject(std::string projectPath,std::string name,std::string defaultNamespace);
private:
	std::string m_projectPath;
	std::string m_projectName;
	std::string m_defaultNamespace;
	std::string m_scriptBinaryPath;
};

#endif // GPROJECT_H