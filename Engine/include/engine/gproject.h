#ifndef GPROJECT_H
#define GPROJECT_H

#include <string>
#include "engine/igobject.h"
#include "engine/GEngine_EXPORT.h"

class ENGINE_API GProject : public IGObject
{
	GOBJECT_DEF(GProject, IGObject)
public:
	GProject(std::string projectPath,std::string name,std::string defaultNamespace);
	
	const char* get_project_path() const noexcept;

	const char* get_project_name() const noexcept;

	const char* get_project_namespace() const noexcept;

	const char* get_script_path();

	void set_project_path(const std::string& path);

	GPROPERTY(NAME=projectName)
	std::string m_projectName;

	GPROPERTY(NAME=projectNamespace)
	std::string m_defaultNamespace;

	GPROPERTY(NAME = scriptBinaryPath)
	std::string m_scriptBinaryPath;
private:
	std::string m_projectPath;
	
};

#endif // GPROJECT_H