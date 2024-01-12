#include "internal/engine/shader/gspirv_shader.h"
#include "internal/engine/shader/spirv_shader_utils.h"

GSpirvShader::GSpirvShader(uint32_t byteSize, uint32_t* words, const glslang_input_t& usedInput)
{
	m_byteSize = byteSize;
	m_words = words;
	m_input = usedInput;
	m_entryPointName = "main";
}

SPIRV_SHADER_STAGE GSpirvShader::get_spirv_stage()
{
	return glslang_stage_to_spirv_shader_stage(m_input.stage);
}

uint32_t* GSpirvShader::get_spirv_words()
{
	return m_words;
}

const char* GSpirvShader::get_entry_point_name()
{
	return m_entryPointName.c_str();
}

uint32_t GSpirvShader::get_size()
{
	return m_byteSize;
}

bool GSpirvShader::is_failed_to_compile()
{
	return true;
}
