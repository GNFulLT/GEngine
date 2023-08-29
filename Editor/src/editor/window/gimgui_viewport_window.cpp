#include "internal/window/gimgui_viewport_window.h"
#include "imgui.h"
#include "engine/rendering/vulkan/ivulkan_viewport.h"
#include "volk.h"
#include "engine/rendering/vulkan/ivulkan_descriptor.h"
#include <algorithm>

GImGuiViewportWindow::GImGuiViewportWindow()
{
	m_name = "Viewport";
	m_initedTheViewportFirstTime = false;
}

void GImGuiViewportWindow::set_the_viewport(IGVulkanViewport* viewport)
{
	m_viewport = viewport;
	
	if (!m_initedTheViewportFirstTime)
	{
		m_viewport->init(640, 320, VkFormat::VK_FORMAT_B8G8R8A8_UNORM);
		m_initedTheViewportFirstTime = true;
		return;
	}
}

bool GImGuiViewportWindow::init()
{
	return true;
}

void GImGuiViewportWindow::set_storage(GImGuiWindowStorage* storage)
{
	m_storage = storage;
}

bool GImGuiViewportWindow::need_render()
{
	return true;
}

void GImGuiViewportWindow::render()
{
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	int x = std::max(0, int(viewportPanelSize.x));
	int y = std::max(0, int(viewportPanelSize.y));
	auto t = m_viewport->get_descriptor();
	if (t != nullptr)
	{
		ImGui::Image(t->get_vk_descriptor(), ImVec2{ float(x),float(y) });
	}
}

void GImGuiViewportWindow::on_resize()
{
	m_viewport->destroy();
	if (m_storage->width > 0 && m_storage->width < 1920 && m_storage->height > 0 && m_storage->height < 1080)
	{
		m_viewport->init(m_storage->width, m_storage->height, VkFormat::VK_FORMAT_B8G8R8A8_UNORM);
	}
}

void GImGuiViewportWindow::on_data_update()
{
}

const char* GImGuiViewportWindow::get_window_name()
{
	return m_name.c_str();
}

void GImGuiViewportWindow::destroy()
{
}
