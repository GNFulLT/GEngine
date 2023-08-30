#include "internal/window/content_helper/descriptors/gimgui_shader_descriptor.h"
#include <imgui/imgui.h>
#include "editor/editor_application_impl.h"
#include "engine/io/iowning_glogger.h"

GImGuiShaderDescriptor::GImGuiShaderDescriptor()
{
	m_supportedFiles.push_back(FILE_TYPE_GLSL);
	m_supportedFiles.push_back(FILE_TYPE_HLSL);
}

GImGuiShaderDescriptor::~GImGuiShaderDescriptor()
{
	int a = 5;
}

const std::vector<FILE_TYPE>* GImGuiShaderDescriptor::get_file_types()
{
	return &m_supportedFiles;
}

void GImGuiShaderDescriptor::draw_menu_for_file(std::filesystem::path path)
{
	if (ImGui::Selectable("Check with compiler"))
	{
		EditorApplicationImpl::get_instance()->get_editor_log_window_logger()->log_d("Trying to compile shader");
	}
}
