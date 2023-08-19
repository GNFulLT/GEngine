#ifndef STB_IMAGE_LOADER_H
#define STB_IMAGE_LOADER_H

#include "engine/io/iimage_loader.h"

class STBImageLoader : public IImageLoader 
{
public:
	STBImageLoader();
	~STBImageLoader();


	virtual std::string_view get_loader_name() override;

	virtual GImage_Descriptor* load(std::string_view path) override;

	virtual void unload(GImage_Descriptor* img) override;
private:
	std::string m_name;
};


#endif // STB_IMAGE_LOADER_H