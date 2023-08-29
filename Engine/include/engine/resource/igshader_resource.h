#ifndef IGSHADER_RESOURCE_H
#define IGSHADER_RESOURCE_H

#include "engine/resource/iresource.h"

class ISpirvShader;

class ENGINE_API IGShaderResource : public IResource
{
public:
	virtual ~IGShaderResource() = default;

	// After loading is succees it returns a valid SpirvShader pointer
	virtual ISpirvShader* get_shader() = 0;
private:
};

#endif // IGSHADER_RESOURCE_H