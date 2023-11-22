#include "internal/gimgui_modal.h"
#include <imgui/imgui.h>

GImGuiModal::GImGuiModal(IGImGuiModalImpl* impl)
{
	m_impl = impl;
	m_isModalOpen = false;
}

GImGuiModal::~GImGuiModal()
{
	delete m_impl;
}

bool GImGuiModal::render_modal()
{
	m_isModalOpen = m_impl->wants_to_render();
	if (m_isModalOpen)
	{
		ImGui::OpenPopup(m_impl->get_modal_id());
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	}

	if (ImGui::BeginPopupModal(m_impl->get_modal_id(),nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		m_impl->render_modal();
		ImGui::EndPopup();
	}
	else
	{
		m_isModalOpen = false;
	}
	return m_isModalOpen;
}
