#ifndef IGSHADER_MANAGER_H
#define IGSHADER_MANAGER_H

#include "engine/GEngine_EXPORT.h"

#include "engine/shader/ispirv_shader.h"
#include <expected>
#include <string>
#include <vector>

enum SHADER_COMPILE_ERROR
{
	SHADER_COMPILE_ERROR_UNKNOWN,
	SHADER_COMPILE_ERROR_PREPROCESS,
	SHADER_COMPILE_ERROR_SHADER_PARSE,
	SHADER_COMPILE_ERROR_PROGRAM_LINK
};

enum SHADER_LOAD_ERROR
{
	SHADER_LOAD_ERROR_UNKNOWN,
	SHADER_LOAD_ERROR_CORRUPTED_SPIRV,
	SHADER_LOAD_ERROR_SPIRV_STAGE_INCOMPATIBLITY
};



class ENGINE_API IGShaderManager
{
public:
	virtual ~IGShaderManager() = default;

	virtual std::expected<ISpirvShader*, SHADER_COMPILE_ERROR> compile_shader_text(const std::string& text, SPIRV_SHADER_STAGE stage, SPIRV_SOURCE_TYPE sourceType) = 0;

	//X Try to use this for creating shader resources. If you want to parse first use other and then use this
	virtual std::expected<ISpirvShader*, SHADER_LOAD_ERROR> load_shader_from_bytes(const std::vector<char>& bytes, SPIRV_SHADER_STAGE stage) = 0;

	virtual bool init() = 0;

	virtual void destroy() {}
private:
};

#endif // IGSHADER_MANAGER_H