#include "internal/window/content_helper/descriptors/gimgui_spirv_descriptor.h"
#include "imgui/imgui.h"
#include "internal/shader/spirv_shader_utils.h"
#include "editor/editor_application_impl.h"
#include "engine/gengine.h"
#include "engine/imanager_table.h"
#include "engine/io/iowning_glogger.h"
#include "engine/manager/igshader_manager.h"
#include "internal/shader/gspirv_byte_shader.h"
#include "internal/imgui_layer.h"
#include "internal/imgui_window_manager.h"

GImGuiSpirvDescriptor::GImGuiSpirvDescriptor()
{
	p_shaderManager = nullptr;
	m_supportedFiles.push_back(FILE_TYPE_SPIRV);
}

const std::vector<FILE_TYPE>* GImGuiSpirvDescriptor::get_file_types()
{
	p_shaderManager = ((GSharedPtr<IGShaderManager>*)EditorApplicationImpl::get_instance()->m_engine->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_SHADER))->get();
	return &m_supportedFiles;
}

//X TODO : Clean code
void GImGuiSpirvDescriptor::draw_menu_for_file(std::filesystem::path path)
{
	if (ImGui::Selectable("Show connections"))
	{
		// First try to deduce the stage from file name
		if (!m_loadFileFuture.valid())
		{
			m_loadFileFuture = std::async([=]() {
				load_spv_file(path);
			});
		}
		else
		{
			auto state = m_loadFileFuture.wait_for(std::chrono::seconds(0));
			if (state == std::future_status::ready)
			{
				m_loadFileFuture = std::async([=]() {
					auto spvFile = load_spv_file(path);
					//X TODO : SAFE CONVERT
					GSpirvByteShader* shader = (GSpirvByteShader*)spvFile;
					if (!shader->is_debug_active())
					{
						EditorApplicationImpl::get_instance()->get_editor_log_window_logger()->log_e("Couldn't disassemble spv file");
					}
					else
					{
						auto debugger = shader->get_debugger();
						auto fileName = path.filename().string();
						EditorApplicationImpl::get_instance()->get_editor_layer()->get_window_manager()->try_to_show_string_in_new_editor(debugger->get_all_uniform_buffers(),fileName,"inspector",true);
					}
				});
			}
		}
		
	}
}

ISpirvShader* GImGuiSpirvDescriptor::load_spv_file(std::filesystem::path path)
{
	auto fileName = path.filename().string();

	auto stage = get_stage_from_spirv_file_name(fileName.data());

	if (stage == SPIRV_SHADER_STAGE_UNKNOWN)
	{
		EditorApplicationImpl::get_instance()->get_editor_log_window_logger()->log_e("Couldn't deduce the stage. Please use this format : [filename].[stage].spv");
		return nullptr;
	}
	// Load the bytes
	auto bytesRes = read_shader_bytes(path);
	if (!bytesRes.has_value())
	{
		EditorApplicationImpl::get_instance()->get_editor_log_window_logger()->log_e("Couldn't read the bytes");
		return nullptr;
	}
	auto bytes = bytesRes.value();
	auto shaderRes = p_shaderManager->load_shader_from_bytes(bytes, stage);
	// Just check bytes are valid spirv
	if (shaderRes.has_value())
	{
		//X TODO : GDNEWDA
		auto shader = shaderRes.value();
		return shader;
	}
}
