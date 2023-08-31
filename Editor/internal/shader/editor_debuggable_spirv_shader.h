#ifndef EDITOR_DEBBUGGABLE_SPIRV_SHADER_H
#define EDITOR_DEBBUGGABLE_SPIRV_SHADER_H

#include "engine/shader/ispirv_shader.h"
#include <glslang/Include/glslang_c_interface.h>
#include <memory>
#include "internal/shader/gspirv_shader_debugger.h"

class EditorDebuggableSPIRVShader : public ISpirvShader
{
public:
	EditorDebuggableSPIRVShader(uint32_t byteSize, uint32_t* words, const glslang_input_t& usedInput);
	// Inherited via ISpirvShader
	virtual SPIRV_SHADER_STAGE get_spirv_stage() override;
	virtual uint32_t get_size() override;
	virtual bool is_failed_to_compile() override;
	virtual uint32_t* get_spirv_words() override;

	GSpirvShaderDebugger* get_debugger();

	bool is_debug_active() const noexcept;
private:
	uint32_t m_byteSize;
	uint32_t* m_words;
	glslang_input_t m_usedInput;
	GSpirvShaderDebugger m_debugger;

};


#endif // EDITOR_DEBBUGGABLE_SPIRV_SHADER_H