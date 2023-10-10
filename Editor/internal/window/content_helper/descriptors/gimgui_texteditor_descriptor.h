#ifndef GIMGUI_TEXTEDITOR_DESCRIPTOR_H
#define GIMGUI_TEXTEDITOR_DESCRIPTOR_H


#include "editor/igimgui_content_descriptor_impl.h"


class GImGuiTextEditorDescriptor : public IGImGuiContentDescriptorImpl
{
public:
	GImGuiTextEditorDescriptor();
	// Inherited via IGImGuiContentDescriptorImpl
	virtual const std::vector<std::string>* get_file_types() override;
	virtual void draw_menu_for_file(std::filesystem::path path) override;
private:
	std::vector<std::string> m_supportedFiles;
};


#endif // GIMGUI_TEXTEDITOR_DESCRIPTOR_H