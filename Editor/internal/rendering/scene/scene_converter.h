#ifndef SCENE_CONVERTER_H
#define SCENE_CONVERTER_H

#include <assimp/scene.h>
#include "engine/rendering/scene/scene.h"
#include <glm/glm.hpp>
#include <vector>
#include "engine/rendering/scene/scene_config.h"
#include "engine/rendering/mesh_data.h"
#include <unordered_map>

enum TEXTURE_MAP_TYPE
{
	TEXTURE_MAP_TYPE_ALBEDO,
	TEXTURE_MAP_TYPE_NORMAL,
	TEXTURE_MAP_TYPE_AMBIENT_OCCLUSION,
	TEXTURE_MAP_TYPE_ROUGHNESS,
	TEXTURE_MAP_TYPE_METALNESS,
	TEXTURE_MAP_TYPE_EMISSIVE
};

class SceneConverter
{
public:
	void traverse(const aiScene* sourceScene, Scene& scene, aiNode* node, int parent, int atLevel);

	glm::mat4 to_glm_mat4(const aiMatrix4x4& mat) const noexcept;

	void process_scene(const SceneConfig& cfg);

	MeshData* load_all_meshes(const char* path,std::vector<MaterialDescription>& desc ,std::unordered_map<uint32_t,std::unordered_map<TEXTURE_MAP_TYPE, std::string>>& textureFiles,Scene** transformDatas, std::unordered_map<uint32_t, glm::mat4>& transforms);
	GMeshletData* load_all_meshes_meshlet(const char* path, std::vector<MaterialDescription>& desc, std::unordered_map<uint32_t, std::unordered_map<TEXTURE_MAP_TYPE, std::string>>& textureFiles, Scene** transformDatas, std::unordered_map<uint32_t, glm::mat4>& transforms);

private:

	uint32_t g_indexOffset = 0;
	uint32_t g_vertexOffset = 0;
	MeshData g_MeshData;
};

#endif // SCENE_CONVERTER_H	