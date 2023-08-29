#ifndef IGSHADER_MANAGER_H
#define IGSHADER_MANAGER_H

#include "engine/GEngine_EXPORT.h"

#include "engine/shader/ispirv_shader.h"
#include <expected>
#include <string>

enum SHADER_COMPILE_ERROR
{
	SHADER_COMPILE_ERROR_UNKNOWN,
	SHADER_COMPILE_ERROR_PREPROCESS,
	SHADER_COMPILE_ERROR_SHADER_PARSE,
	SHADER_COMPILE_ERROR_PROGRAM_LINK
};

class ENGINE_API IGShaderManager
{
public:
	virtual ~IGShaderManager() = default;

	virtual std::expected<ISpirvShader*, SHADER_COMPILE_ERROR> compile_shader_text(const std::string& text, SPIRV_SHADER_STAGE stage, SPIRV_SOURCE_TYPE sourceType) = 0;
private:
};

#endif // IGSHADER_MANAGER_H