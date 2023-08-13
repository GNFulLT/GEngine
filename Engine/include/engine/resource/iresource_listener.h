#ifndef IRESOURCE_LISTENER_H
#define IRESOURCE_LISTENER_H


#include "engine/GEngine_EXPORT.h"
#include "engine/resource/resource_init_code.h"

#include <string_view>

class IResource;

class ENGINE_API IResourceListener
{
public:
	virtual ~IResourceListener() = default;

	virtual void before_loading(IResource* res) = 0;

	virtual void after_loading(IResource* res) = 0;

	virtual void before_unloading(IResource* res) = 0;

	virtual void after_unloading(IResource* res) = 0;
private:
};


#endif // IRESOURCE_LISTENER_H