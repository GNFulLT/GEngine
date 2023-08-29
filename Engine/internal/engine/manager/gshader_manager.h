#ifndef GSHADER_MANAGER_H
#define GSHADER_MANAGER_H

#include "engine/manager/igshader_manager.h"
#include <string>

struct glslang_resource_s;


class GShaderManager : public IGShaderManager
{
public:
	virtual std::expected<ISpirvShader*, SHADER_COMPILE_ERROR> compile_shader_text(const std::string& text,SPIRV_SHADER_STAGE stage,SPIRV_SOURCE_TYPE sourceType) override;

	bool init();
private:
	uint64_t m_targetClientVersion;

	void set_to_defaults(glslang_resource_s* res);
};

#endif // GSHADER_MANAGER_H