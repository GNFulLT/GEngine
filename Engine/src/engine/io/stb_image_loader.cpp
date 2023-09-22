#include "internal/engine/io/stb_image_loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "public/core/stb_image.h"
#include <vulkan/vulkan.h>

STBImageLoader::STBImageLoader()
{
	m_name = "STBImageLoader";
	m_supportedImageTypes.push_back(GIMAGETYPE_2D);
}

STBImageLoader::~STBImageLoader()
{
}

std::string_view STBImageLoader::get_loader_name()
{
	return m_name;
}

bool IImageLoader::has_support_for(GIMAGETYPE type)
{
	auto supportedTypes = get_supported_image_types();
	if (supportedTypes == nullptr)
		return false;
	for (auto supportedType : *supportedTypes)
	{
		if (supportedType == type)
			return true;
	}

	return false;
}

GImage_Descriptor* STBImageLoader::load(std::string_view path,int dedicatedFormat)
{
	int texWidth, texHeight, texChannels;
	auto* pixels = stbi_load(path.data(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	if (pixels == nullptr)
		return nullptr;

	GImage_Descriptor* img = new GImage_Descriptor();
	img->channelCount = texChannels;
	img->width = texWidth;
	img->height = texHeight;
	if(dedicatedFormat == -1)
		img->format = texChannels == 4 ? VK_FORMAT_B8G8R8A8_UNORM : VK_FORMAT_B8G8R8_UNORM;
	else
	{
		img->format = (VkFormat)dedicatedFormat;
	}
	img->size = texWidth * texHeight * 4;
	img->pixels = (uint8_t*)pixels;
	img->imageType = GIMAGETYPE_2D;
	return img;
}

void STBImageLoader::unload(GImage_Descriptor* img)
{
	stbi_image_free(img->pixels);
	delete img;
}

const std::vector<GIMAGETYPE>* STBImageLoader::get_supported_image_types()
{
	return &m_supportedImageTypes;
}
