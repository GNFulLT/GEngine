#ifndef CUBE_IMAGE_LOADER_H
#define CUBE_IMAGE_LOADER_H

#include "engine/io/iimage_loader.h"

class CubemapImageLoader : public IImageLoader
{
public:
	CubemapImageLoader();
	// Inherited via IImageLoader
	virtual std::string_view get_loader_name() override;
	virtual GImage_Descriptor* load(std::string_view path, int dedicatedFormat) override;
	virtual void unload(GImage_Descriptor* img) override;
	virtual bool inject_create_image_info(VkImageCreateInfo* info, VkImageViewCreateInfo* ivInfo) override;
	virtual const std::vector<GIMAGETYPE>* get_supported_image_types() override;
	
private:
	std::vector<GIMAGETYPE> m_supportedImageTypes;
	std::string m_loaderName;

};

#endif // CUBE_IMAGE_LOADER_H