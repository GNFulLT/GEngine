#ifndef IGRESOURCE_LOADER_H
#define IGRESOURCE_LOADER_H

#include "engine/GEngine_EXPORT.h"

enum LOADER_TYPE
{
	LOADER_TYPE_UNKNOWN = 0,
	LOADER_TYPE_IMAGE,
};


class ENGINE_API IGResourceLoader
{
public:
	virtual ~IGResourceLoader() = default;

	virtual const char* get_resource_name() = 0;

	virtual LOADER_TYPE get_loader_type() = 0;
private:
};

#endif // IGRESOURCE_LOADER_H