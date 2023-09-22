#ifndef GIMGUI_GMESH_DESCRIPTOR_H
#define GIMGUI_GMESH_DESCRIPTOR_H


#include "internal/rendering/mesh/gmesh_assimp_encoder.h"
#include "editor/igimgui_content_descriptor_impl.h"
#include <future>
#include <atomic>
#include <memory>

class IGShaderManager;

class GImGuiGMeshDescriptor : public IGImGuiContentDescriptorImpl
{
public:
	GImGuiGMeshDescriptor();

	// Inherited via IGImGuiContentDescriptorImpl
	virtual const std::vector<FILE_TYPE>* get_file_types() override;

	virtual void draw_menu_for_file(std::filesystem::path path) override;
private:
	std::vector<FILE_TYPE> m_supportedFileTypes;
};

#endif // GIMGUI_GMESH_DESCRIPTOR_H