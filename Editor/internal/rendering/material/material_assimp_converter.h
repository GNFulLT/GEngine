#ifndef MATERIAL_ASSIMP_CONVERTER_H
#define MATERIAL_ASSIMP_CONVERTER_H

#include "engine/rendering/material/gmaterial.h"
#include <assimp/material.h>
#include <vector>
#include <unordered_map>

enum TEXTURE_MAP_TYPE;

class MaterialAssimpConvert
{
public:
	static void saveMaterials(const char* fileName, const std::vector<MaterialDescription>& materials, const std::vector<std::string>& files);
	static MaterialDescription aimaterial_to_gmaterial(const aiMaterial* m,std::vector<std::string>& files, std::vector<std::string>& opacityMaps);
	static MaterialDescription aimaterial_to_gmaterial(const aiMaterial* m, std::unordered_map<TEXTURE_MAP_TYPE,std::string>& textureFiles);

	static void convertAndDownscaleAllTextures(
		const std::vector<MaterialDescription>& materials, const std::string& basePath, std::vector<std::string>& files, std::vector<std::string>& opacityMaps
	);
private:
};

#endif // MATERIAL_ASSIMP_CONVERTER_H