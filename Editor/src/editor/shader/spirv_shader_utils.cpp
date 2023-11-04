#include "volk.h"
#include "internal/shader/spirv_shader_utils.h"
#include <fstream>

std::expected<std::vector<char>, READ_SHADER_FILE_ERROR> read_shader_bytes(std::filesystem::path fileName)
{

	std::ifstream stream(fileName, std::ios::ate | std::ios::binary);

	if (!stream.good())
	{
		return std::unexpected(READ_SHADER_FILE_ERROR_FILE_NOT_FOUND);
	}

	size_t fileSize = (size_t)stream.tellg();

	std::vector<char> bytesC(stream.tellg());

	stream.seekg(0);
	stream.read(bytesC.data(), fileSize);

	stream.close();

	return bytesC;

	//auto path = fileName.string();
	//FILE* file = fopen(path.c_str(), "r");

	//if (!file)
	//{
	//	return std::unexpected(READ_SHADER_FILE_ERROR_FILE_NOT_FOUND);
	//}

	//fseek(file, 0L, SEEK_END);
	//const auto bytesinfile = ftell(file);
	//fseek(file, 0L, SEEK_SET);

	//char* buffer = (char*)malloc(bytesinfile + 1);
	//
	//const size_t bytesread = fread(buffer, 1, bytesinfile, file);
	//
	//fclose(file);

	//std::vector<char> buff(bytesread);

	//for (int i = 0; i < bytesread; i++)
	//{
	//	buff[i] = *(buffer + i);
	//}

	//free(buffer);

	//return buff;
}

SPIRV_SHADER_STAGE get_stage_from_spirv_file_name(const char* fileName)
{
	int i = 0;
	int firstDot = -1;
	while (fileName[i] != '\0') {
		if (fileName[i] == '.') {
			firstDot = i;
			break;
		}
		i++;
	}
	if (firstDot == -1)
		return SPIRV_SHADER_STAGE_UNKNOWN;

	// Find the second dot

	int secondDot = -1;
	i++;
	while (fileName[i] != '\0')
	{
		if (fileName[i] == '.') {
			secondDot = i;
			break;
		}
		i++;
	}

	if (secondDot == -1)
		return SPIRV_SHADER_STAGE_UNKNOWN;

	// tt.frag.spv
	// 7 - 2 = 5
	int len = secondDot - firstDot;
	std::string ex(fileName, firstDot,len);

	if (strcmp(ex.data(), ".frag") == 0)
		return SPIRV_SHADER_STAGE_FRAGMENT;
	if (strcmp(ex.data(), ".vert") == 0)
		return SPIRV_SHADER_STAGE_VERTEX;
	if (strcmp(ex.data(), ".geom") == 0)
		return SPIRV_SHADER_STAGE_GEOMETRY;
	if (strcmp(ex.data(), ".comp") == 0)
		return SPIRV_SHADER_STAGE_COMPUTE;
	if (strcmp(ex.data(), ".tesc") == 0)
		return SPIRV_SHADER_STAGE_TESSCONTROL;
	if (strcmp(ex.data(), ".tese") == 0)
		return SPIRV_SHADER_STAGE_TESSEVALUATION;
	

	return SPIRV_SHADER_STAGE_UNKNOWN;
}

std::expected<std::string, READ_SHADER_FILE_ERROR> read_shader_file(const char* fileName)
{
	FILE* file = fopen(fileName, "r");

	if (!file)
	{
		printf("I/O error. Cannot open shader file '%s'\n", fileName);
		return std::string();
	}

	fseek(file, 0L, SEEK_END);
	const auto bytesinfile = ftell(file);
	fseek(file, 0L, SEEK_SET);

	char* buffer = (char*)malloc(bytesinfile + 1);
	const size_t bytesread = fread(buffer, 1, bytesinfile, file);
	fclose(file);

	buffer[bytesread] = 0;

	static constexpr unsigned char BOM[] = { 0xEF, 0xBB, 0xBF };

	if (bytesread > 3)
	{
		if (!memcmp(buffer, BOM, 3))
			memset(buffer, ' ', 3);
	}

	std::string code(buffer);
	
	delete buffer;

	while (code.find("#include ") != code.npos)
	{
		const auto pos = code.find("#include ");
		const auto p1 = code.find('<', pos);
		const auto p2 = code.find('>', pos);
		if (p1 == code.npos || p2 == code.npos || p2 <= p1)
		{
			printf("Error while loading shader program: %s\n", code.c_str());
			return std::string();
		}
		const std::string name = code.substr(p1 + 1, p2 - p1 - 1);
		auto res = read_shader_file(name.c_str());
		if (!res.has_value())
		{
			return std::unexpected(READ_SHADER_FILE_ERROR_INCLUDE_NOT_FOUND);
		}
		const std::string include = res.value();
		code.replace(pos, p2 - pos + 1, include.c_str());
	}

	return code;
}

