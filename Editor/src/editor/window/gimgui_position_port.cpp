#include "internal/window/gimgui_position_port.h"
#include "editor/editor_application_impl.h"
#include "engine/rendering/vulkan/named/igvulkan_named_viewport.h"
#include "internal/rendering/vulkan/gimgui_descriptor_creator.h"
#include "imgui/imgui.h"
#include <algorithm>
#include "engine/rendering/vulkan/ivulkan_descriptor.h"
#include "engine/gengine.h"
GImGuiPositionPortWindow::GImGuiPositionPortWindow()
{
	m_name = "Position";
}

bool GImGuiPositionPortWindow::init()
{
	
	return true;
}

void GImGuiPositionPortWindow::set_storage(GImGuiWindowStorage* storage)
{
	m_storage = storage;
}

bool GImGuiPositionPortWindow::need_render()
{
	return true;
}

void GImGuiPositionPortWindow::render()
{
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	int x = std::max(0, int(viewportPanelSize.x));
	int y = std::max(0, int(viewportPanelSize.y));
	if (EditorApplicationImpl::get_instance()->positionPortSet != nullptr)
	{
		ImGui::Image(EditorApplicationImpl::get_instance()->positionPortSet->get_vk_descriptor(), ImVec2{ float(x),float(y) });
	}
	

}

void GImGuiPositionPortWindow::on_resize()
{
	EditorApplicationImpl::get_instance()->m_engine->add_recreation([&,posPort = EditorApplicationImpl::get_instance()->positionPortSet,
	albedoPort = EditorApplicationImpl::get_instance()->albedoPortSet,normalPort = EditorApplicationImpl::get_instance()->normalPortSet,composSet = EditorApplicationImpl::get_instance()->compositionPortSet]() {
		assert(m_storage->width > 0 && m_storage->height > 0);
		//X Destroy Sets
		EditorApplicationImpl::get_instance()->get_descriptor_creator()->destroy_descriptor_set_dtor(posPort);
		EditorApplicationImpl::get_instance()->get_descriptor_creator()->destroy_descriptor_set_dtor(normalPort);
		EditorApplicationImpl::get_instance()->get_descriptor_creator()->destroy_descriptor_set_dtor(albedoPort);
		EditorApplicationImpl::get_instance()->get_descriptor_creator()->destroy_descriptor_set_dtor(composSet);

		auto deferredVp = EditorApplicationImpl::get_instance()->m_engine->get_deferred_vp();
		EditorApplicationImpl::get_instance()->m_engine->resize_deferred(m_storage->width, m_storage->height);


		EditorApplicationImpl::get_instance()->positionPortSet = EditorApplicationImpl::get_instance()->get_descriptor_creator()->create_descriptor_set_for_texture(deferredVp->get_named_attachment("position_attachment"),
			deferredVp->get_sampler_for_named_attachment("position_attachment")).value();
		EditorApplicationImpl::get_instance()->normalPortSet = EditorApplicationImpl::get_instance()->get_descriptor_creator()->create_descriptor_set_for_texture(deferredVp->get_named_attachment("normal_attachment"),
			deferredVp->get_sampler_for_named_attachment("normal_attachment")).value();
		EditorApplicationImpl::get_instance()->albedoPortSet = EditorApplicationImpl::get_instance()->get_descriptor_creator()->create_descriptor_set_for_texture(deferredVp->get_named_attachment("albedo_attachment"),
			deferredVp->get_sampler_for_named_attachment("albedo_attachment")).value();
		EditorApplicationImpl::get_instance()->compositionPortSet = EditorApplicationImpl::get_instance()->get_descriptor_creator()->create_descriptor_set_for_texture(deferredVp->get_named_attachment("composition_attachment"),
			deferredVp->get_sampler_for_named_attachment("composition_attachment")).value();
		
		});

	EditorApplicationImpl::get_instance()->albedoPortSet = nullptr;
	EditorApplicationImpl::get_instance()->normalPortSet = nullptr;
	EditorApplicationImpl::get_instance()->positionPortSet = nullptr;
	EditorApplicationImpl::get_instance()->compositionPortSet = nullptr;

}

void GImGuiPositionPortWindow::on_data_update()
{
}

void GImGuiPositionPortWindow::destroy()
{
}

const char* GImGuiPositionPortWindow::get_window_name()
{
	return m_name.c_str();
}
