#ifndef GEDITOR_TEXTURE_DEBUG_MANAGER_H
#define GEDITOR_TEXTURE_DEBUG_MANAGER_H

#include "public/core/templates/unordered_dense.h"
#include "engine/resource/igtexture_resource.h"

struct VkDescriptorSet_T;
struct VkSampler_T;

class GEditorTextureDebugManager
{
public:
	virtual bool init();
	virtual VkDescriptorSet_T* get_or_save_texture(IGTextureResource* texture) noexcept;
private:
	ankerl::unordered_dense::segmented_map<IGTextureResource*, VkDescriptorSet_T*> m_savedTextures;
	VkSampler_T* m_selectedSampler;
};

#endif // GEDITOR_TEXTURE_DEBUG_MANAGER_H