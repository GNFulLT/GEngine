#ifndef EDITOR_APPLICATION_IMPL
#define EDITOR_APPLICATION_IMPL

#include "engine/gapplication_impl.h"
#include "editor/GEngine_EXPORT.h"
#include "public/core/templates/shared_ptr.h"
#include <memory>
#include "engine/rendering/vulkan/named/igvulkan_named_viewport.h"
#include "engine/rendering/vulkan/named/viewports/igvulkan_named_deferred_viewport.h"
#include "engine/manager/igproject_manager.h"

class GProjectManager;
class GEngine;
class ImGuiLayer;
class IOwningGLogger;
class IGVulkanViewport;
class GImGuiDescriptorCreator;
class IGVulkanDescriptorPool;
struct VkDescriptorSetLayout_T;
class IGShaderResource;
class IVulkanShaderStage;
class IGVulkanGraphicPipeline;
class IGVulkanChainedViewport;
class GEditorFPSCameraPositioner;
class GEditorTextureDebugManager;

class EditorApplicationImpl : public GApplicationImpl
{
public:
	static EditorApplicationImpl* get_instance();
	
	virtual void destroy() override;
	
	virtual void inject_managers(IInjectManagerHelper* helper) override;

	virtual bool before_update() override;

	virtual void update() override;

	virtual void after_update() override;

	virtual bool before_render() override;

	virtual void render() override;

	virtual void after_render() override;

	virtual bool init(GEngine* engine) override;

	virtual GVulkanCommandBuffer* begin_draw_scene(uint32_t frameIndex) override;

	virtual void end_draw_scene(GVulkanCommandBuffer* cmd, uint32_t frameIndex) override;

	virtual void post_render(GVulkanCommandBuffer* cmd, uint32_t frameIndex) override;


	GEditorTextureDebugManager* get_texture_debug_manager();

	GEngine* m_engine;

	GSharedPtr<IOwningGLogger> get_editor_logger();

	GSharedPtr<IOwningGLogger> get_editor_log_window_logger();

	GImGuiDescriptorCreator* get_descriptor_creator();

	ImGuiLayer* get_editor_layer();

	uint32_t get_current_frame();

	uint32_t get_total_frame();

	IGProjectManager* get_project_manager();

	IGVulkanDescriptorSet* pbrPortSet = nullptr;
	IGVulkanDescriptorSet* normalPortSet = nullptr;
	IGVulkanDescriptorSet* positionPortSet = nullptr;
	IGVulkanDescriptorSet* albedoPortSet = nullptr;
	IGVulkanDescriptorSet* compositionPortSet = nullptr;
	IGVulkanDescriptorSet* sunShadowPortSet = nullptr;
	IGVulkanNamedDeferredViewport* m_renderViewport;

private:
	ImGuiLayer* m_imguiLayer;
	GImGuiDescriptorCreator* m_imguiDescriptorCreator;
	GSharedPtr<IOwningGLogger> m_logger;
	GSharedPtr<IOwningGLogger> m_logWindwLogger;
	inline static EditorApplicationImpl* s_instance;
	GEditorTextureDebugManager* m_textureDebugManager = nullptr;
	uint32_t m_currentFrame;
	uint32_t m_totalFrame;

	std::unique_ptr<GEditorFPSCameraPositioner> m_fpsCameraPositioner;


};


extern EDITOR_API GApplicationImpl* create_the_editor();
#endif // EDITOR_APPLICATION_IMPL