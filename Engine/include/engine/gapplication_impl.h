#ifndef GAPPLICATION_IMPL_H
#define GAPPLICATION_IMPL_H

#include "GEngine_EXPORT.h"
#include <cstdint>

class GEngine;
class IInjectManagerHelper;
class GVulkanCommandBuffer;

class ENGINE_API GApplicationImpl
{
public:
	virtual ~GApplicationImpl() = default;

	virtual bool before_update() = 0;

	virtual void update() = 0;

	virtual void after_update() = 0;

	virtual bool before_render() = 0;

	virtual void render() = 0;

	virtual void after_render() = 0;

	virtual bool init(GEngine* engine) = 0;

	virtual void destroy() = 0;

	virtual void inject_managers(IInjectManagerHelper* helper) {}

	virtual GVulkanCommandBuffer* begin_draw_scene(uint32_t frameIndex);

	virtual void end_draw_scene(GVulkanCommandBuffer* cmd,uint32_t frameIndex);

	virtual void post_render(GVulkanCommandBuffer* cmd, uint32_t frameIndex) {};
private:
};


#endif // GAPPLICATION_IMPL_H