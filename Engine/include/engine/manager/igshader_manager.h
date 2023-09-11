#ifndef IGSHADER_MANAGER_H
#define IGSHADER_MANAGER_H

#include "engine/GEngine_EXPORT.h"

#include "engine/shader/ispirv_shader.h"
#include <expected>
#include <string>
#include <vector>
#include "public/core/templates/shared_ptr.h"
#include <utility>

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

enum SHADER_STAGE_CREATE_ERROR
{
	SHADER_STAGE_CREATE_ERROR_UNKNOWN
};

enum SHADER_LAYOUT_BINDING_ERROR
{
	SHADER_LAYOUT_BINDING_ERROR_UNKNOWN
};

class IVulkanShaderStage;
class IGShaderResource;
class ISpirvShader;
struct VkDescriptorSetLayoutBinding;
class IGVulkanShaderInfo;

class ENGINE_API IGShaderManager
{
public:
	virtual ~IGShaderManager() = default;

	//X THESE ARE FOR INTERNAL USAGE USE RESOURCE MANAGER TO CREATE SHADER RESOURCE
	virtual std::expected<ISpirvShader*, SHADER_COMPILE_ERROR> compile_shader_text(const std::string& text, SPIRV_SHADER_STAGE stage, SPIRV_SOURCE_TYPE sourceType) = 0;

	//X Try to use this for creating shader resources. If you want to parse first use other and then use this
	virtual std::expected<ISpirvShader*, SHADER_LOAD_ERROR> load_shader_from_bytes(const std::vector<char>& bytes, SPIRV_SHADER_STAGE stage) = 0;

	virtual std::expected<IVulkanShaderStage*, SHADER_STAGE_CREATE_ERROR> create_shader_stage_from_shader_res(IGShaderResource* shaderRes) = 0;

	virtual std::expected<IGVulkanShaderInfo*, SHADER_LAYOUT_BINDING_ERROR> get_layout_bindings_from(ISpirvShader* shaderHandle) = 0;

	virtual bool init() = 0;

	virtual void destroy() {}
private:
};

#endif // IGSHADER_MANAGER_H