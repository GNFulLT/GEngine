#include "internal/shader/gspirv_byte_shader.h"
#include "internal/shader/spirv_shader_utils.h"

GSpirvByteShader::GSpirvByteShader(const std::vector<char>& bytes, SPIRV_SHADER_STAGE stage)
{
	m_bytes = bytes;
	m_stage = stage;

	m_debugger = GSpirvShaderDebugger((uint32_t*)bytes.data(), bytes.size() / sizeof(uint32_t),stage);

	m_debugger.load();
}

SPIRV_SHADER_STAGE GSpirvByteShader::get_spirv_stage()
{
	return m_stage;
}

uint32_t GSpirvByteShader::get_size()
{
	return m_bytes.size();
}

bool GSpirvByteShader::is_failed_to_compile()
{
	return false;
}

uint32_t* GSpirvByteShader::get_spirv_words()
{
	return (uint32_t*)m_bytes.data();
}

bool GSpirvByteShader::is_debug_active() const noexcept
{
	return m_debugger.is_valid();
}

GSpirvShaderDebugger* GSpirvByteShader::get_debugger()
{
	return &m_debugger;
}
