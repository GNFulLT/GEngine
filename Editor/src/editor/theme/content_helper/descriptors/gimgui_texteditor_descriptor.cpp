#include "internal/window/content_helper/descriptors/gimgui_texteditor_descriptor.h"
#include "internal/window/gimgui_texteditor_window.h"
#include <imgui/imgui.h>
#include "editor/editor_application_impl.h"
#include "internal/imgui_layer.h"
#include "internal/imgui_window_manager.h"
GImGuiTextEditorDescriptor::GImGuiTextEditorDescriptor()
{
	m_supportedFiles.push_back(FILE_TYPE_GLSL);
	m_supportedFiles.push_back(FILE_TYPE_HLSL);
	m_supportedFiles.push_back(FILE_TYPE_TXT);
}

const std::vector<FILE_TYPE>* GImGuiTextEditorDescriptor::get_file_types()
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
