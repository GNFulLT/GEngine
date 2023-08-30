#include "internal/shader/editor_debuggable_spirv_shader.h"
#include "internal/shader/spirv_shader_utils.h"

EditorDebuggableSPIRVShader::EditorDebuggableSPIRVShader(uint32_t byteSize, uint32_t* words, const glslang_input_t& usedInput)
{
	m_byteSize = byteSize;
	m_words = words;
	m_usedInput = usedInput;
}

SPIRV_SHADER_STAGE EditorDebuggableSPIRVShader::get_spirv_stage()
{
	return glslang_stage_to_spirv_shader_stage(m_usedInput.stage);
}

uint32_t EditorDebuggableSPIRVShader::get_size()
{
	return m_byteSize;
}

bool EditorDebuggableSPIRVShader::is_failed_to_compile()
{
	return false;
}

uint32_t* EditorDebuggableSPIRVShader::get_spirv_words()
{
	return m_words;
}
