#include "internal/manager/geditor_texture_debug_manager.h"
#include "editor/editor_application_impl.h"
#include "internal/rendering/vulkan/gimgui_descriptor_creator.h"
#include "engine/gengine.h"
#include "engine/imanager_table.h"
#include "engine/manager/igpipeline_object_manager.h"
#include "engine/rendering/vulkan/ivulkan_descriptor.h"
bool GEditorTextureDebugManager::init()
{
	auto pipe = ((GSharedPtr<IGPipelineObjectManager>*)EditorApplicationImpl::get_instance()->m_engine->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_PIPELINE_OBJECT))->get();
	m_selectedSampler =	pipe->get_named_sampler(IGPipelineObjectManager::MAX_PERFORMANT_SAMPLER.data())->get_vk_sampler();
	return true;
}

VkDescriptorSet_T* GEditorTextureDebugManager::get_or_save_texture(IGTextureResource* texture) noexcept
{
	if (auto setIter = m_savedTextures.find(texture); setIter != m_savedTextures.end())
	{
		return setIter->second;
	}
	//X Texture is not saved create set for it 
	auto imguiCreator = EditorApplicationImpl::get_instance()->get_descriptor_creator();
	auto setRes = imguiCreator->create_descriptor_set_for_texture(texture->get_vulkan_image(), m_selectedSampler);
	auto set = setRes.value();
	auto handle = set->get_vk_descriptor();
	m_savedTextures.emplace(texture,handle);
	return handle;
}
