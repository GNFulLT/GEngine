#ifndef STB_IMAGE_LOADER_H
#define STB_IMAGE_LOADER_H

#include "engine/io/iimage_loader.h"

class STBImageLoader : public IImageLoader 
{
public:
	STBImageLoader();
	~STBImageLoader();


	virtual std::string_view get_loader_name() override;

	virtual GImage_Descriptor* load(std::string_view path,int dedicatedFormat = -1) override;

	virtual void unload(GImage_Descriptor* img) override;
	
	virtual const std::vector<GIMAGETYPE>* get_supported_image_types() override;
private:
	std::string m_name;
	std::vector<GIMAGETYPE> m_supportedImageTypes;
};


#endif // STB_IMAGE_LOADER_H