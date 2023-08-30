#ifndef GIMGUI_CONTENT_DESCRIPTOR_H
#define GIMGUI_CONTENT_DESCRIPTOR_H

#include "editor/igimgui_content_descriptor_impl.h"

class GImGuiContentDescriptor
{
public:
	GImGuiContentDescriptor(IGImGuiContentDescriptorImpl* impl);

	const std::vector<FILE_TYPE>* get_file_type();

	void draw_menu_for_file(std::filesystem::path path);
private:
	IGImGuiContentDescriptorImpl* m_impl;

};

#endif // GIMGUI_CONTENT_DESCRIPTOR_H