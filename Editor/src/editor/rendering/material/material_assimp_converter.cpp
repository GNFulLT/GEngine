#include "internal/rendering/material/material_assimp_converter.h"
#include <assimp/cimport.h>
#include <assimp/material.h>
#include <assimp/pbrmaterial.h>
#include <assimp/postprocess.h>
#include <algorithm>
#include <unordered_map>
#include <execution>
#include <filesystem>
#include <fstream>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "public/core/stb_image_write.h"
#include "public/core/stb_image.h"
#include "public/core/stb_image_resize.h"
#include "internal/rendering/scene/scene_converter.h"
#include <filesystem>

std::string lowercaseString(const std::string& s)
{
	std::string out(s.length(), ' ');
	std::transform(s.begin(), s.end(), out.begin(), tolower);
	return out;
}

std::string findSubstitute(const std::string& origFile)
{
	auto apath = std::filesystem::absolute(std::filesystem::path(origFile));
	auto afile = lowercaseString(apath.filename().string());
	auto dir = std::filesystem::path(origFile).remove_filename();

	for (auto& p : std::filesystem::directory_iterator(dir))
		if (afile == lowercaseString(p.path().filename().string()))
			return p.path().string();

	return std::string{};
}


std::string fixTextureFile(const std::string& file)
{
	return std::filesystem::exists(file) ? file : findSubstitute(file);
}



std::string replaceAll(const std::string& str, const std::string& oldSubStr, const std::string& newSubStr)
{
	std::string result = str;

	for (size_t p = result.find(oldSubStr); p != std::string::npos; p = result.find(oldSubStr))
		result.replace(p, oldSubStr.length(), newSubStr);

	return result;
}

int find_or_add(std::vector<std::string>& arr,const std::string& name)
{
	if (name.empty())
		return -1;

	auto i = std::find(std::begin(arr), std::end(arr), name);

	if (i == arr.end())
	{
		arr.push_back(name);
		return (int)arr.size() - 1;
	}

	return (int)std::distance(arr.begin(), i);
}


void MaterialAssimpConvert::saveMaterials(const char* fileName, const std::vector<MaterialDescription>& materials, const std::vector<std::string>& files)
{
	std::string pth(fileName);
	std::ofstream ofstream(pth, std::ios::trunc | std::ios::binary | std::ios::out);

	bool failed = false;
	ofstream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		uint32_t sz = (uint32_t)materials.size();
		ofstream.write((char*)&sz,sizeof(uint32_t));
		ofstream.write((char*)materials.data(), sizeof(MaterialDescription) * sz);
		ofstream.flush();
	}
	catch (std::exception& ex)
	{

	}
	ofstream.close();
}

