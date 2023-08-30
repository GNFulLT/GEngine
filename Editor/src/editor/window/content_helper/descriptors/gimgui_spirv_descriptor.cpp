#include "internal/window/content_helper/descriptors/gimgui_spirv_descriptor.h"
#include "imgui/imgui.h"
#include "internal/shader/spirv_shader_utils.h"
#include "editor/editor_application_impl.h"
#include "engine/gengine.h"
#include "engine/imanager_table.h"
#include "engine/io/iowning_glogger.h"
#include "engine/manager/igshader_manager.h"

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
	if (ImGui::Selectable("Load shader bytes"))
	{
		// First try to deduce the stage from file name
		auto fileName = path.filename().string();
		auto stage = get_stage_from_spirv_file_name(fileName.data());
		if (stage == SPIRV_SHADER_STAGE_UNKNOWN)
		{
			EditorApplicationImpl::get_instance()->get_editor_log_window_logger()->log_e("Couldn't deduce the stage. Please use this format : [filename].[stage].spv");
		}
		else
		{
			// Load the bytes
			auto bytesRes = read_shader_bytes(path);
			if (!bytesRes.has_value())
			{
				EditorApplicationImpl::get_instance()->get_editor_log_window_logger()->log_e("Couldn't read the bytes");
			}
			else
			{
				auto bytes = bytesRes.value();
				auto shaderRes = p_shaderManager->load_shader_from_bytes(bytes,stage);
				// Just check bytes are valid spirv
				if (shaderRes.has_value())
				{
					//X TODO : GDNEWDA
					auto shader = shaderRes.value();
					delete shader;
					EditorApplicationImpl::get_instance()->get_editor_log_window_logger()->log_d("Loaded bytes");

				}
			}
			
		}
	}
}
