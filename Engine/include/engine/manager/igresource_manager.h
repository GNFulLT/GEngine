#ifndef IGRESOURCE_MANAGER_H
#define IGRESOURCE_MANAGER_H


#include "engine/GEngine_EXPORT.h"
#include "public/core/templates/shared_ptr.h"

#include <string_view>
#include <expected>

enum RESOURCE_ERROR
{
	RESOURCE_ERROR_UNKNOWN = 0,
	RESOURCE_ERROR_NAME_OR_GROUP_NAME_IS_NULL,
	RESOURCE_ERROR_NAME_ALREADY_IN_USE,
};

class GResource;
class IResource;
class IGTextureResource;
class IGVulkanDescriptorCreator;

typedef GSharedPtr<GResource, GSHARED_PTR_INTERNAL_MODE_THREAD_SAFE> GResourcePtr;
typedef GSharedPtr<IResource, GSHARED_PTR_INTERNAL_MODE_THREAD_SAFE> IResourcePtr;

class ENGINE_API IGResourceManager
{
public:
	constexpr static std::string_view DEFAULT_RESOURCE_GROUP_NAME = "EngineResources";

	virtual ~IGResourceManager() = default;


	virtual std::expected<GResource*, RESOURCE_ERROR> create_resource(std::string_view name, std::string_view groupName) = 0;

	virtual std::expected<IGTextureResource*, RESOURCE_ERROR> create_texture_resource(std::string_view name, std::string_view groupName, std::string_view filePath, IGVulkanDescriptorCreator* descriptorCreator,int format = -1) = 0;

	virtual void destroy_texture_resource(IGTextureResource* texture) = 0;
private:
};
#endif // IGRESOURCE_MANAGER_H