#include "internal/manager/gproject_manager.h"
#include <filesystem>
#include <fstream>
#include <spdlog/fmt/fmt.h>

std::expected<GProject*, GPROJECT_CREATE_ERROR> GProjectManager::create_project(std::string path, std::string projectName)
{
	return create_project(path,projectName, projectName);
}

std::expected<GProject*, GPROJECT_CREATE_ERROR> GProjectManager::create_project(std::string path, std::string projectName, std::string projectNamespace)
{
	std::filesystem::path projectFolderPath = std::filesystem::path(path) / projectName;
	if (std::filesystem::exists(projectFolderPath))
	{
		//X TODO : PROJECT ALREADY CREATED
		return std::unexpected(GPROJECT_CREATE_ERROR_ALREADY_EXIST);
	}

	bool created = std::filesystem::create_directory(projectFolderPath);
	if (!created)
	{
		//X TODO : UNKNOWN ERROR
		return std::unexpected(GPROJECT_CREATE_ERROR_UNKNOWN);
	}

	std::string cmakeString = fmt::format(R"(set(GSCRIPT_PROJECT_NAME "{}")
set(GSCRIPT_TARGETED_CPP "{}"))", projectName, "main.cpp");
	std::filesystem::path pth = projectFolderPath / "gscript.cmake";
	std::ofstream ofstream(pth, std::ios::trunc | std::ios::binary | std::ios::out);
	ofstream.exceptions(std::ifstream::badbit);
	bool failed = false;
	try
	{
		ofstream.write(cmakeString.c_str(), sizeof(char) * cmakeString.size());
		ofstream.flush();
	}
	catch (std::exception& ex)
	{
		failed = true;
	}
	ofstream.close();

	//X Copy the build script
	auto buildScriptPath = std::filesystem::current_path() / "editor" / "programs" / "build_script" / "CMakeLists.txt";
	if (!std::filesystem::exists(buildScriptPath))
	{
		//X TODO : LOG 
		return std::unexpected(GPROJECT_CREATE_ERROR_UNKNOWN);
	}
	if (!std::filesystem::copy_file(buildScriptPath, projectFolderPath / "CMakeLists.txt", std::filesystem::copy_options::overwrite_existing))
	{
		//X TODO : LOG
		return std::unexpected(GPROJECT_CREATE_ERROR_UNKNOWN);
	}

	{
		std::ofstream file(projectFolderPath / "main.cpp");
		file.close();
	}

	return new GProject(projectFolderPath.string(), projectName, projectNamespace);
}

void GProjectManager::swap_selected_project(GProject* project)
{
	
	m_selectedProject = project;
}

void GProjectManager::unload_selected_project()
{
	if (m_selectedProject == nullptr)
		return;

	delete m_selectedProject;
	m_selectedProject = nullptr;
}

void GProjectManager::load_project(GProject* project)
{
}
