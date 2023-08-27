#include "volk.h"

#include "internal/rendering/vulkan/gscene_renderer.h"
#include "engine/rendering/vulkan/ivulkan_viewport.h"
#include "engine/rendering/vulkan/ivulkan_device.h"
#include "engine/rendering/vulkan/vulkan_command_buffer.h"
#include "editor/editor_application_impl.h"
#include "engine/io/iowning_glogger.h"
#include "engine/rendering/vulkan/ivulkan_device.h"
#include "engine/rendering/vulkan/vulkan_memory.h"
static int ct = 0;

GSceneRenderer::GSceneRenderer(IGVulkanViewport* viewport,IGVulkanDevice* device)
{
	m_viewport = viewport;
	m_device = device;
	m_cmd = nullptr;
	m_semaphore = nullptr;
}

void GSceneRenderer::render_the_scene()
{
	m_device->add_wait_semaphore_for_this_frame(m_semaphore, (int)VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	
	auto vp = m_viewport;
	vp->begin_draw_cmd(m_cmd);


	vp->end_draw_cmd(m_cmd);

	auto cmd = m_cmd->get_handle();
	auto smph = m_semaphore->get_semaphore();
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo inf = {};
	inf.pNext = nullptr;
	inf.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	inf.commandBufferCount = 1;
	inf.pCommandBuffers = &cmd;
	inf.waitSemaphoreCount = 0;
	inf.signalSemaphoreCount = 1;
	inf.pSignalSemaphores = &smph;
	inf.pWaitDstStageMask = &waitStage;

	m_device->execute_cmd_from_main(m_cmd,&inf,nullptr);

}

bool GSceneRenderer::init()
{
	EditorApplicationImpl::get_instance()->get_editor_logger()->log_d("Create the main cmd");
	m_cmd = m_device->create_cmd_from_main_pool();

	m_semaphore = m_device->create_semaphore(false);
	return true;
}

void GSceneRenderer::destroy()
{
	m_device->destroy_semaphore(m_semaphore);
	m_device->destroy_cmd_main_pool(m_cmd);
	m_cmd = nullptr;
}

void GSceneRenderer::set_the_viewport(IGVulkanViewport* viewport)
{
	m_viewport = viewport;
	
}
