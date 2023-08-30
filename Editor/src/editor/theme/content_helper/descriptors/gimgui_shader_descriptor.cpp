#include "internal/window/content_helper/descriptors/gimgui_shader_descriptor.h"
#include <imgui/imgui.h>

GImGuiShaderDescriptor::GImGuiShaderDescriptor()
{
	m_supportedFiles.push_back(FILE_TYPE_GLSL);
	m_supportedFiles.push_back(FILE_TYPE_HLSL);
}

const std::vector<FILE_TYPE>* GImGuiShaderDescriptor::get_file_types()
{
	return &m_supportedFiles;
}

void GImGuiShaderDescriptor::draw_menu_for_file(std::filesystem::path path)
{
	if (ImGui::Selectable("Check with compiler"))
	{

	}
}
