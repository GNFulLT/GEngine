#include "internal/modal/gimgui_function_modal.h"

GImGuiFunctionModal::GImGuiFunctionModal(const std::string& name,modal_function fnc)
{
	m_name = name;
	m_fnc = fnc;
	m_wantsOpen = true;
}

const char* GImGuiFunctionModal::get_modal_id() const noexcept
{
	return m_name.c_str();
}

void GImGuiFunctionModal::render_modal()
{
	m_wantsOpen = m_fnc();
}

bool GImGuiFunctionModal::wants_to_render() const noexcept
{
	return m_wantsOpen;
}
