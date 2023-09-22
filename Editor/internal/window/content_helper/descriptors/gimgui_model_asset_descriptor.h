#ifndef GIMGUI_MODEL_DESCRIPTOR_H
#define GIMGUI_MODEL_DESCRIPTOR_H

#include "internal/rendering/mesh/gmesh_assimp_encoder.h"
#include "editor/igimgui_content_descriptor_impl.h"
#include <future>
#include <atomic>
#include <memory>

class IGShaderManager;

class GImGuiModelAssetDescriptor : public IGImGuiContentDescriptorImpl
{
public:
	GImGuiModelAssetDescriptor();
	// Inherited via IGImGuiContentDescriptorImpl
	virtual const std::vector<FILE_TYPE>* get_file_types() override;
	virtual void draw_menu_for_file(std::filesystem::path path) override;
private:
	std::vector<FILE_TYPE> m_supportedTypes;
	std::unique_ptr<GMeshEncoder> m_encoder;
};

#endif // GIMGUI_MODEL_DESCRIPTOR_H