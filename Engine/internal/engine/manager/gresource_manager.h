#ifndef GRESOURCE_MANAGER_H
#define GRESOURCE_MANAGER_H




#include "engine/resource/iresource.h"
#include "public/core/templates/shared_ptr.h"
#include "engine/io/iowning_glogger.h"
#include "public/core/templates/unordered_dense.h"
#include "engine/manager/igresource_manager.h"
#include "public/core/templates/unordered_dense.h"

#include <expected>
#include <vector>

class IGVulkanSamplerCreator;
class IImageLoader;

class IResource;
class IGVulkanDescriptorCreator;

class GResourceManager : public IGResourceManager
{
public:
	GResourceManager();
	~GResourceManager();

	//X TODO IResourceImpl must comes with deleter
	virtual std::expected<GResource*, RESOURCE_ERROR> create_resource(std::string_view name,std::string_view groupName) override;

	virtual std::expected<IGTextureResource*, RESOURCE_ERROR> create_texture_resource(std::string_view name, std::string_view groupName,std::string_view filePath, IGVulkanDescriptorCreator* descriptorCreator,int format = -1) override;

	virtual std::expected<IGTextureResource*, RESOURCE_ERROR> create_texture_resource(std::string_view name, std::string_view groupName, std::string_view filePath, IGVulkanDescriptorCreator* descriptorCreator, IImageLoader* customLoader, int format = -1) override;

	virtual std::expected<IGShaderResource*, RESOURCE_ERROR> create_shader_resource(std::string_view name, std::string_view groupName, std::string_view filePath) override;


	virtual void destroy_texture_resource(IGTextureResource* texture) override;

	virtual void destroy_shader_resource(IGShaderResource* shader) override;

	bool init();

	void destroy();

	virtual const std::vector<IImageLoader*>* get_imageloaders_by_loading_type(GIMAGETYPE type) override;

	virtual IImageLoader* get_imageloader_with_name(const char* name) override;
private:
	bool add_image_loader(IImageLoader* loader);
	
	ankerl::unordered_dense::map<std::string, GResourcePtr> m_resourceMap;
	std::mutex m_resourceMutex;
	GSharedPtr<IOwningGLogger> m_logger;

	std::unordered_map<std::string, IImageLoader*> m_imageLoadersMap;

	ankerl::unordered_dense::segmented_map<GIMAGETYPE, std::vector<IImageLoader*>> m_imageTypeLoader;
	
	std::vector<IGVulkanSamplerCreator*> m_samplerCreators;
	std::vector<IImageLoader*> m_imageLoaders;



	IGVulkanSamplerCreator* m_defaultSamplerCreator;
	IImageLoader* m_defaultImageLoader;



};

#endif // GRESOURCE_MANAGER_H