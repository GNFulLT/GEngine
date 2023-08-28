#include "internal/engine/manager/gresource_manager.h"
#include "internal/engine/manager/glogger_manager.h"
#include <spdlog/fmt/fmt.h>
#include "internal/engine/resource/gtexture_resource.h"
#include "internal/engine/rendering/vulkan/vulkan_ldevice.h"
#include "internal/engine/io/stb_image_loader.h"
#include "internal/engine/rendering/vulkan/gvulkan_default_sampler_creator.h"
#include "engine/resource/igtexture_resource.h"

GResourceManager::GResourceManager()
{
	//X TODO : GDNEWDA
	m_defaultImageLoader = new STBImageLoader();
	m_defaultSamplerCreator = new GDefaultSamplerCreator();
	m_imageLoaders.push_back(m_defaultImageLoader);
	m_samplerCreators.push_back(m_defaultSamplerCreator);
	
}

GResourceManager::~GResourceManager()
{
	for (int i = 0; i < m_samplerCreators.size(); i++)
	{
		delete m_samplerCreators[i];
	}
	for (int i = 0; i < m_imageLoaders.size(); i++)
	{
		delete m_imageLoaders[i];
	}
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

std::expected<IGTextureResource*, RESOURCE_ERROR> GResourceManager::create_texture_resource(std::string_view name, std::string_view groupName,std::string_view filePath, IGVulkanDescriptorCreator* descriptorCreator,int format)
{
	if (name.empty() || groupName.empty())
		return std::unexpected(RESOURCE_ERROR::RESOURCE_ERROR_NAME_OR_GROUP_NAME_IS_NULL);

	m_logger->log_d(fmt::format("A Texture resource created by name : [{}]{}", groupName, name).c_str());
	GTextureResource* res; 
	if (format != -1)
	{
		res = new GTextureResource(filePath, m_defaultImageLoader, GVulkanLogicalDevice::get_instance(), m_defaultSamplerCreator, descriptorCreator,format);
	}
	else
	{
		res = new GTextureResource(filePath, m_defaultImageLoader, GVulkanLogicalDevice::get_instance(), m_defaultSamplerCreator, descriptorCreator);
	}
	res->m_creatorOwner = this;
	return res;
}

void GResourceManager::destroy_texture_resource(IGTextureResource* texture)
{
	
}

bool GResourceManager::init()
{
	m_logger = GLoggerManager::get_instance()->create_owning_glogger("GResourceManager");

	return true;
}

void GResourceManager::destroy()
{
}

