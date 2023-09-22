#ifndef GMATERIAL_H
#define GMATERIAL_H

#include "engine/rendering/gputypes.h"
#include "engine/GEngine_EXPORT.H"
#include <string_view>
#include <vector>

enum MATERIAL_FLAG
{
	MATERIAL_FLAG_CAST_SHADOW = 0x1,
	MATERIAL_FLAG_RECEIVE_SHADOW = 0x2,
	MATERIAL_FLAG_TRANSPARENT = 0x4,
};

enum MATERIAL_DECODE_RESULT
{
	MATERIAL_DECODE_RESULT_OK,
	MATERIAL_DECODE_RESULT_UNKNOWN_ERROR
};

struct PACKED_STRUCT MaterialDescription final
{
	inline constexpr static const std::string_view EXTENSION_NAME = ".gmaterial";

	GPUTypes::gpuvec4 emissiveColor_ = { 0.0f, 0.0f, 0.0f, 0.0f };
	GPUTypes::gpuvec4 albedoColor_ = { 1.0f, 1.0f, 1.0f, 1.0f };

	float occlusionFactor_ = 0.f;
	float roughnessFactor_ = 0.f;
	float metallicFactor_ = 0.0f;

	float transparencyFactor_ = 1.0f;
	float alphaTest_ = 0.0f;

	uint32_t flags_ = MATERIAL_FLAG_CAST_SHADOW | MATERIAL_FLAG_RECEIVE_SHADOW;
	uint32_t ambientOcclusionMap_ = GPUTypes::INVALID_TEXTURE;
	uint32_t emissiveMap_ = GPUTypes::INVALID_TEXTURE;
	uint32_t albedoMap_ = GPUTypes::INVALID_TEXTURE;

	/// Occlusion (R), Roughness (G), Metallic (B) https://github.com/KhronosGroup/glTF/issues/857
	uint32_t metallicRoughnessMap_ = GPUTypes::INVALID_TEXTURE;
	uint32_t normalMap_ = GPUTypes::INVALID_TEXTURE;
	uint32_t opacityMap_ = GPUTypes::INVALID_TEXTURE;


	ENGINE_API static bool save_materials(const std::vector<MaterialDescription>& materials,const std::vector<std::string>& textureFileNames,const char* path);
	ENGINE_API static MATERIAL_DECODE_RESULT load_materials(const char* path,std::vector<MaterialDescription>& materials,std::vector<std::string>& textureFileNames);
};
#endif // GMATERIAL_H