MaterialDescription MaterialAssimpConvert::aimaterial_to_gmaterial(const aiMaterial* m, std::vector<std::string>& files, std::vector<std::string>& opacityMaps)
{
	MaterialDescription desc;
	aiColor4D Color;
	if (aiGetMaterialColor(m, AI_MATKEY_COLOR_AMBIENT, &Color) == AI_SUCCESS) {
		desc.emissiveColor_ = { Color.r, Color.g, Color.b, Color.a };
		if (desc.emissiveColor_.w > 1.0f)
			desc.emissiveColor_.w = 1.0f;
	}
	if (aiGetMaterialColor(m, AI_MATKEY_COLOR_DIFFUSE, &Color) == AI_SUCCESS) {
		desc.albedoColor_ = { Color.r, Color.g, Color.b, Color.a };
		if (desc.albedoColor_.w > 1.0f)      desc.albedoColor_.w = 1.0f;
	}
	if (aiGetMaterialColor(m, AI_MATKEY_COLOR_EMISSIVE, &Color) == AI_SUCCESS) {
		desc.emissiveColor_.x += Color.r;    desc.emissiveColor_.y += Color.g;    desc.emissiveColor_.z += Color.b;    desc.emissiveColor_.w += Color.a;
		if (desc.emissiveColor_.w > 1.0f)
			desc.albedoColor_.w = 1.0f;
	}
	const float opaquenessThreshold = 0.05f;
	float Opacity = 1.0f;

	if (aiGetMaterialFloat(m, AI_MATKEY_OPACITY, &Opacity) == AI_SUCCESS) {
		desc.transparencyFactor_ = glm::clamp(1.0f - Opacity, 0.0f, 1.0f);
		if (desc.transparencyFactor_ >= 1.0f - opaquenessThreshold)
			desc.transparencyFactor_ = 0.0f;
	}
	if (aiGetMaterialColor(m, AI_MATKEY_COLOR_TRANSPARENT, &Color) == AI_SUCCESS) {
		const float Opacity = std::max(std::max(Color.r, Color.g), Color.b);
		desc.transparencyFactor_ = glm::clamp(Opacity, 0.0f, 1.0f);
		if (desc.transparencyFactor_ >= 1.0f - opaquenessThreshold)
			desc.transparencyFactor_ = 0.0f;
		desc.alphaTest_ = 0.5f;
	}
	float tmp = 1.0f;
	if (aiGetMaterialFloat(m, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, &tmp) == AI_SUCCESS)
		desc.metallicFactor_ = tmp;

	if (aiGetMaterialFloat(m, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, &tmp) == AI_SUCCESS)
		desc.roughnessFactor_ = tmp;
	
	//X Finding textures
	//X And it to files

	aiString Path;
	aiTextureMapping Mapping;
	unsigned int UVIndex = 0;
	float Blend = 1.0f;
	aiTextureOp TextureOp = aiTextureOp_Add;
	aiTextureMapMode TextureMapMode[2] = { aiTextureMapMode_Wrap, aiTextureMapMode_Wrap };
	unsigned int TextureFlags = 0;

	if (aiGetMaterialTexture(m, aiTextureType_EMISSIVE, 0, &Path, &Mapping, &UVIndex, &Blend, &TextureOp, TextureMapMode, &TextureFlags) == AI_SUCCESS)
	{
		desc.emissiveMap_ = find_or_add(files, Path.C_Str());
	}

	if (aiGetMaterialTexture(m, aiTextureType_DIFFUSE, 0, &Path, &Mapping, &UVIndex, &Blend, &TextureOp, TextureMapMode, &TextureFlags) == AI_SUCCESS)
	{
		desc.albedoMap_ = find_or_add(files, Path.C_Str());
		const std::string albedoMap = std::string(Path.C_Str());
		if (albedoMap.find("grey_30") != albedoMap.npos)
			desc.flags_ |= MATERIAL_FLAG::MATERIAL_FLAG_TRANSPARENT;
	}

	// first try tangent space normal map
	if (aiGetMaterialTexture(m, aiTextureType_NORMALS, 0, &Path, &Mapping, &UVIndex, &Blend, &TextureOp, TextureMapMode, &TextureFlags) == AI_SUCCESS)
	{
		desc.normalMap_ = find_or_add(files, Path.C_Str());
	}
	if (desc.normalMap_ == 0xFFFFFFFF)
		if (aiGetMaterialTexture(m, aiTextureType_HEIGHT, 0, &Path, &Mapping, &UVIndex, &Blend, &TextureOp, TextureMapMode, &TextureFlags) == AI_SUCCESS)
			desc.normalMap_ = find_or_add(files, Path.C_Str());

	if (aiGetMaterialTexture(m, aiTextureType_OPACITY, 0, &Path, &Mapping, &UVIndex, &Blend, &TextureOp, TextureMapMode, &TextureFlags) == AI_SUCCESS)
	{
		desc.opacityMap_ = find_or_add(opacityMaps, Path.C_Str());
		desc.alphaTest_ = 0.5f;
	}


	//X Specific material finds goes here

	
	return desc;
}