std::pair<SPIRV_SHADER_STAGE, SPIRV_SOURCE_TYPE> shader_stage_from_file_name(const char* fileName)
{
	if (endsWith(fileName, ".hlsl_vert"))
		return { SPIRV_SHADER_STAGE_VERTEX,SPIRV_SOURCE_TYPE_HLSL };
	if (endsWith(fileName, ".glsl_vert"))
		return { SPIRV_SHADER_STAGE_VERTEX,SPIRV_SOURCE_TYPE_GLSL };

	if (endsWith(fileName, ".hlsl_frag"))
		return { SPIRV_SHADER_STAGE_FRAGMENT,SPIRV_SOURCE_TYPE_HLSL };
	if(endsWith(fileName, ".glsl_frag"))
		return { SPIRV_SHADER_STAGE_FRAGMENT,SPIRV_SOURCE_TYPE_GLSL };

	if (endsWith(fileName, ".hlsl_geom"))
		return { SPIRV_SHADER_STAGE_GEOMETRY,SPIRV_SOURCE_TYPE_HLSL };
	if (endsWith(fileName, ".glsl_geom"))
		return { SPIRV_SHADER_STAGE_GEOMETRY,SPIRV_SOURCE_TYPE_GLSL };

	if (endsWith(fileName, ".hlsl_comp"))
		return { SPIRV_SHADER_STAGE_COMPUTE,SPIRV_SOURCE_TYPE_HLSL };
	if(endsWith(fileName, ".glsl_comp"))
		return { SPIRV_SHADER_STAGE_COMPUTE,SPIRV_SOURCE_TYPE_HLSL };

	if (endsWith(fileName, ".hlsl_tesc"))
		return { SPIRV_SHADER_STAGE_TESSCONTROL,SPIRV_SOURCE_TYPE_HLSL };
	if(endsWith(fileName, ".glsl_tesc"))
		return { SPIRV_SHADER_STAGE_TESSCONTROL,SPIRV_SOURCE_TYPE_HLSL };

	if (endsWith(fileName, ".hlsl_tese"))
		return { SPIRV_SHADER_STAGE_TESSEVALUATION,SPIRV_SOURCE_TYPE_HLSL };
	if (endsWith(fileName, ".glsl_tese"))
		return { SPIRV_SHADER_STAGE_TESSEVALUATION,SPIRV_SOURCE_TYPE_GLSL };
	if (endsWith(fileName, ".glsl_task"))
		return { SPIRV_SHADER_STAGE_TASK,SPIRV_SOURCE_TYPE_GLSL };
	if (endsWith(fileName, ".glsl_mesh"))
		return { SPIRV_SHADER_STAGE_MESH,SPIRV_SOURCE_TYPE_GLSL };

	return { SPIRV_SHADER_STAGE_UNKNOWN,SPIRV_SOURCE_TYPE_UNKNOWN };
}

glslang_stage_t spirv_shader_stage_to_glslang_stage(SPIRV_SHADER_STAGE stage)
{
	switch (stage)
	{
	case SPIRV_SHADER_STAGE_UNKNOWN:
		return glslang_stage_t::GLSLANG_STAGE_VERTEX;
	case SPIRV_SHADER_STAGE_VERTEX:
		return GLSLANG_STAGE_VERTEX;
	case SPIRV_SHADER_STAGE_FRAGMENT:
		return GLSLANG_STAGE_FRAGMENT;
	case SPIRV_SHADER_STAGE_GEOMETRY:
		return GLSLANG_STAGE_GEOMETRY;
	case SPIRV_SHADER_STAGE_COMPUTE:
		return GLSLANG_STAGE_COMPUTE;
	case SPIRV_SHADER_STAGE_TESSCONTROL:
		return GLSLANG_STAGE_TESSCONTROL;
	case SPIRV_SHADER_STAGE_TESSEVALUATION:
		return GLSLANG_STAGE_TESSEVALUATION;
	case SPIRV_SHADER_STAGE_TASK:
		return GLSLANG_STAGE_TASK;
	case SPIRV_SHADER_STAGE_MESH:
		return GLSLANG_STAGE_MESH;
	default:
		return GLSLANG_STAGE_VERTEX;
	}
}

