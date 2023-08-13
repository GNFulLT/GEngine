#ifndef GRESOURCE_H
#define GRESOURCE_H

#include "engine/resource/iresource_impl.h"

#include <string_view>
#include <string>
#include <atomic>
#include <cstdint>
#include <set>
#include <vector>
#include <mutex>

#include "public/core/templates/shared_ptr.h"


class GResourceManager;
class GResourceLoader;

class GResource
{
	friend class GResourceManager;
	friend class GResourceLoader;
public:
	GResource(ResourceImplUniquePtr resource);


	void bind_listener(IResourceListener* listener);

	void unbind_listener(IResourceListener* listener);


	void before_load();

	RESOURCE_INIT_CODE load();

	void after_load();

	void before_unload();

	void unload();

	void after_unload();

	bool prepare();

	void unprepare();
	
	std::string_view get_resource_path() const;

protected:
	// Created by :
	GResourceManager* m_creatorOwner;

	GResourceLoader* m_creator;

	// Unique name for this resource
	std::string m_identifierName;

	// Belonging group name (Editor, System, Game, Etc...)
	std::string m_resourceGroupName;

	std::atomic<RESOURCE_LOADING_STATE> m_loadingState;

	// Created by custom loader ( PLUGIN, SCRIPT ETC...)
	bool m_isCreatedByExternal;

	std::uint64_t m_size;

	std::set<IResourceListener*> m_listenerSet;
	std::vector<IResourceListener*> m_listeners;


	std::mutex m_listenerMutex;

	ResourceImplUniquePtr m_implementer;
};

// Resource Pointers are thread safe
typedef GSharedPtr<GResource,GSHARED_PTR_INTERNAL_MODE_THREAD_SAFE> GResourcePtr;

#endif // GRESOURCE_H