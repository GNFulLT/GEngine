#ifndef IGRESOURCE_MANAGER_H
#define IGRESOURCE_MANAGER_H


#include "engine/GEngine_EXPORT.h"
#include "public/core/templates/shared_ptr.h"

#include <string_view>
#include <expected>

class GResource;
class IResource;

typedef GSharedPtr<GResource, GSHARED_PTR_INTERNAL_MODE_THREAD_SAFE> GResourcePtr;
typedef GSharedPtr<IResource, GSHARED_PTR_INTERNAL_MODE_THREAD_SAFE> IResourcePtr;

class ENGINE_API IGResourceManager
{
public:
	constexpr static std::string_view DEFAULT_RESOURCE_GROUP_NAME = "EngineResources";

	virtual ~IGResourceManager() = default;


	virtual std::expected<GResource*, RESOURCE_ERROR> create_resource(std::string_view name, std::string_view groupName) = 0;

	virtual std::expected<IResourcePtr, RESOURCE_ERROR> create_texture_resource(std::string_view name, std::string_view groupName, std::string_view filePath) = 0;

private:
};
#endif // IGRESOURCE_MANAGER_H