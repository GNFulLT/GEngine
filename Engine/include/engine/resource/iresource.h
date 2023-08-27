#ifndef IRESOURCE_H
#define IRESOURCE_H

#include "engine/GEngine_EXPORT.h"
#include "engine/resource/resource_init_code.h"
#include "engine/resource/iresource_listener.h"
#include <cstdint>
#include <string>
#include <string_view>
#include <mutex>
#include <atomic>
#include <vector>
#include <set>

enum RESOURCE_LOADING_STATE
{
	RESOURCE_LOADING_STATE_UNLOADED = 0,
	RESOURCE_LOADING_STATE_LOADING,
	RESOURCE_LOADING_STATE_LOADED,
	RESOURCE_LOADING_STATE_UNLOADING,
	RESOURCE_LOADING_STATE_PREPARING,
	RESOURCE_LOADING_STATE_UNPREPARING
};

class GResourceManager;
class IGResourceLoader;

class ENGINE_API IResource
{
public:
	virtual ~IResource() = default;
	

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

	void bind_listener(IResourceListener* listener);

	void unbind_listener(IResourceListener* listener);

	bool before_load();

	RESOURCE_INIT_CODE load();

	void after_load();

	void before_unload();

	void unload();

	void after_unload();

	bool prepare();

	void unprepare();

	virtual std::string_view get_resource_path() const = 0;

protected:
	// Created by :
	GResourceManager* m_creatorOwner;

	IGResourceLoader* m_creator;

	// Unique name for this resource
	std::string m_identifierName;

	// Belonging group name (Editor, System, Game, Etc...)
	std::string m_resourceGroupName;

	std::atomic<RESOURCE_LOADING_STATE> m_loadingState;

	// Created by custom loader ( PLUGIN, SCRIPT ETC...)
	bool m_isCreatedByExternal;

	mutable std::uint64_t m_size;

	std::set<IResourceListener*> m_listenerSet;
	std::vector<IResourceListener*> m_listeners;


	std::mutex m_listenerMutex;
};


#endif // IRESOURCE_H