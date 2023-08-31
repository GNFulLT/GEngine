#include "internal/manager/geditor_shader_manager.h"
#include <glslang/Public/resource_limits_c.h>
#include <glslang/Include/glslang_c_interface.h>
#include "internal/shader/spirv_shader_utils.h"
#include "editor/editor_application_impl.h"
#include "engine/gengine.h"
#include "engine/rendering/vulkan/ivulkan_app.h"
#include "internal/shader/editor_debuggable_spirv_shader.h"
#include "engine/io/iowning_glogger.h"
#include "spirv-tools/libspirv.h"
#include "internal/shader/gspirv_byte_shader.h"
#include <spdlog/fmt/fmt.h>
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <SPIRV/SpvTools.h>
#include "MachineIndependent/reflection.h"

std::expected<ISpirvShader*, SHADER_LOAD_ERROR> GEditorShaderManager::load_shader_from_bytes(const std::vector<char>& bytes, SPIRV_SHADER_STAGE stage)
{
	//X First check the bytes are corrupted
	spv_context context = spvContextCreate(SPV_ENV_UNIVERSAL_1_5);
	spv_const_binary_t binary = { (uint32_t*)bytes.data(), (bytes.size()/sizeof(uint32_t)) };
	spv_diagnostic diagnostic = nullptr;
	spv_result_t result = spvValidate(context, &binary, &diagnostic);

	if (result != SPV_SUCCESS) {
		
		EditorApplicationImpl::get_instance()->get_editor_log_window_logger()->log_e(fmt::format("Binary file is corrupted. Size was : {}",binary.wordCount).c_str());
		EditorApplicationImpl::get_instance()->get_editor_log_window_logger()->log_e(diagnostic->error);
		spvDiagnosticDestroy(diagnostic);
		return std::unexpected(SHADER_LOAD_ERROR_CORRUPTED_SPIRV);
	}
	EditorApplicationImpl::get_instance()->get_editor_log_window_logger()->log_d("Loaded spirv binary file checking for stage");
	
	spvContextDestroy(context);
	return new GSpirvByteShader(bytes, stage);
}

void GEditorShaderManager::editor_init()
{
	m_targetClientVersion = EditorApplicationImpl::get_instance()->m_engine->get_app()->get_version_info()->vulkanVersion;
}

void GEditorShaderManager::destroy()
{
	m_defaultShaderMng->get()->destroy();
}

std::expected<IVulkanShaderStage*, SHADER_STAGE_CREATE_ERROR> GEditorShaderManager::create_shader_stage_from_shader_res(GSharedPtr<IGShaderResource> shaderRes)
{
	return m_defaultShaderMng->get()->create_shader_stage_from_shader_res(shaderRes);
}

void GEditorShaderManager::set_default(GSharedPtr<IGShaderManager>* ptr)
{
	m_defaultShaderMng = ptr;
}

GEditorShaderManager::~GEditorShaderManager()
{
	assert(m_defaultShaderMng != nullptr);
	delete m_defaultShaderMng;
}

std::expected<ISpirvShader*, SHADER_COMPILE_ERROR> GEditorShaderManager::compile_shader_text(const std::string& text, SPIRV_SHADER_STAGE stage, SPIRV_SOURCE_TYPE sourceType)
{
	auto glslang_stage = spirv_shader_stage_to_glslang_stage(stage);

	const glslang_input_t input = {
		.language = spirv_source_type_to_glslang_source(sourceType),
		.stage = glslang_stage,
		.client = GLSLANG_CLIENT_VULKAN,
		.client_version = (glslang_target_client_version_t)m_targetClientVersion,
		.target_language = GLSLANG_TARGET_SPV,
		.target_language_version = GLSLANG_TARGET_SPV_1_5,
		.code = text.c_str(),
		.default_version = 100,
		.default_profile = GLSLANG_NO_PROFILE,
		.force_default_version_and_profile = false,
		.forward_compatible = false,
		.messages = GLSLANG_MSG_DEFAULT_BIT,
		.resource = glslang_default_resource()
	};

	glslang_initialize_process();

	glslang_shader_t* shader = glslang_shader_create(&input);
	// Preprocess 
	if (!glslang_shader_preprocess(shader, &input))
	{
		//X TODO CONSOLE SINK
		EditorApplicationImpl::get_instance()->get_editor_log_window_logger()->log_e(glslang_shader_get_info_log(shader)); ;
		EditorApplicationImpl::get_instance()->get_editor_log_window_logger()->log_e(glslang_shader_get_info_debug_log(shader)); 
		glslang_shader_get_info_debug_log(shader);
		glslang_shader_delete(shader);
		return std::unexpected(SHADER_COMPILE_ERROR_PREPROCESS);
	}
	
	EditorApplicationImpl::get_instance()->get_editor_log_window_logger()->log_d("Shader preprocessed");
	
	if (!glslang_shader_parse(shader, &input)) {
		EditorApplicationImpl::get_instance()->get_editor_log_window_logger()->log_e(glslang_shader_get_info_log(shader)); ;
		EditorApplicationImpl::get_instance()->get_editor_log_window_logger()->log_e(glslang_shader_get_info_debug_log(shader));
		glslang_shader_delete(shader);
		return std::unexpected(SHADER_COMPILE_ERROR_SHADER_PARSE);
	}

	EditorApplicationImpl::get_instance()->get_editor_log_window_logger()->log_d("Shader parsed");

	glslang_program_t* program = glslang_program_create();
	glslang_program_add_shader(program, shader);

	auto msgs = GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT;

	if (!glslang_program_link(program, msgs))
	{
		EditorApplicationImpl::get_instance()->get_editor_log_window_logger()->log_e(glslang_program_get_info_log(program));
		EditorApplicationImpl::get_instance()->get_editor_log_window_logger()->log_e(glslang_program_get_info_debug_log(program));
		glslang_program_delete(program);
		glslang_shader_delete(shader);
		return std::unexpected(SHADER_COMPILE_ERROR_PROGRAM_LINK);
	}

	glslang_program_SPIRV_generate(program, glslang_stage);
	EditorApplicationImpl::get_instance()->get_editor_log_window_logger()->log_d("Shader compile success");
	// Now create the GSPIRVSHADER
	//X TODO : GDNWEDA

	auto wordCount = glslang_program_SPIRV_get_size(program);
	auto words = (uint32_t*)malloc(wordCount * sizeof(uint32_t));

	glslang_program_SPIRV_get(program, words);

	EditorDebuggableSPIRVShader* spirvShader = new EditorDebuggableSPIRVShader(wordCount * sizeof(uint32_t), words, input);

	//X TODO : CAN GET SPIRV OUTPUT MESSAGES	

	glslang_program_delete(program);
	glslang_shader_delete(shader);


	 


	glslang_finalize_process();

	return spirvShader;
}

bool GEditorShaderManager::init()
{
	return true;
}