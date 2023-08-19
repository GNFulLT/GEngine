#ifndef IIMAGE_LOADER_H
#define IIMAGE_LOADER_H

#include "engine/GEngine_EXPORT.h"

#include <string_view>
#include "engine/io/gimage_descriptor.h"

class ENGINE_API IImageLoader
{
public:

	virtual std::string_view get_loader_name() = 0;

	virtual GImage_Descriptor* load(std::string_view path) = 0;

	virtual void unload(GImage_Descriptor* img) = 0;
private:
};

#endif // IIMAGE_LOADER_H