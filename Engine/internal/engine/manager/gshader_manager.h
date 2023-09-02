#ifndef GSHADER_MANAGER_H
#define GSHADER_MANAGER_H

#include "engine/manager/igshader_manager.h"
#include <string>

struct glslang_resource_s;


class GShaderManager : public IGShaderManager
{
public:
	virtual std::expected<ISpirvShader*, SHADER_COMPILE_ERROR> compile_shader_text(const std::string& text,SPIRV_SHADER_STAGE stage,SPIRV_SOURCE_TYPE sourceType) override;

	virtual std::expected<ISpirvShader*, SHADER_LOAD_ERROR> load_shader_from_bytes(const std::vector<char>& bytes, SPIRV_SHADER_STAGE stage) override;

	virtual std::expected<IVulkanShaderStage*, SHADER_STAGE_CREATE_ERROR> create_shader_stage_from_shader_res(GSharedPtr<IGShaderResource> shaderRes) override;
	
	virtual std::expected<IGVulkanShaderInfo*, SHADER_LAYOUT_BINDING_ERROR> get_layout_bindings_from(ISpirvShader* shaderHandle) override;

	virtual bool init() override;
private:
	uint64_t m_targetClientVersion;

	void set_to_defaults(glslang_resource_s* res);
};

#endif // GSHADER_MANAGER_H