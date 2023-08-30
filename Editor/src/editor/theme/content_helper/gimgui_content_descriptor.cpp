#include "internal/window/content_helper/gimgui_content_descriptor.h"

GImGuiContentDescriptor::GImGuiContentDescriptor(IGImGuiContentDescriptorImpl* impl)
{
	m_impl = impl;
}

const std::vector<FILE_TYPE>* GImGuiContentDescriptor::get_file_type()
{
	return m_impl->get_file_types();
}

void GImGuiContentDescriptor::draw_menu_for_file(std::filesystem::path path)
{
	m_impl->draw_menu_for_file(path);
}
