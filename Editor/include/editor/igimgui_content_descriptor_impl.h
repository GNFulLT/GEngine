#ifndef IGIMGUI_CONTENT_DESCRIPTOR_H
#define IGIMGUI_CONTENT_DESCRIPTOR_H


#include "GEngine_EXPORT.h"
#include "editor/file_type.h"
#include <filesystem>
#include <vector>
class EDITOR_API IGImGuiContentDescriptorImpl
{
public:
	virtual ~IGImGuiContentDescriptorImpl() = default;

	virtual const std::vector<FILE_TYPE>* get_file_types() = 0;
	
	virtual void draw_menu_for_file(std::filesystem::path path) = 0;
private:
};
#endif // IGIMGUI_CONTENT_DESCRIPTOR