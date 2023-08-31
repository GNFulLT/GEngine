#ifndef IGSHADER_RESOURCE_H
#define IGSHADER_RESOURCE_H

#include "engine/resource/iresource.h"

enum SPIRV_SHADER_STAGE;
struct VkShaderModule_T;

class ENGINE_API IGShaderResource : public IResource
{
public:
	virtual ~IGShaderResource() = default;

	virtual VkShaderModule_T* get_vk_shader_module() = 0;
	
	virtual SPIRV_SHADER_STAGE get_shader_stage() = 0;

	virtual const char* get_entry_point_name() = 0;
};

#endif // IGSHADER_RESOURCE_H