#ifndef GIMAGE_DESCRIPTOR_H
#define GIMAGE_DESCRIPTOR_H

#include <cstdint>


enum VkFormat;

struct GImage_Descriptor
{
	size_t      width;
	size_t      height;
	VkFormat format;
	uint8_t* pixels;
};

#endif // GIMAGE_DESCRIPTOR_H