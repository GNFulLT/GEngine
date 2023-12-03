#include "internal/manager/gproject_manager.h"
#include <filesystem>
#include <fstream>
#include <spdlog/fmt/fmt.h>
#include "engine/io/json_utils.h"

inline static constexpr const std::string_view exportString = R"(#include "{}_EXPORT.h"
#include <engine/plugin/gplugin.h> 

#define API_DEF {}_API
// Script Includes
{}

bool script_space_register(const GNFScriptRegistration* registration)
{{
		GNFScriptSpaceRegisterArgs space;
		space.scriptNamespace = GSCRIPT_SPACE_NAME;
		space.pluginVersion.version_minor = 0;
		space.pluginVersion.version_major = 1;
		return registration->scriptSpaceRegister(&space);
}}	

extern "C" API_DEF GSCRIPT_REGISTRATION 
{{

	bool spaceRegistered = script_space_register(registration);
	if(!spaceRegistered)
	{{
		return;
	}}
	GNFScriptRegisterArgs scriptRegister;
	scriptRegister.pluginVersion.version_minor = 0;
	scriptRegister.pluginVersion.version_major = 1;
	
	// Script Register
	{}
}}
)";

std::string replaced_path(std::filesystem::path path)
{
	auto str = path.string();
	std::replace(str.begin(),str.end(),'\\', '/');
	return str;
}

GProjectManager::GProjectManager()
{
	std::filesystem::path posix_path("a/b/c");
	posix_path.make_preferred();
}


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
set(GENGINE_DIR "{}")
set(GSCRIPT_NAMESPACE_NAME "{}")
set(GSCRIPT_TARGETED_CPP "src/{}/main.cpp" )
)", projectName, replaced_path(std::filesystem::current_path()), projectNamespace, projectName);
	

	

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
	
	create_source_cpp_file(gproject, srcDir / "main.cpp");


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
#include <engine/plugin/gplugin.h> 

class {} : public IGScript {{
public:
	virtual void update(float dt) override;
private:
}};

extern "C" pGNFScriptObject {}_C_CTOR(pGObject* obj)
{{
	return new {}();
}}

extern "C" void {}_C_DTOR(pGNFScriptObject* obj)
{{	
	auto realObj = ({}*)obj;
	delete realObj;
}}

extern "C" bool {}_REGISTER_FUNC(const GNFScriptRegistration* registration,GNFScriptRegisterArgs* arg)
{{
	arg->scriptName = "{}";
	arg->scriptCtor = &{}_C_CTOR;
	arg->scriptDtor = &{}_C_DTOR;

	return registration->scriptRegister(arg);
}}  

#endif)";

	constexpr const std::string_view cpp = R"(#include "{}/{}.h"

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
		const std::string frmatStr = fmt::format(header.data(), upperSnakeCase, upperSnakeCase, className,className,
			className,className,className,className,className,className, className);
		classHeaderStream.write(frmatStr.c_str(), frmatStr.size());
	}
	classHeaderStream.flush();
	classHeaderStream.close();

	auto cppPath = projectSrcPath / fmt::format("{}.cpp", snakeCase);
	std::ofstream classCppStream(cppPath);
	{
		auto frmatStr = fmt::format(cpp.data(), m_selectedProject->get_project_name(),snakeCase, className);
		classCppStream.write(frmatStr.c_str(), frmatStr.size());
	}
	classCppStream.flush();
	classCppStream.close();
	std::error_code err;


	auto relativeCppPath = std::filesystem::relative(cppPath, scriptPath,err );
	if(!err)
		add_file_to_project_cmake(m_selectedProject, replaced_path(relativeCppPath));
	else
		add_file_to_project_cmake(m_selectedProject, replaced_path(cppPath));

	auto relativeHPath = std::filesystem::relative(hPath, scriptPath, err);
	auto srcDir = get_src_path(m_selectedProject);

	if (!err)
	{
		add_file_to_project_cmake(m_selectedProject, replaced_path(relativeHPath));
	}
	else
	{
		add_file_to_project_cmake(m_selectedProject, replaced_path(hPath));
	}
	add_script_to_registration(srcDir / "main.cpp", hPath, className);


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

bool GProjectManager::create_source_cpp_file(GProject* project,std::filesystem::path filePath)
{
	std::ofstream fileStream(filePath);
	if (!fileStream.is_open()) {
		return false;
	}
	auto projName = project->get_project_name();
	fileStream << fmt::format(exportString.data(), projName, projName,"","").c_str();

	return true;
}

bool GProjectManager::add_script_to_registration(std::filesystem::path mainCppPath,std::filesystem::path scriptHeaderPath, std::string className)
{
	std::ifstream mainFile(mainCppPath);
	if (!mainFile.is_open()) {
		return false;
	}

	std::stringstream buffer;
	buffer << mainFile.rdbuf();
	std::string fileContent = buffer.str();

	std::size_t targetPosition = fileContent.find("GSCRIPT_REGISTRATION");
	if (targetPosition == std::string::npos) {
		return false;
	}

	//X Find first bracket
	std::size_t openingParenthesis = fileContent.find("{", targetPosition);
	if (openingParenthesis == std::string::npos) {
		return false;
	}
	//X Find closing brack
	int balance = 1;
	std::size_t iter = openingParenthesis;
	while (balance != 0)
	{
		iter++;
		if (fileContent[iter] == '{')
			balance++;
		else if (fileContent[iter] == '}')
			balance--;
	}
	std::string newContent;
	newContent = fileContent.insert(iter, fmt::format(R"(	{}_REGISTER_FUNC(registration,&scriptRegister);
)",className));

	
	auto scriptPath = get_script_include_path(m_selectedProject);
	std::error_code errCode;
	auto relativeHPath = std::filesystem::relative(scriptHeaderPath, scriptPath, errCode);
	auto relativeHStr = replaced_path(relativeHPath);
	newContent = newContent.insert(0,fmt::format("#include <{}>\n", relativeHStr));

	std::ofstream mainOutput(mainCppPath);
	if (!mainOutput.is_open()) {
		return false;
	}

	mainOutput << newContent;
	mainOutput.flush();

	return true;

}


