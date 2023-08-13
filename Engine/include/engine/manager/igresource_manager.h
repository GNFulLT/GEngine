#ifndef IGRESOURCE_MANAGER_H
#define IGRESOURCE_MANAGER_H


#include "engine/GEngine_EXPORT.h"
#include "public/core/templates/shared_ptr.h"
#include "engine/resource/iresource_impl.h"

#include <string_view>
#include <expected>

class GResource;

typedef GSharedPtr<GResource, GSHARED_PTR_INTERNAL_MODE_THREAD_SAFE> GResourcePtr;

class ENGINE_API IGResourceManager
{
public:
	constexpr static std::string_view DEFAULT_RESOURCE_GROUP_NAME = "EngineResources";

	virtual ~IGResourceManager() = default;


	virtual std::expected<GResourcePtr, RESOURCE_ERROR> create_resource(std::string_view name, std::string_view groupName, ResourceImplUniquePtr resourceImpl) = 0;

private:
};
#endif // IGRESOURCE_MANAGER_H