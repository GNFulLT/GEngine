#include "internal/engine/shader/gspirv_byte_shader.h"

GSpirvByteShader::GSpirvByteShader(const std::vector<char>& bytes, SPIRV_SHADER_STAGE stage)
{
	m_bytes = bytes;
	m_entryPointName = "main";
	m_stage = stage;
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


const char* GSpirvByteShader::get_entry_point_name()
{
	return m_entryPointName.c_str();
}
