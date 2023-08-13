#ifndef IRESOURCE_H
#define IRESOURCE_H

#include "engine/GEngine_EXPORT.h"
#include "engine/resource/resource_init_code.h"
#include "engine/resource/iresource_listener.h"
#include <cstdint>
#include <memory>

enum RESOURCE_LOADING_STATE
{
	RESOURCE_LOADING_STATE_UNLOADED = 0,
	RESOURCE_LOADING_STATE_LOADING,
	RESOURCE_LOADING_STATE_LOADED,
	RESOURCE_LOADING_STATE_UNLOADING,
	RESOURCE_LOADING_STATE_PREPARING,
	RESOURCE_LOADING_STATE_UNPREPARING
};

class ENGINE_API IResourceImpl
{
public:
	virtual ~IResourceImpl() = default;
	

	// If system is ready for load operation. This method will be called and ask the resource if there is any internal things prepare them and if u are ready too
	// return true and load implementation will be started

	virtual bool prepare_impl() = 0;

	// Undo method for preparing. Will be called after unload.

	virtual void unprepare_impl() = 0;

	// Loads the resource

	virtual RESOURCE_INIT_CODE load_impl() = 0;

	// Unloads the resource
		
	virtual void unload_impl() = 0;

	// Will be called after load operation
	virtual std::uint64_t calculateSize() const = 0;

	virtual std::string_view get_resource_path() const = 0;
};


typedef std::unique_ptr<IResourceImpl> ResourceImplUniquePtr;

#endif // IRESOURCE_H