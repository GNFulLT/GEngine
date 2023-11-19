#ifndef GPROJECT_MANAGER_H
#define GPROJECT_MANAGER_H

#include "internal/gproject.h"
#include <expected>

enum GPROJECT_CREATE_ERROR
{
	GPROJECT_CREATE_ERROR_UNKNOWN,
	GPROJECT_CREATE_ERROR_ALREADY_EXIST
};

class GProjectManager
{
public:
	std::expected<GProject*, GPROJECT_CREATE_ERROR> create_project(std::string path,std::string projectName);

	std::expected<GProject*, GPROJECT_CREATE_ERROR> create_project(std::string path, std::string projectName,std::string projectNamespace);

	void swap_selected_project(GProject* project);

	void unload_selected_project();

	void load_project(GProject* project);
private:
	GProject* m_selectedProject = nullptr;
};

#endif // GPROJECT_MANAGER_H