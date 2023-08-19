#include "internal/engine/manager/gresource_manager.h"
#include "internal/engine/manager/glogger_manager.h"
#include <spdlog/fmt/fmt.h>

GResourceManager::GResourceManager()
{
}

GResourceManager::~GResourceManager()
{
}

std::expected<GResource*, RESOURCE_ERROR> GResourceManager::create_resource(std::string_view name, std::string_view groupName)
{
	if (name.empty() || groupName.empty())
		return std::unexpected(RESOURCE_ERROR::RESOURCE_ERROR_NAME_OR_GROUP_NAME_IS_NULL);
	//
	//std::string nameCopy(name);
	//GResourcePtr resource;
	//

	//{
	//	std::lock_guard guard(m_resourceMutex);
	//	if (auto res = m_resourceMap.find(nameCopy); res != m_resourceMap.end())
	//	{
	//		return std::unexpected(RESOURCE_ERROR::RESOURCE_ERROR_NAME_ALREADY_IN_USE);
	//	}

	//	m_logger->log_d(fmt::format("A resource created by name : [{}]{}",groupName,name).c_str());
	//	//X TODO GDNEWD  POOL 
	//	resource = GResourcePtr(new GResource(std::move(resourceImpl)));
	//	m_resourceMap.emplace(nameCopy, resource);
	//}

	//// Initialize info

	//resource->m_creatorOwner = this;
	//resource->m_creator = nullptr;
	//resource->m_identifierName = name;
	//resource->m_isCreatedByExternal = false;
	//

	return std::unexpected(RESOURCE_ERROR::RESOURCE_ERROR_NAME_OR_GROUP_NAME_IS_NULL);

}

bool GResourceManager::init()
{
	m_logger = GLoggerManager::get_instance()->create_owning_glogger("GResourceManager");

	return true;
}

void GResourceManager::destroy()
{
}

