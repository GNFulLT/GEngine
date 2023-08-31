#include "internal/shader/editor_debuggable_spirv_shader.h"
#include "internal/shader/spirv_shader_utils.h"
#include "engine/gengine.h"
#include "engine/io/iowning_glogger.h"
#include "editor/editor_application_impl.h"

EditorDebuggableSPIRVShader::EditorDebuggableSPIRVShader(uint32_t byteSize, uint32_t* words, const glslang_input_t& usedInput) : m_debugger(words, byteSize / sizeof(uint32_t), glslang_stage_to_spirv_shader_stage(m_usedInput.stage))
{
	m_byteSize = byteSize;
	m_words = words;
	m_usedInput = usedInput;
	m_entryPointName = "main";
	//X TODO CHECK COMPILER CTOR THROWS EXCEPTION

	m_debugger.load();	
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

GSpirvShaderDebugger* EditorDebuggableSPIRVShader::get_debugger()
{
	return &m_debugger;
}

bool EditorDebuggableSPIRVShader::is_debug_active() const noexcept
{
	return m_debugger.is_valid();
}

const char* EditorDebuggableSPIRVShader::get_entry_point_name()
{
	return m_entryPointName.c_str();
}
