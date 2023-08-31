#include "internal/shader/gspirv_shader_debugger.h"
#include "editor/editor_application_impl.h"
#include "engine/gengine.h"
#include "engine/io/iowning_glogger.h"
#include "internal/shader/spirv_shader_utils.h"
#include <spdlog/fmt/fmt.h>

GSpirvShaderDebugger::GSpirvShaderDebugger()
{
	m_isValid = false;
	m_words = 0;
	m_wordCount = 0;
	m_shaderResources = {};
	m_stage = SPIRV_SHADER_STAGE_UNKNOWN;
	m_executionModel = spv::ExecutionModelMax;
}

GSpirvShaderDebugger::GSpirvShaderDebugger(uint32_t* words, uint32_t wordCount,SPIRV_SHADER_STAGE stage)
{
	m_isValid = false;
	m_words = words;
	m_wordCount = wordCount;
	m_shaderResources = {};
	m_stage = stage;
	m_executionModel = spriv_shader_stage_to_execution_model(stage);
}

bool GSpirvShaderDebugger::is_valid() const noexcept
{
	return m_isValid;
}

void GSpirvShaderDebugger::load()
{
	try
	{
		m_compiler = std::unique_ptr<spirv_cross::Compiler>(new spirv_cross::Compiler(m_words, m_wordCount));
		m_isValid = true;
		m_shaderResources = m_compiler->get_shader_resources();
	}
	catch (std::exception& ex)
	{
		m_isValid = false;
		EditorApplicationImpl::get_instance()->get_editor_log_window_logger()->log_e("Couldn't reflect the shader");
	}

}

std::string GSpirvShaderDebugger::get_all_uniform_buffers()
{
	auto uniforms = m_shaderResources.uniform_buffers;
	auto outs = m_shaderResources.stage_outputs;

	std::string debugStr;

	for (const auto& out : outs)
	{
		uint32_t binding = m_compiler->get_decoration(out.id, spv::DecorationLocation);
		debugStr += fmt::format("Out name : {}, Location to : {}", out.name.c_str(), binding);
	}
	for (const auto& uniform : uniforms)
	{
		uint32_t binding = m_compiler->get_decoration(uniform.id, spv::DecorationBinding);
		debugStr += fmt::format("Uniform name : {}, Binding to : {}", uniform.name.c_str(), binding);

	}
	
	return debugStr;
}
