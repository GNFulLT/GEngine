#ifndef GIMAGE_DESCRIPTOR_H
#define GIMAGE_DESCRIPTOR_H

#include <cstdint>


enum VkFormat;

enum GIMAGETYPE
{
	GIMAGETYPE_2D,
	GIMAGETYPE_CUBEMAP
};

struct GImage_Descriptor
{
	std::size_t      width;
	std::size_t      height;
	VkFormat format;
	std::uint8_t* pixels;
	std::size_t channelCount;
	GIMAGETYPE imageType;
	std::uint32_t flag = 0;
	std::uint32_t size = 0;
};

#endif // GIMAGE_DESCRIPTOR_H