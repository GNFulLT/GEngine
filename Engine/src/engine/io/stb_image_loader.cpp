#include "internal/engine/io/stb_image_loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "public/core/stb_image.h"
#include <vulkan/vulkan.h>

STBImageLoader::STBImageLoader()
{
	m_name = "STBImageLoader";
}

STBImageLoader::~STBImageLoader()
{
}

std::string_view STBImageLoader::get_loader_name()
{
	return m_name;
}

GImage_Descriptor* STBImageLoader::load(std::string_view path,int dedicatedFormat)
{
	if (dedicatedFormat != -1 && (dedicatedFormat != VK_FORMAT_B8G8R8A8_SRGB && dedicatedFormat != VK_FORMAT_B8G8R8_SRGB && dedicatedFormat != VK_FORMAT_R8G8B8A8_SRGB && dedicatedFormat != VK_FORMAT_R8G8B8_SRGB && dedicatedFormat != VK_FORMAT_R8G8B8A8_SNORM && dedicatedFormat != VK_FORMAT_R8G8B8_SNORM))
	{
		return nullptr;
	}
	int texWidth, texHeight, texChannels;
	auto* pixels = stbi_load(path.data(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);


	GImage_Descriptor* img = new GImage_Descriptor();
	img->channelCount = texChannels;
	img->width = texWidth;
	img->height = texHeight;
	if(dedicatedFormat == -1)
		img->format = texChannels == 4 ? VK_FORMAT_B8G8R8A8_SRGB : VK_FORMAT_B8G8R8_SRGB;
	else
	{
		img->format = (VkFormat)dedicatedFormat;
	}
	img->pixels = (uint8_t*)pixels;
	img->imageType = GIMAGETYPE_2D;
	return img;
}

void STBImageLoader::unload(GImage_Descriptor* img)
{
	delete img;
}