glslang_source_t spirv_source_type_to_glslang_source(SPIRV_SOURCE_TYPE type)
{
	switch (type)
	{
	case SPIRV_SOURCE_TYPE_UNKNOWN:
		return GLSLANG_SOURCE_NONE;
	case SPIRV_SOURCE_TYPE_GLSL:
		return GLSLANG_SOURCE_GLSL;
	case SPIRV_SOURCE_TYPE_HLSL:
		return GLSLANG_SOURCE_HLSL;
	default:
		return GLSLANG_SOURCE_NONE;
	}
}

SPIRV_SHADER_STAGE glslang_stage_to_spirv_shader_stage(glslang_stage_t stage)
{
	switch (stage)
	{
	case GLSLANG_STAGE_VERTEX:
		return SPIRV_SHADER_STAGE_VERTEX;
	case GLSLANG_STAGE_FRAGMENT:
		return SPIRV_SHADER_STAGE_FRAGMENT;
	case GLSLANG_STAGE_GEOMETRY:
		return SPIRV_SHADER_STAGE_GEOMETRY;
	case GLSLANG_STAGE_COMPUTE:
		return SPIRV_SHADER_STAGE_COMPUTE;
	case GLSLANG_STAGE_TESSCONTROL:
		return SPIRV_SHADER_STAGE_TESSCONTROL;
	case GLSLANG_STAGE_TESSEVALUATION:
		return SPIRV_SHADER_STAGE_TESSEVALUATION;
	case GLSLANG_STAGE_TASK:
		return SPIRV_SHADER_STAGE_TASK;
	case GLSLANG_STAGE_MESH:
		return SPIRV_SHADER_STAGE_MESH;
	default:
		return SPIRV_SHADER_STAGE_UNKNOWN;
	}
}

SPIRV_SOURCE_TYPE glslang_source_to_spirv_source_type(glslang_source_t type)
{
	switch (type)
	{
	case GLSLANG_SOURCE_GLSL:
		return SPIRV_SOURCE_TYPE_GLSL;
	case GLSLANG_SOURCE_HLSL:
		return SPIRV_SOURCE_TYPE_HLSL;
	default:
		return SPIRV_SOURCE_TYPE_UNKNOWN;
	}
}

glslang_target_client_version_t vulkan_version_to_glslang_version(uint32_t vers)
{
	switch (vers)
	{
	case VK_API_VERSION_1_0:
		return GLSLANG_TARGET_VULKAN_1_0;
	case VK_API_VERSION_1_1:
		return GLSLANG_TARGET_VULKAN_1_1;
	case VK_API_VERSION_1_2:
		return GLSLANG_TARGET_VULKAN_1_2;
	case VK_API_VERSION_1_3:
		return GLSLANG_TARGET_VULKAN_1_3;
	default:
		return (glslang_target_client_version_t)-1;
	}
}

std::string stage_to_extension(SPIRV_SHADER_STAGE stage)
{
	switch (stage)
	{
	case SPIRV_SHADER_STAGE_UNKNOWN:
		return "";
	case SPIRV_SHADER_STAGE_VERTEX:
		return ".vert";
	case SPIRV_SHADER_STAGE_FRAGMENT:
		return ".frag";
	case SPIRV_SHADER_STAGE_GEOMETRY:
		return ".geom";
	case SPIRV_SHADER_STAGE_COMPUTE:
		return ".comp";
	case SPIRV_SHADER_STAGE_TESSCONTROL:
		return ".tesc";
	case SPIRV_SHADER_STAGE_TESSEVALUATION:
		return "tese";
	default:
		return "";
	}
}

spv::ExecutionModel spriv_shader_stage_to_execution_model(SPIRV_SHADER_STAGE stage)
{
	switch (stage)
	{
	case SPIRV_SHADER_STAGE_UNKNOWN:
		return spv::ExecutionModelMax;
	case SPIRV_SHADER_STAGE_VERTEX:
		return spv::ExecutionModelVertex;
	case SPIRV_SHADER_STAGE_FRAGMENT:
		return spv::ExecutionModelFragment;
	case SPIRV_SHADER_STAGE_GEOMETRY:
		return spv::ExecutionModelGeometry;
	case SPIRV_SHADER_STAGE_COMPUTE:
		return spv::ExecutionModelGLCompute;
	case SPIRV_SHADER_STAGE_TESSCONTROL:
		return spv::ExecutionModelTessellationControl;
	case SPIRV_SHADER_STAGE_TESSEVALUATION:
		return spv::ExecutionModelTessellationEvaluation;
	default:
		return spv::ExecutionModelMax;
	}
}
