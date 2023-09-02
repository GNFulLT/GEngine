#ifndef GEDITOR_SHADER_MANAGER_H
#define GEDITOR_SHADER_MANAGER_H

#include "engine/manager/igshader_manager.h"
#include "engine/shader/ispirv_shader.h"
#include <string>
#include "public/core/templates/shared_ptr.h"

class GEditorShaderManager : public IGShaderManager
{
public:
	void set_default(GSharedPtr<IGShaderManager>* ptr);
	~GEditorShaderManager();
	// Inherited via IGShaderManager
	virtual std::expected<ISpirvShader*, SHADER_COMPILE_ERROR> compile_shader_text(const std::string& text, SPIRV_SHADER_STAGE stage, SPIRV_SOURCE_TYPE sourceType) override;
	virtual bool init() override;
	virtual std::expected<ISpirvShader*, SHADER_LOAD_ERROR> load_shader_from_bytes(const std::vector<char>& bytes, SPIRV_SHADER_STAGE stage) override;
	void editor_init();

	// Inherited via IGShaderManager
	virtual std::expected<IVulkanShaderStage*, SHADER_STAGE_CREATE_ERROR> create_shader_stage_from_shader_res(GSharedPtr<IGShaderResource> shaderRes) override;
	// Inherited via IGShaderManager
	virtual std::expected<IGVulkanShaderInfo*, SHADER_LAYOUT_BINDING_ERROR> get_layout_bindings_from(ISpirvShader* shaderHandle) override;

	virtual void destroy() override;
private:
	uint32_t m_targetClientVersion;
	GSharedPtr<IGShaderManager>* m_defaultShaderMng;



};

#endif // GEDITOR_SHADER_MANAGER_H