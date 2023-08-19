#ifndef IGTEXTURE_RESOURCE_H
#define IGTEXTURE_RESOURCE_H


#include "engine/resource/iresource.h"


class IImageLoader;

class ENGINE_API IGTextureResource : public IResource
{
public:
	virtual ~IGTextureResource() = default;


	// If system is ready for load operation. This method will be called and ask the resource if there is any internal things prepare them and if u are ready too
	// return true and load implementation will be started

	virtual RESOURCE_INIT_CODE prepare_impl() = 0;

	// Undo method for preparing. Will be called after unload.

	virtual void unprepare_impl() = 0;

	// Loads the resource

	virtual RESOURCE_INIT_CODE load_impl() = 0;

	// Unloads the resource

	virtual void unload_impl() = 0;

	// Will be called after load operation
	virtual std::uint64_t calculateSize() const = 0;

	virtual std::string_view get_resource_path() const = 0;

	IImageLoader* get_loader();

protected:
	IImageLoader* m_loader;
};

#endif // IGTEXTURE_RESOURCE_H