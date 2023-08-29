#ifndef GSHADER_RESOURCE_H
#define GSHADER_RESOURCE_H

#include "engine/resource/igshader_resource.h"

class GShaderResource :public IGShaderResource
{
public:
	GShaderResource(std::string_view path);
	// Inherited via IGShaderResource
	virtual RESOURCE_INIT_CODE prepare_impl() override;
	virtual void unprepare_impl() override;
	virtual RESOURCE_INIT_CODE load_impl() override;
	virtual void unload_impl() override;
	virtual std::uint64_t calculateSize() const override;
	virtual std::string_view get_resource_path() const override;
	virtual void destroy_impl() override;
	virtual ISpirvShader* get_shader() override;
private:
	std::string m_shaderText;

	std::string m_path;
};

#endif // GSHADER_RESOURCE_H