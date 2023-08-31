#ifndef GSPIRV_SHADER_DEBUGGER_H
#define GSPIRV_SHADER_DEBUGGER_H


#include <glslang/Include/glslang_c_interface.h>
#include <spirv_cross/spirv_cross.hpp>
#include <memory>
#include "engine/shader/ispirv_shader.h"

class GSpirvShaderDebugger
{
public:
	GSpirvShaderDebugger();
	GSpirvShaderDebugger(uint32_t* words,uint32_t wordCount,SPIRV_SHADER_STAGE stage);
	~GSpirvShaderDebugger();
	bool is_valid() const noexcept;

	void load();

	std::string get_all_uniform_buffers();
private:
	bool m_isValid;
	uint32_t m_wordCount;
	uint32_t* m_words;
	std::unique_ptr<spirv_cross::Compiler> m_compiler;
	spirv_cross::ShaderResources m_shaderResources;
	spv::ExecutionModel m_executionModel;
	SPIRV_SHADER_STAGE m_stage;
};


#endif // GSPIRV_SHADER_DEBUGGER_H