#ifndef GIMGUI_SHADER_DESCRIPTOR_H
#define GIMGUI_SHADER_DESCRIPTOR_H

#include "editor/igimgui_content_descriptor_impl.h"


class GImGuiShaderDescriptor : public IGImGuiContentDescriptorImpl
{
public:
	GImGuiShaderDescriptor();
	// Inherited via IGImGuiContentDescriptorImpl
	virtual const std::vector<FILE_TYPE>* get_file_types() override;
	virtual void draw_menu_for_file(std::filesystem::path path) override;
private:
	std::vector<FILE_TYPE> m_supportedFiles;
};

#endif // GIMGUI_SHADER_DESCRIPTOR_H