#ifndef ISPIRV_SHADER_H
#define ISPIRV_SHADER_H


#include "engine/GEngine_EXPORT.h"
#include <cstdint>

enum SPIRV_SHADER_STAGE
{
	SPIRV_SHADER_STAGE_UNKNOWN,
	SPIRV_SHADER_STAGE_VERTEX,
	SPIRV_SHADER_STAGE_FRAGMENT,
	SPIRV_SHADER_STAGE_GEOMETRY,
	SPIRV_SHADER_STAGE_COMPUTE,
	SPIRV_SHADER_STAGE_TESSCONTROL,
	SPIRV_SHADER_STAGE_TESSEVALUATION
};

enum SPIRV_SOURCE_TYPE
{
	SPIRV_SOURCE_TYPE_UNKNOWN,
	SPIRV_SOURCE_TYPE_GLSL,
	SPIRV_SOURCE_TYPE_HLSL
};


class ENGINE_API ISpirvShader
{
public:
	virtual ~ISpirvShader() = default;

	virtual SPIRV_SHADER_STAGE get_spirv_stage() = 0;

	virtual uint32_t get_size() = 0;

	virtual bool is_failed_to_compile() = 0;

	virtual uint32_t* get_spirv_words() = 0;
private:
};

#endif // ISPIRV_SHADER_H