MaterialDescription MaterialAssimpConvert::aimaterial_to_gmaterial(const aiMaterial* m, std::unordered_map<TEXTURE_MAP_TYPE, std::string>& textureFiles)
{
	MaterialDescription desc;
	aiColor4D Color;
	if (aiGetMaterialColor(m, AI_MATKEY_COLOR_AMBIENT, &Color) == AI_SUCCESS) {
		desc.emissiveColor_ = { Color.r, Color.g, Color.b, Color.a };
		if (desc.emissiveColor_.w > 1.0f)
			desc.emissiveColor_.w = 1.0f;
	}
	if (aiGetMaterialColor(m, AI_MATKEY_COLOR_DIFFUSE, &Color) == AI_SUCCESS) {
		desc.albedoColor_ = { Color.r, Color.g, Color.b, Color.a };
		if (desc.albedoColor_.w > 1.0f)      desc.albedoColor_.w = 1.0f;
	}
	if (aiGetMaterialColor(m, AI_MATKEY_COLOR_EMISSIVE, &Color) == AI_SUCCESS) {
		desc.emissiveColor_.x += Color.r;    desc.emissiveColor_.y += Color.g;    desc.emissiveColor_.z += Color.b;    desc.emissiveColor_.w += Color.a;
		if (desc.emissiveColor_.w > 1.0f)
			desc.albedoColor_.w = 1.0f;
	}
	const float opaquenessThreshold = 0.05f;
	float Opacity = 1.0f;

	if (aiGetMaterialFloat(m, AI_MATKEY_OPACITY, &Opacity) == AI_SUCCESS) {
		desc.transparencyFactor_ = glm::clamp(1.0f - Opacity, 0.0f, 1.0f);
		if (desc.transparencyFactor_ >= 1.0f - opaquenessThreshold)
			desc.transparencyFactor_ = 0.0f;
	}
	if (aiGetMaterialColor(m, AI_MATKEY_COLOR_TRANSPARENT, &Color) == AI_SUCCESS) {
		const float Opacity = std::max(std::max(Color.r, Color.g), Color.b);
		desc.transparencyFactor_ = glm::clamp(Opacity, 0.0f, 1.0f);
		if (desc.transparencyFactor_ >= 1.0f - opaquenessThreshold)
			desc.transparencyFactor_ = 0.0f;
		desc.alphaTest_ = 0.5f;
	}
	float tmp = 1.0f;
	if (aiGetMaterialFloat(m, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, &tmp) == AI_SUCCESS)
		desc.metallicFactor_ = tmp;

	if (aiGetMaterialFloat(m, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, &tmp) == AI_SUCCESS)
		desc.roughnessFactor_ = tmp;

	//X Finding textures
	//X And it to files

	aiString Path;
	aiTextureMapping Mapping;
	unsigned int UVIndex = 0;
	float Blend = 1.0f;
	aiTextureOp TextureOp = aiTextureOp_Add;
	aiTextureMapMode TextureMapMode[2] = { aiTextureMapMode_Wrap, aiTextureMapMode_Wrap };
	unsigned int TextureFlags = 0;

	if (aiGetMaterialTexture(m, aiTextureType_DIFFUSE, 0, &Path, &Mapping, &UVIndex, &Blend, &TextureOp, TextureMapMode, &TextureFlags) == AI_SUCCESS)
	{
		textureFiles.emplace(TEXTURE_MAP_TYPE_ALBEDO, Path.C_Str());
	}
	if (aiGetMaterialTexture(m, aiTextureType_NORMALS, 0, &Path, &Mapping, &UVIndex, &Blend, &TextureOp, TextureMapMode, &TextureFlags) == AI_SUCCESS)
	{
		textureFiles.emplace(TEXTURE_MAP_TYPE_NORMAL, Path.C_Str());

	}
	if (aiGetMaterialTexture(m, aiTextureType_METALNESS, 0, &Path, &Mapping, &UVIndex, &Blend, &TextureOp, TextureMapMode, &TextureFlags) == AI_SUCCESS)
	{
		textureFiles.emplace(TEXTURE_MAP_TYPE_METALNESS, Path.C_Str());
	}
	if (aiGetMaterialTexture(m, aiTextureType_DIFFUSE_ROUGHNESS, 0, &Path, &Mapping, &UVIndex, &Blend, &TextureOp, TextureMapMode, &TextureFlags) == AI_SUCCESS)
	{
		textureFiles.emplace(TEXTURE_MAP_TYPE_ROUGHNESS, Path.C_Str());
	}
	if (aiGetMaterialTexture(m, aiTextureType_LIGHTMAP, 0, &Path, &Mapping, &UVIndex, &Blend, &TextureOp, TextureMapMode, &TextureFlags) == AI_SUCCESS)
	{
		textureFiles.emplace(TEXTURE_MAP_TYPE_AMBIENT_OCCLUSION, Path.C_Str());
	}
	if (aiGetMaterialTexture(m, aiTextureType_EMISSIVE, 0, &Path, &Mapping, &UVIndex, &Blend, &TextureOp, TextureMapMode, &TextureFlags) == AI_SUCCESS)
	{
		textureFiles.emplace(TEXTURE_MAP_TYPE_EMISSIVE, Path.C_Str());
	}
	return desc;
}


