#include "internal/window/content_helper/descriptors/gimgui_gmesh_descriptor.h"
#include <imgui/imgui.h>

#include "editor/editor_application_impl.h"
#include "engine/io/iowning_glogger.h"

GImGuiGMeshDescriptor::GImGuiGMeshDescriptor()
{
	m_supportedFileTypes.push_back(FILE_TYPE_GMESH);
}

const std::vector<FILE_TYPE>* GImGuiGMeshDescriptor::get_file_types()
{
	return &m_supportedFileTypes;
}

void GImGuiGMeshDescriptor::draw_menu_for_file(std::filesystem::path path)
{
	if (ImGui::Selectable("Load gmesh file"))
	{
		auto res = decode_file(path.string().c_str());
		if (res.has_value())
		{
			EditorApplicationImpl::get_instance()->get_editor_log_window_logger()->log_d("Loaded");
			auto lod = res.value();

			delete lod;
		}
		else
		{
			EditorApplicationImpl::get_instance()->get_editor_log_window_logger()->log_e("Error while trying to decode gmesh");
		}
	}

}
