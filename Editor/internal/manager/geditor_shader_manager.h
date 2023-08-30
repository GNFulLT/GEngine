#ifndef GEDITOR_SHADER_MANAGER_H
#define GEDITOR_SHADER_MANAGER_H

#include "engine/manager/igshader_manager.h"
#include "engine/shader/ispirv_shader.h"
#include <string>

class GEditorShaderManager : public IGShaderManager
{
public:

	// Inherited via IGShaderManager
	virtual std::expected<ISpirvShader*, SHADER_COMPILE_ERROR> compile_shader_text(const std::string& text, SPIRV_SHADER_STAGE stage, SPIRV_SOURCE_TYPE sourceType) override;
	virtual bool init() override;
	virtual std::expected<ISpirvShader*, SHADER_LOAD_ERROR> load_shader_from_bytes(const std::vector<char>& bytes, SPIRV_SHADER_STAGE stage) override;
	void editor_init();

private:
	uint32_t m_targetClientVersion;

};

#endif // GEDITOR_SHADER_MANAGER_H