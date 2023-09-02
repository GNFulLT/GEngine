#include "internal/engine/manager/gshader_manager.h"
#include <glslang_c_interface.h>
#include "internal/engine/shader/spirv_shader_utils.h"
#include "engine/gengine.h"
#include "engine/rendering/vulkan/ivulkan_app.h"
#include <glslang/Public/resource_limits_c.h>
#include "internal/engine/shader/gspirv_shader.h"
#include "spirv-tools/libspirv.h"
#include "internal/engine/shader/gspirv_byte_shader.h"
#include "internal/engine/rendering/vulkan/gvulkan_shader_stage.h"
#include "internal/engine/shader/spirv_parser.h"

std::expected<ISpirvShader*, SHADER_COMPILE_ERROR> GShaderManager::compile_shader_text(const std::string& text, SPIRV_SHADER_STAGE stage, SPIRV_SOURCE_TYPE sourceType)
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

		glslang_shader_delete(shader);
		return std::unexpected(SHADER_COMPILE_ERROR_PREPROCESS);
	}

	if (!glslang_shader_parse(shader, &input)) {
		glslang_shader_delete(shader);
		return std::unexpected(SHADER_COMPILE_ERROR_SHADER_PARSE);
	}

	glslang_program_t* program = glslang_program_create();
	glslang_program_add_shader(program, shader);

	auto msgs = GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT;

	if (!glslang_program_link(program, msgs))
	{
		glslang_program_delete(program);
		glslang_shader_delete(shader);
		return std::unexpected(SHADER_COMPILE_ERROR_PROGRAM_LINK);
	}
	
	glslang_program_SPIRV_generate(program, glslang_stage);

	// Now create the GSPIRVSHADER
	//X TODO : GDNWEDA

	auto size = glslang_program_SPIRV_get_size(program);
	auto words = (uint32_t*)malloc(size * sizeof(uint32_t));

	glslang_program_SPIRV_get(program, words);

	GSpirvShader* spirvShader = new GSpirvShader(size, words, input);
	
	//X TODO : CAN GET SPIRV OUTPUT MESSAGES

	glslang_program_delete(program);
	glslang_shader_delete(shader);

	glslang_finalize_process();

	return spirvShader;
}
	
std::expected<IVulkanShaderStage*, SHADER_STAGE_CREATE_ERROR> GShaderManager::create_shader_stage_from_shader_res(GSharedPtr<IGShaderResource> shaderRes)
{
	return new GVulkanShaderStage(shaderRes);
}

std::expected<std::vector<VkDescriptorSetLayoutBinding>, SHADER_LAYOUT_BINDING_ERROR> GShaderManager::get_layout_bindings_from(ISpirvShader* shaderHandle)
{
	std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
	auto res = get_bindings_from_binary(shaderHandle->get_spirv_words(), shaderHandle->get_size(), layoutBindings);
	if (res != SPIRV_READ_BINARY_RESULT_OK)
	{
		return std::unexpected(SHADER_LAYOUT_BINDING_ERROR_UNKNOWN);
	}

	return layoutBindings;
}

bool GShaderManager::init()
{
	auto info = GEngine::get_instance()->get_app()->get_version_info();

	long value = (long)vulkan_version_to_glslang_version(info->vulkanVersion);
	if (value == -1)
	{
		return false;
	}

	m_targetClientVersion = value;

	return true;

}

std::expected<ISpirvShader*, SHADER_LOAD_ERROR> GShaderManager::load_shader_from_bytes(const std::vector<char>& bytes, SPIRV_SHADER_STAGE stage)
{
	//X First check the bytes are corrupted
	spv_context context = spvContextCreate(SPV_ENV_UNIVERSAL_1_5);
	spv_const_binary_t binary = { (uint32_t*)bytes.data(), bytes.size() };
	spv_result_t result = spvValidate(context, &binary, nullptr);

	if (result != SPV_SUCCESS) {
		return std::unexpected(SHADER_LOAD_ERROR_CORRUPTED_SPIRV);
	}	
	spvContextDestroy(context);
	
	return new GSpirvByteShader(bytes,stage);
}

