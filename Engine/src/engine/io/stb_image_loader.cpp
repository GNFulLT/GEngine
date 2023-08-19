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

GImage_Descriptor* STBImageLoader::load(std::string_view path)
{
	int texWidth, texHeight, texChannels;
	auto* pixels = stbi_load(path.data(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);


	GImage_Descriptor* img = new GImage_Descriptor();
	img->channelCount = texChannels;
	img->width = texWidth;
	img->height = texHeight;
	img->format = VK_FORMAT_R8G8B8A8_SRGB;
	img->pixels = (uint8_t*)pixels;
	img->imageType = GIMAGETYPE_2D;
	return img;
}

void STBImageLoader::unload(GImage_Descriptor* img)
{
	delete img;
}
