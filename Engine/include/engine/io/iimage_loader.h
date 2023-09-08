#ifndef IIMAGE_LOADER_H
#define IIMAGE_LOADER_H

#include "engine/GEngine_EXPORT.h"

#include <string_view>
#include "engine/io/gimage_descriptor.h"

struct VkImageCreateInfo;
struct VkImageViewCreateInfo;

#include <vector>
#include <string>

class ENGINE_API IImageLoader
{
public:
	virtual ~IImageLoader() = default;

	virtual std::string_view get_loader_name() = 0;

	virtual GImage_Descriptor* load(std::string_view path,int dedicatedFormat) = 0;

	virtual void unload(GImage_Descriptor* img) = 0;

	virtual bool inject_create_image_info(VkImageCreateInfo* info, VkImageViewCreateInfo* ivInfo) { return false; }
	
	virtual const std::vector<GIMAGETYPE>* get_supported_image_types() = 0;

	bool has_support_for(GIMAGETYPE type);
private:
};

#endif // IIMAGE_LOADER_H