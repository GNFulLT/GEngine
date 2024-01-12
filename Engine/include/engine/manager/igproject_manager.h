#ifndef IGPROJECT_MANAGER_H
#define IGPROJECT_MANAGER_H

#include <expected>
#include <string>
#include <filesystem>
#include "engine/gproject.h"

enum GPROJECT_CREATE_ERROR
{
	GPROJECT_CREATE_ERROR_UNKNOWN,
	GPROJECT_CREATE_ERROR_ALREADY_EXIST
};


class IGProjectManager
{
public:
	inline static constexpr const std::string_view SRC_FOLDER_NAME = "src";
	inline static constexpr const std::string_view INCLUDE_FOLDER_NAME = "include";

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

	virtual ~IGProjectManager() = default;


	virtual std::expected<GProject*, GPROJECT_CREATE_ERROR> create_project(std::string path, std::string projectName) = 0;

	virtual std::expected<GProject*, GPROJECT_CREATE_ERROR> create_project(std::string path, std::string projectName, std::string projectNamespace) = 0;

	virtual GProject* open_project(std::filesystem::path path) = 0;

	virtual void load_project(GProject* project) = 0;

	virtual bool build_script(bool debug) = 0;

	virtual bool save_project(GProject* project) = 0;

	virtual bool is_path_gproject_path(std::filesystem::path path) = 0;

	virtual void swap_selected_project(GProject* project) = 0;

	virtual void unload_selected_project() = 0;

	virtual GProject* get_selected_project() const noexcept = 0;

	virtual bool any_project_selected() const noexcept = 0;

	virtual bool create_script(const std::string& className) = 0;
private:
};

#endif // IGPROJECT_MANAGER_H