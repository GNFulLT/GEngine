#include "internal/window/gimgui_script_window.h"
#include "engine/gengine.h"
#include "engine/imanager_table.h"
#include <imgui/imgui.h>
#include "engine/plugin/igscript_space.h"

GImGuiScriptWindow::GImGuiScriptWindow()
{
	m_windowName = "Scripts";
}

bool GImGuiScriptWindow::init()
{
	m_scriptManager = ((GSharedPtr<IGScriptManager>*)GEngine::get_instance()->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_SCRIPT))->get();
	return true;
}

void GImGuiScriptWindow::set_storage(GImGuiWindowStorage* storage)
{
}

bool GImGuiScriptWindow::need_render()
{
	return true;
}

void GImGuiScriptWindow::render()
{
	auto scripts = m_scriptManager->get_loaded_script_spaces();

	for (auto scriptSpace : *scripts)
	{
		if (ImGui::CollapsingHeader(scriptSpace->get_script_space_name()))
		{
			auto scriptObjs = scriptSpace->get_loaded_script_objects();
			for (auto scriptObj : *scriptObjs)
			{
				auto scriptName = scriptObj->get_script_name();
				ImGui::Text(scriptName);
			}
		}
	}
}

void GImGuiScriptWindow::on_resize()
{
}

void GImGuiScriptWindow::on_data_update()
{
}

void GImGuiScriptWindow::destroy()
{
}

const char* GImGuiScriptWindow::get_window_name()
{
	return m_windowName.c_str();
}
