#ifndef SCENE_LOADER_H
#define SCENE_LOADER_H

#include <filesystem>
#include "engine/manager/igscene_manager.h"
#include "engine/rendering/material/gmaterial.h"
#include "engine/rendering/scene/scene_config.h"
#include "engine/rendering/mesh_data.h"
#include <unordered_map>
#include "engine/manager/igresource_manager.h"

class aiScene;
class aiMesh;
class aiMaterial;


class SceneLoader
{
public:
	struct MeshletGenerationData
	{
		GMeshletDataExtra gmeshletData;
		uint32_t meshletOff;
		uint32_t meshletVertOff;
		uint32_t meshletTriangleOff;
	};
	enum TEXTURE_MAP_TYPE
	{
		TEXTURE_MAP_TYPE_ALBEDO,
		TEXTURE_MAP_TYPE_NORMAL,
		TEXTURE_MAP_TYPE_AMBIENT_OCCLUSION,
		TEXTURE_MAP_TYPE_ROUGHNESS,
		TEXTURE_MAP_TYPE_METALNESS,
		TEXTURE_MAP_TYPE_EMISSIVE
	};

	static bool load_scene(std::filesystem::path path,IGSceneManager* scene,IGResourceManager* resourceMng,bool meshletEnable = true);

private:
	static bool load_scene_mesh(std::filesystem::path path, IGSceneManager* sceneMng, IGResourceManager* resourceMng);
	static bool load_scene_meshlet(std::filesystem::path path, IGSceneManager* sceneMng, IGResourceManager* resourceMng,bool meshletEnable);


	//X Helpers

	static const aiScene* load_scene_ai(std::filesystem::path path);
	static GMeshMeshlet ai_mesh_to_gmeshlet(const aiMesh* mesh,bool loadLOD,uint32_t scaleFactor,GMeshletData&, uint32_t vertexOff, uint32_t indexOff, uint32_t meshletOff,
		uint32_t meshletVertOff, uint32_t meshletTriangleOff,bool remapEnable = true);

	static GMeshData ai_mesh_to_gmesh(const aiMesh* mesh, bool loadLOD, uint32_t scaleFactor, MeshData2& meshData, uint32_t& vertexOff, uint32_t& indexOff, bool remapEnable = true,
		MeshletGenerationData* meshletGeneration = nullptr);

	static BoundingBox calculate_aabb_for_mesh(std::vector<float>& vertices, std::vector<uint32_t>& indices, uint32_t elementPerVertex);

	static uint32_t remap_vertex_index(std::vector<float>& vertices,std::vector<uint32_t>& indices,uint32_t vertexCount,uint32_t vertexSize);

	static void process_meshlets(std::vector<uint32_t>& indices, std::vector<float>& vertices, std::vector<GMeshlet>& outMeshlets, std::vector<uint32_t>& outMeshletVert, std::vector<uint8_t>& outMeshletTriangles, 
		uint32_t elementPerVertex, uint32_t meshletVertexOffset, uint32_t meshletTriangleOffset);
	
	//X Material
	static MaterialDescription aimaterial_to_gmaterial(const aiMaterial* m, std::unordered_map<TEXTURE_MAP_TYPE, std::string>& textureFiles);

	static void transfer_to_gpu(Scene* scene, MeshData2& meshData, std::unordered_map<uint32_t, glm::mat4>& outTransforms, std::vector<MaterialDescription>& materials,const std::unordered_map<uint32_t, std::unordered_map<TEXTURE_MAP_TYPE, std::string>>& textureFiles, IGSceneManager* sceneManager, IGResourceManager* resManager,GMeshletDataExtra* meshlet = nullptr);

};

#endif // SCENE_LOADER_H