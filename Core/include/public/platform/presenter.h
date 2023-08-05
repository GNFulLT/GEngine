#ifndef PRESENTER_H
#define PRESENTER_H

#include "public/typedefs.h"
#include "public/core/templates/shared_ptr.fwd"

struct VulkanRenderpass;

class CORE_API Presenter
{
public:
	virtual ~Presenter() = default;
	
	const VulkanRenderpass* get_image_as_renderpass() = 0;
	
private:

};

#endif // PRESENTATION