#ifndef GIMGUI_SHADER_DESCRIPTOR_H
#define GIMGUI_SHADER_DESCRIPTOR_H

#include "editor/igimgui_content_descriptor_impl.h"
#include <future>
#include <atomic>

class IGShaderManager;

class GImGuiShaderDescriptor : public IGImGuiContentDescriptorImpl
{
public:
	GImGuiShaderDescriptor();
	~GImGuiShaderDescriptor();
	// Inherited via IGImGuiContentDescriptorImpl
	virtual const std::vector<FILE_TYPE>* get_file_types() override;
	virtual void draw_menu_for_file(std::filesystem::path path) override;

	void try_to_compile_shader(std::filesystem::path path);
private:
	std::vector<FILE_TYPE> m_supportedFiles;
	std::future<void> m_compileFuture;
	
	IGShaderManager* m_shaderManager;
};

#endif // GIMGUI_SHADER_DESCRIPTOR_H