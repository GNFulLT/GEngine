#include "internal/engine/manager/gresource_manager.h"
#include "internal/engine/manager/glogger_manager.h"
#include <spdlog/fmt/fmt.h>
#include "internal/engine/resource/gtexture_resource.h"
#include "internal/engine/rendering/vulkan/vulkan_ldevice.h"
#include "internal/engine/io/stb_image_loader.h"
#include "internal/engine/rendering/vulkan/gvulkan_default_sampler_creator.h"
#include "engine/resource/igtexture_resource.h"
#include "internal/engine/resource/gshader_resource.h"
#include "internal/engine/io/cube_image_loader.h"

GResourceManager::GResourceManager()
{
	//X TODO : GDNEWDA
	m_defaultImageLoader = new STBImageLoader();
	m_defaultSamplerCreator = new GDefaultSamplerCreator();
	m_imageLoaders.push_back(m_defaultImageLoader);
	m_samplerCreators.push_back(m_defaultSamplerCreator);
	auto cubemaploader = new CubemapImageLoader();
	assert(add_image_loader(cubemaploader));
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
std::expected<IGTextureResource*, RESOURCE_ERROR> GResourceManager::create_texture_resource(std::string_view name, std::string_view groupName, std::string_view filePath, IGVulkanDescriptorCreator* descriptorCreator, IImageLoader* customLoader,int format)
{
	if (name.empty() || groupName.empty())
		return std::unexpected(RESOURCE_ERROR::RESOURCE_ERROR_NAME_OR_GROUP_NAME_IS_NULL);

	m_logger->log_d(fmt::format("A Texture resource created by name : [{}]{}", groupName, name).c_str());
	GTextureResource* res;
	if (format != -1)
	{
		res = new GTextureResource(filePath, customLoader, GVulkanLogicalDevice::get_instance(),m_defaultSamplerCreator, descriptorCreator, format);
	}
	else
	{
		res = new GTextureResource(filePath, customLoader, GVulkanLogicalDevice::get_instance(), m_defaultSamplerCreator, descriptorCreator);
	}
	res->m_creatorOwner = this;
	return res;
}
std::expected<IGTextureResource*, RESOURCE_ERROR> GResourceManager::create_texture_resource(std::string_view name, std::string_view groupName,std::string_view filePath, IGVulkanDescriptorCreator* descriptorCreator,int format)
{
	return create_texture_resource(name, groupName, filePath, descriptorCreator, m_defaultImageLoader, format);
}

std::expected<IGShaderResource*, RESOURCE_ERROR> GResourceManager::create_shader_resource(std::string_view name, std::string_view groupName, std::string_view filePath)
{
	if (name.empty() || groupName.empty())
		return std::unexpected(RESOURCE_ERROR::RESOURCE_ERROR_NAME_OR_GROUP_NAME_IS_NULL);

	m_logger->log_d(fmt::format("A Shader resource created by name : [{}]{}", groupName, name).c_str());

	//X TODO GDNEWDA

	GShaderResource* res = new GShaderResource(GVulkanLogicalDevice::get_instance(),filePath);
	res->m_creatorOwner = this;

	return res;
	
}

void GResourceManager::destroy_texture_resource(IGTextureResource* texture)
{
	
}

void GResourceManager::destroy_shader_resource(IGShaderResource* shader)
{
}

bool GResourceManager::init()
{
	m_logger = GLoggerManager::get_instance()->create_owning_glogger("GResourceManager");

	return true;
}

void GResourceManager::destroy()
{
	for (int i = 0; i < m_imageLoaders.size();i++)
	{
		delete m_imageLoaders[i];
	}
	for (int i = 0; i < m_samplerCreators.size(); i++)
	{
		delete m_samplerCreators[i];
	}
}

IImageLoader* GResourceManager::get_imageloader_with_name(const char* name)
{
	if (auto imgLoader = m_imageLoadersMap.find(name); imgLoader != m_imageLoadersMap.end())
	{
		return imgLoader->second;
	}
	return nullptr;
}

bool GResourceManager::add_image_loader(IImageLoader* loader)
{
	auto name = std::string(loader->get_loader_name());
	if (auto imgLoader = m_imageLoadersMap.find(name); imgLoader != m_imageLoadersMap.end())
	{
		return false;
	}
	auto supportedTypes = loader->get_supported_image_types();
	if (supportedTypes != nullptr)
	{
		for (auto supportedType : *supportedTypes)
		{
			if (auto type = m_imageTypeLoader.find(supportedType); type == m_imageTypeLoader.end())
			{
				m_imageTypeLoader.emplace(supportedType, std::vector<IImageLoader*>());
			}

			m_imageTypeLoader[supportedType].push_back(loader);
		}
	}
	m_imageLoadersMap.emplace(name,loader);
	m_imageLoaders.push_back(loader);
	return true;
}

const std::vector<IImageLoader*>* GResourceManager::get_imageloaders_by_loading_type(GIMAGETYPE supportedType)
{
	if (auto type = m_imageTypeLoader.find(supportedType); type != m_imageTypeLoader.end())
	{
		return &type->second;
	}
	
	return nullptr;
}