std::string convertTexture(const std::string& file, const std::string& basePath, std::unordered_map<std::string, uint32_t>& opacityMapIndices, const std::vector<std::string>& opacityMaps)
{
	const int maxNewWidth = 512;
	const int maxNewHeight = 512;

	const auto srcFile = replaceAll(basePath + file, "\\", "/");
	const auto newFile = std::string("data/out_textures/") + lowercaseString(replaceAll(replaceAll(srcFile, "..", "__"), "/", "__") + std::string("__rescaled")) + std::string(".png");

	// load this image
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(fixTextureFile(srcFile).c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	uint8_t* src = pixels;
	texChannels = STBI_rgb_alpha;

	std::vector<uint8_t> tmpImage(maxNewWidth * maxNewHeight * 4);

	if (!src)
	{
		printf("Failed to load [%s] texture\n", srcFile.c_str());
		texWidth = maxNewWidth;
		texHeight = maxNewHeight;
		texChannels = STBI_rgb_alpha;
		src = tmpImage.data();
	}
	else
	{
		printf("Loaded [%s] %dx%d texture with %d channels\n", srcFile.c_str(), texWidth, texHeight, texChannels);
	}

	if (opacityMapIndices.count(file) > 0)
	{
		const auto opacityMapFile = replaceAll(basePath + opacityMaps[opacityMapIndices[file]], "\\", "/");
		int opacityWidth, opacityHeight;
		stbi_uc* opacityPixels = stbi_load(fixTextureFile(opacityMapFile).c_str(), &opacityWidth, &opacityHeight, nullptr, 1);

		if (!opacityPixels)
		{
			printf("Failed to load opacity mask [%s]\n", opacityMapFile.c_str());
		}

		assert(opacityPixels);
		assert(texWidth == opacityWidth);
		assert(texHeight == opacityHeight);

		// store the opacity mask in the alpha component of this image
		if (opacityPixels)
			for (int y = 0; y != opacityHeight; y++)
				for (int x = 0; x != opacityWidth; x++)
					src[(y * opacityWidth + x) * texChannels + 3] = opacityPixels[y * opacityWidth + x];

		stbi_image_free(opacityPixels);
	}

	const uint32_t imgSize = texWidth * texHeight * texChannels;
	std::vector<uint8_t> mipData(imgSize);
	uint8_t* dst = mipData.data();

	const int newW = std::min(texWidth, maxNewWidth);
	const int newH = std::min(texHeight, maxNewHeight);

	stbir_resize_uint8(src, texWidth, texHeight, 0, dst, newW, newH, 0, texChannels);

	stbi_write_png(newFile.c_str(), newW, newH, texChannels, dst, 0);

	if (pixels)
		stbi_image_free(pixels);

	return newFile;
}


void MaterialAssimpConvert::convertAndDownscaleAllTextures(
	const std::vector<MaterialDescription>& materials, const std::string& basePath, std::vector<std::string>& files, std::vector<std::string>& opacityMaps
)
{
	std::unordered_map<std::string, uint32_t> opacityMapIndices(files.size());

	for (const auto& m : materials)
		if (m.opacityMap_ != 0xFFFFFFFF && m.albedoMap_ != 0xFFFFFFFF)
			opacityMapIndices[files[m.albedoMap_]] = (uint32_t)m.opacityMap_;

	auto converter = [&](const std::string& s) -> std::string
	{
		return convertTexture(s, basePath, opacityMapIndices, opacityMaps);
	};

	std::transform(std::execution::par, std::begin(files), std::end(files), std::begin(files), converter);
}
