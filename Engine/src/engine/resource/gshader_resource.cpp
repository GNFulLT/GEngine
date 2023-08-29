#include "internal/engine/resource/gshader_resource.h"
#include "internal/engine/shader/spirv_shader_utils.h"
#include "internal/engine/manager/gshader_manager.h"
GShaderResource::GShaderResource(std::string_view path)
{
	m_path = path;
}

RESOURCE_INIT_CODE GShaderResource::prepare_impl()
{
	auto res = read_shader_file(m_path.c_str());
	if (!res.has_value())
	{
		return RESOURCE_INIT_CODE_UNKNOWN_EX;
	}
	m_shaderText = res.value();
	return RESOURCE_INIT_CODE_OK;
}

void GShaderResource::unprepare_impl()
{
	m_shaderText.clear();
}

RESOURCE_INIT_CODE GShaderResource::load_impl()
{
	auto stage = shader_stage_from_file_name(m_path.c_str());
	return RESOURCE_INIT_CODE_UNKNOWN_EX;
}

void GShaderResource::unload_impl()
{
}

std::uint64_t GShaderResource::calculateSize() const
{
	return std::uint64_t();
}

std::string_view GShaderResource::get_resource_path() const
{
	return std::string_view();
}

void GShaderResource::destroy_impl()
{
}

ISpirvShader* GShaderResource::get_shader()
{
	return nullptr;
}
