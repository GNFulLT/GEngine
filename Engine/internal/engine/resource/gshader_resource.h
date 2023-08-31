#ifndef GSHADER_RESOURCE_H
#define GSHADER_RESOURCE_H

#include "engine/resource/igshader_resource.h"

class IGVulkanLogicalDevice;
class GResourceManager;

class GShaderResource :public IGShaderResource
{
	friend GResourceManager;
public:
	GShaderResource(IGVulkanLogicalDevice* device,std::string_view path);
	// Inherited via IGShaderResource
	virtual RESOURCE_INIT_CODE prepare_impl() override;
	virtual void unprepare_impl() override;
	virtual RESOURCE_INIT_CODE load_impl() override;
	virtual void unload_impl() override;
	virtual std::uint64_t calculateSize() const override;
	virtual std::string_view get_resource_path() const override;
	virtual void destroy_impl() override;

	

	virtual VkShaderModule_T* get_vk_shader_module() override;

	virtual SPIRV_SHADER_STAGE get_shader_stage() override;
	virtual const VkPipelineShaderStageCreateInfo* get_creation_info() override;
private:
	VkPipelineShaderStageCreateInfo m_stageCreateInfo;
	std::string m_shaderText;
	VkShaderModule_T* m_vkShaderModule;
	std::string m_path;
	IGVulkanLogicalDevice* m_boundedDevice;
	SPIRV_SHADER_STAGE m_stage;
	
};

#endif // GSHADER_RESOURCE_H