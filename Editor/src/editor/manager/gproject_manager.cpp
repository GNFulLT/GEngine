#include "internal/manager/gproject_manager.h"
#include <filesystem>
#include <fstream>
#include <spdlog/fmt/fmt.h>
#include "engine/io/json_utils.h"
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

	bool created = std::filesystem::create_directories(projectFolderPath);
	if (!created)
	{
		//X TODO : UNKNOWN ERROR
		return std::unexpected(GPROJECT_CREATE_ERROR_UNKNOWN);
	}
	auto gproject = new GProject(projectFolderPath.string(), projectName, projectNamespace);

	std::string cmakeString = fmt::format(R"(
include(GenerateExportHeader)

set(GSCRIPT_PROJECT_NAME "{}")
set(GSCRIPT_TARGETED_CPP )
set(GENGINE_DIR {})
)", projectName,std::filesystem::current_path().string());
	
	auto scriptPath = get_script_path(gproject);

	std::filesystem::path pth = scriptPath / "gscript.cmake";
	
	std::filesystem::create_directories(scriptPath);

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
	if (!std::filesystem::copy_file(buildScriptPath, scriptPath / "CMakeLists.txt", std::filesystem::copy_options::overwrite_existing))
	{
		//X TODO : LOG
		return std::unexpected(GPROJECT_CREATE_ERROR_UNKNOWN);
	}

	auto outPath = projectFolderPath / fmt::format("{}.gproject",projectName);

	if (!GJsonUtils::serialize_igobject(outPath, gproject))
	{
		delete gproject;
		return nullptr;
	}
	auto includeDir = scriptPath / INCLUDE_FOLDER_NAME / gproject->get_project_name();
	auto srcDir = scriptPath / SRC_FOLDER_NAME / gproject->get_project_name();
	std::filesystem::create_directories(includeDir);
	std::filesystem::create_directories(srcDir);

	return gproject;
}

bool GProjectManager::is_path_gproject_path(std::filesystem::path path)
{
	using directory_iterator = std::filesystem::directory_iterator;

	for (const auto& dirEntry : directory_iterator(path))
	{
		if (dirEntry.is_directory())
			continue;
		if (strcmp(dirEntry.path().extension().string().c_str(), ".gproject") == 0)
		{
			return true;
		}
	}
	return false;
}

void GProjectManager::swap_selected_project(GProject* project)
{
	unload_selected_project();
	load_project(project);
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
	m_selectedProject = project;
}

GProject* GProjectManager::get_selected_project() const noexcept
{
	return m_selectedProject;
}

bool GProjectManager::any_project_selected() const noexcept
{
	return m_selectedProject != nullptr;
}

GProject* GProjectManager::open_project(std::filesystem::path path)
{
	if (strcmp(path.extension().string().c_str(), ".gproject") != 0)
		return nullptr;
	GProject* proj = new GProject(path.parent_path().string(), "Unk", "Unk");
	GJsonUtils::set_serialized_values_from_json(path, proj);

	load_project(proj);

	return proj;
}

bool GProjectManager::create_script(const std::string& className)
{
	if (!m_selectedProject)
		return false;
	
	std::string snakeCase;
	for (char ch : className) {
		if (std::isupper(ch)) {
			// Convert uppercase letter to lowercase and add underscore before it
			snakeCase += '_';
			snakeCase += std::tolower(ch);
		}
		else {
			// Keep lowercase letters and digits unchanged
			snakeCase += ch;
		}
	}

	// Remove leading underscore if present
	if (!snakeCase.empty() && snakeCase[0] == '_') {
		snakeCase = snakeCase.substr(1);
	}

	constexpr const std::string_view header = R"(#ifndef {}_H
#define {}_H

#include "engine/scene/component/igscript.h"
#include "{}_EXPORT.h"

class {}_API {} : public IGScript {{
public:
	virtual void update(float dt) override;
private:
}};

#endif)";

	constexpr const std::string_view cpp = R"(#include "{}.h"

void {}::update(float dt)
{{
}}
)";
	auto scriptPath = get_script_path(m_selectedProject);

	auto projectIncludePath = get_include_path(m_selectedProject);
	auto projectSrcPath = get_src_path(m_selectedProject);
	auto hPath = projectIncludePath / fmt::format("{}.h", snakeCase);
	std::ofstream classHeaderStream(hPath);
	std::string upperSnakeCase;
	upperSnakeCase.resize(snakeCase.size());
	std::transform(snakeCase.begin(), snakeCase.end(), upperSnakeCase.begin(), ::toupper);
	{
		auto projectName = m_selectedProject->get_project_name();
		const std::string frmatStr = fmt::format(header.data(), upperSnakeCase, upperSnakeCase, projectName, m_selectedProject->get_project_name(),className);
		classHeaderStream.write(frmatStr.c_str(), frmatStr.size());
	}
	classHeaderStream.flush();
	classHeaderStream.close();

	auto cppPath = projectSrcPath / fmt::format("{}.cpp", snakeCase);
	std::ofstream classCppStream(cppPath);
	{
		auto frmatStr = fmt::format(cpp.data(), snakeCase, className);
		classCppStream.write(frmatStr.c_str(), frmatStr.size());
	}
	classCppStream.flush();
	classCppStream.close();
	std::error_code err;


	auto relativeCppPath = std::filesystem::relative(cppPath, scriptPath,err );
	if(!err)
		add_file_to_project_cmake(m_selectedProject, relativeCppPath);
	else
		add_file_to_project_cmake(m_selectedProject, cppPath);

	auto relativeHPath = std::filesystem::relative(hPath, scriptPath, err);

	if(!err)
		add_file_to_project_cmake(m_selectedProject, relativeHPath);
	else
		add_file_to_project_cmake(m_selectedProject, cppPath);

	return true;
}

const std::string& GProjectManager::get_vs22_path() const noexcept
{
	return m_vs2022Path;
}

bool GProjectManager::add_file_to_project_cmake(GProject* project, std::filesystem::path file)
{
	auto scriptCmakePath = get_script_path(project) / "gscript.cmake";
	std::ifstream scriptCmakeFile(scriptCmakePath);
	if (!scriptCmakeFile.is_open())
	{
		return false;
	}

	std::stringstream buffer;
	buffer << scriptCmakeFile.rdbuf();
	std::string fileContent = buffer.str();

	std::size_t targetPosition = fileContent.find("set(GSCRIPT_TARGETED_CPP ");
	if (targetPosition == std::string::npos) {
		//X TODO ADD AND CONTINUE
		return false;
	}
	std::size_t closingParenthesis = fileContent.find(")", targetPosition);

	std::string newContent = fileContent.insert(closingParenthesis, fmt::format(R"("{}" )", file.string()));

	std::ofstream scriptCmakeOutput(scriptCmakePath);
	if (!scriptCmakeOutput.is_open()) {
		return false;
	}

	scriptCmakeOutput << newContent;

	scriptCmakeOutput.flush();

	return true;
}


