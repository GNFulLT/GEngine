#ifndef GCUBE_RENDERER_H
#define GCUBE_RENDERER_H

#include <string>
#include "engine/rendering/vulkan/ivulkan_descriptor_creator.h"
#include "public/core/templates/shared_ptr.h"
#include "public/math/gtransform.h"
#include "engine/manager/igpipeline_object_manager.h"

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

class GCubeRenderer
{
public:
	GCubeRenderer(IGVulkanLogicalDevice*  boundedDevice,IGResourceManager* mng, IGCameraManager* cameraManager,IGPipelineObjectManager* obj ,IGVulkanViewport* viewport, IGShaderManager* shaderMng,uint32_t framesInFlight,const char* cubeTexturePath);

	bool init();
	
	void destroy();
	
	void render(GVulkanCommandBuffer* buff,uint32_t frameIndex, IGVulkanViewport* vp);
private:
	uint32_t m_framesInFlight;
	GSharedPtr<IGTextureResource> m_cubemapTextureResource;
	GSharedPtr<IGShaderResource> m_cubemapVertexShader;
	GSharedPtr<IGShaderResource> m_cubemapFragShader;

	IVulkanShaderStage* m_cubemapVertexStage;
	IVulkanShaderStage* m_cubemapFragStage;

	IGVulkanLogicalDevice* m_boundedDevice;
	GCubePipelinelayoutCreator* m_pipelineCreator;
	IGCameraManager* p_cameraManager;
	
	IGShaderManager* m_shaderManager;
	IGVulkanViewport* m_viewport;

	IGVulkanGraphicPipeline* m_pipeline;
	IGPipelineObjectManager* m_obj;
	gtransform cubeTransform;
};


#endif // GCUBE_RENDERER_H