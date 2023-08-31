#ifndef GSPIRV_BYTE_SHADER_H
#define GSPIRV_BYTE_SHADER_H

#include "engine/shader/ispirv_shader.h"
#include <glslang_c_interface.h>
#include <vector>
#include "internal/shader/gspirv_shader_debugger.h"


class GSpirvByteShader : public ISpirvShader
{
public:
	GSpirvByteShader(const std::vector<char>& bytes, SPIRV_SHADER_STAGE stage);
	~GSpirvByteShader();
	// Inherited via ISpirvShader
	virtual SPIRV_SHADER_STAGE get_spirv_stage() override;
	virtual uint32_t get_size() override;
	virtual bool is_failed_to_compile() override;
	virtual uint32_t* get_spirv_words() override;

	bool is_debug_active() const noexcept;
	GSpirvShaderDebugger* get_debugger();
private:
	SPIRV_SHADER_STAGE m_stage;
	std::vector<char> m_bytes;
	GSpirvShaderDebugger m_debugger;

};
#endif //GSPIRV_BYTE_SHADER_H