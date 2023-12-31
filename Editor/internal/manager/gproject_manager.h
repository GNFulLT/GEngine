#ifndef GPROJECT_MANAGER_H
#define GPROJECT_MANAGER_H

#include "engine/gproject.h"
#include <expected>
#include <filesystem>
#include <string_view>
#include <cassert>

enum GPROJECT_CREATE_ERROR
{
	GPROJECT_CREATE_ERROR_UNKNOWN,
	GPROJECT_CREATE_ERROR_ALREADY_EXIST
};

class GProjectManager
{
public:
	inline static constexpr const std::string_view SRC_FOLDER_NAME = "src";
	inline static constexpr const std::string_view INCLUDE_FOLDER_NAME = "include";
	
	GProjectManager();

	std::expected<GProject*, GPROJECT_CREATE_ERROR> create_project(std::string path,std::string projectName);

	std::expected<GProject*, GPROJECT_CREATE_ERROR> create_project(std::string path, std::string projectName,std::string projectNamespace);

	bool save_project(GProject* project);

	bool is_path_gproject_path(std::filesystem::path path);

	void swap_selected_project(GProject* project);

	void unload_selected_project();

	void load_project(GProject* project);

	GProject* get_selected_project() const noexcept;

	bool any_project_selected() const noexcept;

	GProject* open_project(std::filesystem::path path);

	bool create_script(const std::string& className);

	inline static std::filesystem::path get_include_path(GProject* proj)
	{
		assert(proj != nullptr);
		return get_script_path(proj) / INCLUDE_FOLDER_NAME / proj->get_project_name();
	} 

	inline static std::filesystem::path get_src_path(GProject* proj) 
	{
		assert(proj != nullptr);
		return get_script_path(proj) / SRC_FOLDER_NAME / proj->get_project_name();
	}

	inline static std::filesystem::path get_script_path(GProject* proj)
	{
		assert(proj != nullptr);
		return std::filesystem::path(proj->get_project_path()) / proj->get_script_path();
	}
	
	inline static std::filesystem::path get_script_include_path(GProject* proj)
	{
		assert(proj != nullptr);
		return get_script_path(proj) / INCLUDE_FOLDER_NAME;
	}
	inline static std::filesystem::path get_asset_mesh_path(GProject* proj)
	{
		assert(proj != nullptr);
		return std::filesystem::path(proj->get_project_path()) / "Assets" / "Meshes";

	}
	inline static std::filesystem::path get_asset_texture_path(GProject* proj)
	{
		assert(proj != nullptr);
		return std::filesystem::path(proj->get_project_path()) / "Assets" / "Textures";
	}
	inline static std::filesystem::path get_asset_material_path(GProject* proj)
	{
		assert(proj != nullptr);
		return std::filesystem::path(proj->get_project_path()) / "Assets" / "Materials";
	}
	const std::string& get_vs22_path() const noexcept;
private:
	bool add_file_to_project_cmake(GProject* project,std::filesystem::path file);
	bool create_source_cpp_file(GProject* project,std::filesystem::path filePath);
	bool add_script_to_registration(std::filesystem::path mainCppPath,std::filesystem::path scriptHeaderPath,std::string className);
private:
	GProject* m_selectedProject = nullptr;

	std::string m_vs2022Path;
};

#endif // GPROJECT_MANAGER_H