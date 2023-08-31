#ifndef GIMGUI_SPIRV_DESCRIPTOR_H
#define GIMGUI_SPIRV_DESCRIPTOR_H

#include "editor/igimgui_content_descriptor_impl.h"
#include <future>

class IGShaderManager;
class ISpirvShader;
class GImGuiSpirvDescriptor : public IGImGuiContentDescriptorImpl
{
public:
	GImGuiSpirvDescriptor();
	// Inherited via IGImGuiContentDescriptorImpl
	virtual const std::vector<FILE_TYPE>* get_file_types() override;
	virtual void draw_menu_for_file(std::filesystem::path path) override;
private:
	ISpirvShader* load_spv_file(std::filesystem::path path);
private:
	IGShaderManager* p_shaderManager;
	std::vector<FILE_TYPE> m_supportedFiles;
	std::future<void> m_loadFileFuture;
};

#endif // GIMGUI_SPIRV_DESCRIPTOR_H