#ifndef GRESOURCE_MANAGER_H
#define GRESOURCE_MANAGER_H


enum RESOURCE_ERROR
{
	RESOURCE_ERROR_UNKNOWN = 0,
	RESOURCE_ERROR_NAME_OR_GROUP_NAME_IS_NULL,
	RESOURCE_ERROR_NAME_ALREADY_IN_USE,
};

#include "engine/resource/iresource.h"
#include "public/core/templates/shared_ptr.h"
#include "engine/io/iowning_glogger.h"
#include "public/core/templates/unordered_dense.h"
#include "engine/manager/igresource_manager.h"

#include <expected>


class GResource;

class GResourceManager : public IGResourceManager
{
public:
	GResourceManager();
	~GResourceManager();

	//X TODO IResourceImpl must comes with deleter
	virtual std::expected<GResource*, RESOURCE_ERROR> create_resource(std::string_view name,std::string_view groupName) override;

	bool init();

	void destroy();
private:
	ankerl::unordered_dense::map<std::string, GResourcePtr> m_resourceMap;
	std::mutex m_resourceMutex;
	GSharedPtr<IOwningGLogger> m_logger;
};

#endif // GRESOURCE_MANAGER_H