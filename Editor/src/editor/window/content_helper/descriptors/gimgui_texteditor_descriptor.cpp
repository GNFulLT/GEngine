#include "internal/window/content_helper/descriptors/gimgui_texteditor_descriptor.h"
#include "internal/window/gimgui_texteditor_window.h"
#include <imgui/imgui.h>
#include "editor/editor_application_impl.h"
#include "internal/imgui_layer.h"
#include "internal/imgui_window_manager.h"
#include "internal/utils.h"
GImGuiTextEditorDescriptor::GImGuiTextEditorDescriptor()
{
	for (auto& glsl : all_glsl_files)
	{
		m_supportedFiles.push_back(glsl);
	}
	for (auto& hlsl : all_glsl_files)
	{
		m_supportedFiles.push_back(hlsl);
	}
	m_supportedFiles.push_back(".txt");
}

const std::vector<std::string>* GImGuiTextEditorDescriptor::get_file_types()
{
	return &m_supportedFiles;
}

void GImGuiTextEditorDescriptor::draw_menu_for_file(std::filesystem::path path)
{
	if (ImGui::Selectable("Open in text editor"))
	{
		EditorApplicationImpl::get_instance()->get_editor_layer()->get_window_manager()->try_to_open_file_in_new_editor(path);
	}
}
