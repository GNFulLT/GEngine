#include "internal/window/content_helper/descriptors/gimgui_shader_descriptor.h"
#include <imgui/imgui.h>
#include "editor/editor_application_impl.h"
#include "engine/io/iowning_glogger.h"
#include <fstream>
#include "editor/editor_application_impl.h"
#include "engine/gengine.h"
#include "engine/imanager_table.h"
#include "engine/shader/ispirv_shader.h"
#include "engine/manager/igshader_manager.h"
#include "internal/shader/spirv_shader_utils.h"
#include "internal/utils.h"

GImGuiShaderDescriptor::GImGuiShaderDescriptor()
{
	for (auto& glsl : all_glsl_files)
	{
		m_supportedFiles.push_back(glsl);
	}
	for (auto& hlsl : all_glsl_files)
	{
		m_supportedFiles.push_back(hlsl);
	}
	m_shaderManager = nullptr;
}

GImGuiShaderDescriptor::~GImGuiShaderDescriptor()
{
	int a = 5;
}

const std::vector<std::string>* GImGuiShaderDescriptor::get_file_types()
{
	m_shaderManager = ((GSharedPtr<IGShaderManager>*)EditorApplicationImpl::get_instance()->m_engine->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_SHADER))->get();
	return &m_supportedFiles;
}

void GImGuiShaderDescriptor::draw_menu_for_file(std::filesystem::path path)
{
	if (ImGui::Selectable("Check with compiler"))
	{
		EditorApplicationImpl::get_instance()->get_editor_log_window_logger()->log_d("Trying to compile shader");
		//try_to_compile_shader(path);
		if (!m_compileFuture.valid())
		{
			m_compileFuture = std::async([=]() {
					try_to_compile_shader(path);
				});
		}
		else
		{
			auto state = m_compileFuture.wait_for(std::chrono::seconds(0));
			if (state == std::future_status::ready)
			{
				m_compileFuture = std::async([=]() {
					try_to_compile_shader(path);
				});
			}
		}

	}
}

void GImGuiShaderDescriptor::try_to_compile_shader(std::filesystem::path path)
{
	auto fileName = path.filename().string();
	auto typeStage = shader_stage_from_file_name(fileName.c_str());

	if (typeStage.first == SPIRV_SHADER_STAGE_UNKNOWN)
		return;
	if (typeStage.second == SPIRV_SOURCE_TYPE_UNKNOWN)
		return;

	std::ifstream t(path);
	if (t.good())
	{
		std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
		auto res = m_shaderManager->compile_shader_text(str, typeStage.first, typeStage.second);
		if (res.has_value())
		{
			auto val = res.value();
			// Now write to out
			std::string recommendName = path.filename().stem().string() + stage_to_extension(val->get_spirv_stage()) + ".spv";
			//X TODO : SAVE THE FILE TO THE PROJECT DIRECTORY
			std::ofstream out("./" + recommendName, std::ios::out | std::ios::trunc | std::ios::binary);
			if (out.good())
			{
				//X TODO : ADD ALL ENDIAN SUPPORT
				out.write((char*)val->get_spirv_words(), val->get_size());
				out.flush();
			}
			out.close();
			delete val;
		}
	}
}