void GShaderManager::set_to_defaults(glslang_resource_s* res)
{	
	res->max_lights = 32;
	res->max_clip_planes = 6;
	res->max_texture_units = 32;
	res->max_texture_coords = 32;
	res->max_vertex_attribs = 64;
	res->max_vertex_uniform_components = 4096;
	res->max_varying_floats = 64;
	res->max_vertex_texture_image_units = 32;
	res->max_combined_texture_image_units = 80;
	res->max_texture_image_units = 32;
	res->max_fragment_uniform_components = 4096;
	res->max_draw_buffers = 32;
	res->max_vertex_uniform_vectors = 128;
	res->max_varying_vectors = 8;
	res->max_fragment_uniform_vectors = 16;
	res->max_vertex_output_vectors = 16;
	res->max_fragment_input_vectors = 15;
	res->min_program_texel_offset = -8;
	res->max_program_texel_offset = 7;
	res->max_clip_distances = 8;
	res->max_compute_work_group_count_x = 65535;
	res->max_compute_work_group_count_y = 65535;
	res->max_compute_work_group_count_z = 65535;
	res->max_compute_work_group_size_x = 1024;
	res->max_compute_work_group_size_y = 1024;
	res->max_compute_work_group_size_z = 64;
	res->max_compute_uniform_components = 1024;
	res->max_compute_texture_image_units = 16;
	res->max_compute_image_uniforms = 8;
	res->max_compute_atomic_counters = 8;
	res->max_compute_atomic_counter_buffers = 1;
	res->max_varying_components = 60;
	res->max_vertex_output_components = 64;
	res->max_geometry_input_components = 64;
	res->max_geometry_output_components = 128;
	res->max_fragment_input_components = 128;
	res->max_image_units = 8;
	res->max_combined_image_units_and_fragment_outputs = 8;
	res->max_image_samples = 0;
	res->max_vertex_image_uniforms = 0;
	res->max_tess_control_image_uniforms = 0;
	res->max_tess_evaluation_image_uniforms = 0;
	res->max_geometry_image_uniforms = 0;
	res->max_fragment_image_uniforms = 8;
	res->max_combined_image_uniforms = 8;
	res->max_geometry_texture_image_units = 16;
	res->max_geometry_output_vertices = 256;
	res->max_geometry_total_output_components = 1024;
	res->max_geometry_uniform_components = 1024;
	res->max_geometry_varying_components = 64;
	res->max_tess_control_input_components = 128;
	res->max_tess_control_output_components = 128;
	res->max_tess_control_texture_image_units = 16;
	res->max_tess_control_uniform_components = 1024;
	res->max_tess_control_total_output_components = 4096;
	res->max_tess_evaluation_input_components = 128;
	res->max_tess_evaluation_output_components = 128;
	res->max_tess_evaluation_texture_image_units = 16;
	res->max_tess_evaluation_uniform_components = 1024;
	res->max_tess_patch_components = 120;
	res->max_patch_vertices = 32;
	res->max_tess_gen_level = 64;
	res->max_viewports = 16;
	res->max_vertex_atomic_counters = 0;
	res->max_tess_control_atomic_counters = 0;
	res->max_tess_evaluation_atomic_counters = 0;
	res->max_geometry_atomic_counters = 0;
	res->max_fragment_atomic_counters = 8;
	res->max_combined_atomic_counters = 8;
	res->max_atomic_counter_bindings = 1;
	res->max_vertex_atomic_counter_buffers = 0;
	res->max_tess_control_atomic_counter_buffers = 0;
	res->max_tess_evaluation_atomic_counter_buffers = 0;
	res->max_geometry_atomic_counter_buffers = 0;
	res->max_fragment_atomic_counter_buffers = 1;
	res->max_combined_atomic_counter_buffers = 1;
	res->max_atomic_counter_buffer_size = 16384;
	res->max_transform_feedback_buffers = 4;
	res->max_transform_feedback_interleaved_components = 64;
	res->max_cull_distances = 8;
	res->max_combined_clip_and_cull_distances = 8;
	res->max_samples = 4;
	res->max_mesh_output_vertices_nv = 256;
	res->max_mesh_output_primitives_nv = 512;
	res->max_mesh_work_group_size_x_nv = 32;
	res->max_mesh_work_group_size_y_nv = 1;
	res->max_mesh_work_group_size_z_nv = 1;
	res->max_task_work_group_size_x_nv = 32;
	res->max_task_work_group_size_y_nv = 1;
	res->max_task_work_group_size_z_nv = 1;
	res->max_mesh_view_count_nv = 4;
	res->limits.non_inductive_for_loops = 1;
	res->limits.while_loops = 1;
	res->limits.do_while_loops = 1;
	res->limits.general_uniform_indexing = 1;
	res->limits.general_attribute_matrix_vector_indexing = 1;
	res->limits.general_varying_indexing = 1;
	res->limits.general_sampler_indexing = 1;
	res->limits.general_variable_indexing = 1;
	res->limits.general_constant_matrix_vector_indexing = 1;

}
