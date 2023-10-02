#ifndef GCUBE_RENDERER_H
#define GCUBE_RENDERER_H

#include <string>
#include "engine/rendering/vulkan/ivulkan_descriptor_creator.h"
#include "public/core/templates/shared_ptr.h"
#include "public/math/gtransform.h"
#include "engine/manager/igpipeline_object_manager.h"
#include "engine/rendering/vulkan/named/igvulkan_named_graphic_pipeline.h"

class IGVulkanLogicalDevice;
class IGResourceManager;
class IGTextureResource;
class IGVulkanDescriptorPool;
class GCubePipelinelayoutCreator;
class IGCameraManager;
class IGVulkanViewport;
class IGVulkanShaderStage;
class IGShaderResource;
class IVulkanShaderStage;
class IGShaderManager;
class IGVulkanGraphicPipeline;
class GVulkanCommandBuffer;
class IGVulkanViewport;
class IGVulkanNamedPipelineLayout;

#include "engine/manager/igscene_manager.h"
#include "engine/rendering/vulkan/named/igvulkan_named_viewport.h"
#include "engine/rendering/vulkan/named/igvulkan_named_renderpass.h"
class GCubeRenderer
{
public:
	GCubeRenderer(IGVulkanLogicalDevice*  boundedDevice,IGResourceManager* mng, IGCameraManager* cameraManager, IGSceneManager* sceneManager,
		IGPipelineObjectManager* obj ,IGVulkanNamedViewport* viewport, IGShaderManager* shaderMng,IGVulkanNamedRenderPass* renderpass,uint32_t framesInFlight,const char* cubeTexturePath);

	bool init();
	
	void destroy();
	
	void render(GVulkanCommandBuffer* buff,uint32_t frameIndex, IGVulkanNamedViewport* vp);
private:
	uint32_t m_framesInFlight;
	GSharedPtr<IGTextureResource> m_cubemapTextureResource;
	GSharedPtr<IGShaderResource> m_cubemapVertexShader;
	GSharedPtr<IGShaderResource> m_cubemapFragShader;

	IVulkanShaderStage* m_cubemapVertexStage;
	IVulkanShaderStage* m_cubemapFragStage;
	IGSceneManager* p_sceneManager;
	IGVulkanLogicalDevice* m_boundedDevice;
	IGCameraManager* p_cameraManager;
	
	IGVulkanNamedPipelineLayout* m_pipeLayout;
	IGVulkanNamedSetLayout* m_csLayout;

	VkDescriptorSet_T* m_csSet;

	IGVulkanDescriptorPool* m_csPool;

	IGShaderManager* m_shaderManager;
	IGVulkanNamedViewport* m_viewport;
	IGVulkanNamedRenderPass* m_renderpass;
	IGVulkanNamedGraphicPipeline* m_pipeline;
	IGPipelineObjectManager* m_obj;
	gtransform cubeTransform;
};


#endif // GCUBE_RENDERER_H