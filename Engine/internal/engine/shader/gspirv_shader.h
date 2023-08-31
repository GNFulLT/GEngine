#ifndef GSPIRV_SHADER_H
#define GSPIRV_SHADER_H

#include "engine/shader/ispirv_shader.h"
#include <glslang_c_interface.h>
#include <string>
class GShaderManager;

class GSpirvShader : public ISpirvShader
{
	friend class GShaderManager;
public:
	GSpirvShader(uint32_t byteSize, uint32_t* words,const glslang_input_t& usedInput);
	// Inherited via ISpirvShader
	virtual SPIRV_SHADER_STAGE get_spirv_stage() override;
	virtual uint32_t get_size() override;
	virtual bool is_failed_to_compile() override;
	virtual uint32_t* get_spirv_words() override;

	// Inherited via ISpirvShader
	virtual const char* get_entry_point_name() override;
private:
	glslang_input_t m_input;
	uint32_t m_byteSize;
	uint32_t* m_words;
	std::string m_entryPointName;


};

#endif // GSPIRV_SHADER_H