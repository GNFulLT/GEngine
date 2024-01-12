#ifndef GPROJECT_MANAGER_H
#define GPROJECT_MANAGER_H

#include "engine/manager/igproject_manager.h"

#include <expected>
#include <filesystem>
#include <string_view>
#include <cassert>

class GProjectManager : public IGProjectManager
{
public:
	GProjectManager();

	std::expected<GProject*, GPROJECT_CREATE_ERROR> create_project(std::string path, std::string projectName) override;

	std::expected<GProject*, GPROJECT_CREATE_ERROR> create_project(std::string path, std::string projectName, std::string projectNamespace) override;

	bool build_script(bool debug) override;

	bool save_project(GProject* project) override;

	bool is_path_gproject_path(std::filesystem::path path);

	void swap_selected_project(GProject* project) override;

	void unload_selected_project()  override;

	void load_project(GProject* project) override;

	GProject* get_selected_project() const noexcept override;

	bool any_project_selected() const noexcept override;

	GProject* open_project(std::filesystem::path path) override;

	bool create_script(const std::string& className) override;

	
	const std::string& get_vs22_path() const noexcept;
private:
	bool add_file_to_project_cmake(GProject* project, std::filesystem::path file);
	bool create_source_cpp_file(GProject* project, std::filesystem::path filePath);
	bool add_script_to_registration(std::filesystem::path mainCppPath, std::filesystem::path scriptHeaderPath, std::string className);
private:
	GProject* m_selectedProject = nullptr;

	std::string m_vs2022Path;
};

#endif // GPROJECT_MANAGER_H