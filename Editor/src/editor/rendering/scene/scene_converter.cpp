#include "internal/rendering/scene/scene_converter.h"
#include <assimp/cimport.h>
#include <assimp/material.h>
#include <assimp/pbrmaterial.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "internal/rendering/mesh/gmesh_assimp_encoder.h"
#include "engine/rendering/material/gmaterial.h"
#include "internal/rendering/material/material_assimp_converter.h"
#include <queue>
void SceneConverter::traverse(const aiScene* sourceScene, Scene& scene, aiNode* node, int parent, int atLevel)
{
    int newNode = Scene::add_node(scene, parent, atLevel);
    if (node->mName.C_Str()) {
        uint32_t stringID = (uint32_t)scene.names_.size();
        scene.names_.push_back(std::string(node->mName.C_Str()));
        scene.nameForNode_[newNode] = stringID;
    }
    for (size_t i = 0; i < node->mNumMeshes; i++) {
        int newSubNode = Scene::add_node(scene, newNode, atLevel + 1);
        uint32_t stringID = (uint32_t)scene.names_.size();
        scene.names_.push_back(std::string(node->mName.C_Str()) + "_Mesh_" + std::to_string(i));
        scene.nameForNode_[newSubNode] = stringID;
        int mesh = (int)node->mMeshes[i];
        scene.meshes_[newSubNode] = mesh;
        scene.materialForNode_[newSubNode] = sourceScene->mMeshes[mesh]->mMaterialIndex;
        scene.globalTransform_[newSubNode] = glm::mat4(1.0f);
        scene.localTransform_[newSubNode] = glm::mat4(1.0f);
    }

    scene.globalTransform_[newNode] = glm::mat4(1.0f);
    scene.localTransform_[newNode] = to_glm_mat4(node->mTransformation);
    // Travese the childrens too
    for (unsigned int n = 0; n < node->mNumChildren; n++)
        traverse(sourceScene, scene, node->mChildren[n], newNode, atLevel + 1);
}

glm::mat4 SceneConverter::to_glm_mat4(const aiMatrix4x4& from) const noexcept
{
    glm::mat4 to;
    to[0][0] = (float)from.a1; to[0][1] = (float)from.b1;  to[0][2] = (float)from.c1; to[0][3] = (float)from.d1;
    to[1][0] = (float)from.a2; to[1][1] = (float)from.b2;  to[1][2] = (float)from.c2; to[1][3] = (float)from.d2;
    to[2][0] = (float)from.a3; to[2][1] = (float)from.b3;  to[2][2] = (float)from.c3; to[2][3] = (float)from.d3;
    to[3][0] = (float)from.a4; to[3][1] = (float)from.b4;  to[3][2] = (float)from.c4; to[3][3] = (float)from.d4;
    return to;
}

void dumpMaterial(const std::vector<std::string>& files, const MaterialDescription& d)
{
	printf("files: %d\n", (int)files.size());
	printf("maps: %u/%u/%u/%u/%u\n", (uint32_t)d.albedoMap_, (uint32_t)d.ambientOcclusionMap_, (uint32_t)d.emissiveMap_, (uint32_t)d.opacityMap_, (uint32_t)d.metallicRoughnessMap_);
	printf(" albedo:    %s\n", (d.albedoMap_ < 0xFFFF) ? files[d.albedoMap_].c_str() : "");
	printf(" occlusion: %s\n", (d.ambientOcclusionMap_ < 0xFFFF) ? files[d.ambientOcclusionMap_].c_str() : "");
	printf(" emission:  %s\n", (d.emissiveMap_ < 0xFFFF) ? files[d.emissiveMap_].c_str() : "");
	printf(" opacity:   %s\n", (d.opacityMap_ < 0xFFFF) ? files[d.opacityMap_].c_str() : "");
	printf(" MeR:       %s\n", (d.metallicRoughnessMap_ < 0xFFFF) ? files[d.metallicRoughnessMap_].c_str() : "");
	printf(" Normal:    %s\n", (d.normalMap_ < 0xFFFF) ? files[d.normalMap_].c_str() : "");
}

void SceneConverter::process_scene(const SceneConfig& cfg)
{
	// clear mesh data from previous scene
	g_MeshData.meshes_.clear();
	g_MeshData.boxes_.clear();
	g_MeshData.indexData_.clear();
	g_MeshData.vertexData_.clear();

	g_indexOffset = 0;
	g_vertexOffset = 0;

	// extract base model path
	const std::size_t pathSeparator = cfg.fileName.find_last_of("/\\");
	const std::string basePath = (pathSeparator != std::string::npos) ? cfg.fileName.substr(0, pathSeparator + 1) : std::string();

	const unsigned int flags = 0 |
		aiProcess_JoinIdenticalVertices |
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_LimitBoneWeights |
		aiProcess_SplitLargeMeshes |
		aiProcess_ImproveCacheLocality |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_FindDegenerates |
		aiProcess_FindInvalidData |
		aiProcess_GenUVCoords;

	printf("Loading scene from '%s'...\n", cfg.fileName.c_str());

	const aiScene* scene = aiImportFile(cfg.fileName.c_str(), flags);

	if (!scene || !scene->HasMeshes())
	{
		printf("Unable to load '%s'\n", cfg.fileName.c_str());
		exit(EXIT_FAILURE);
	}

	// 1. Mesh conversion as in Chapter 5
	g_MeshData.meshes_.reserve(scene->mNumMeshes);
	g_MeshData.boxes_.reserve(scene->mNumMeshes);

	for (unsigned int i = 0; i != scene->mNumMeshes; i++)
	{
		printf("\nConverting meshes %u/%u...", i + 1, scene->mNumMeshes);
		GMesh mesh = GMeshEncoder::ai_mesh_to_gmesh(scene->mMeshes[i], cfg.calculateLODs,cfg.scale,g_MeshData, g_indexOffset, g_vertexOffset);
		g_MeshData.meshes_.push_back(mesh);

	}

	recalculateBoundingBoxes(g_MeshData);

	GMeshEncoder::save_to_file_and_reset(cfg.outputMesh.c_str(), g_MeshData);

	Scene ourScene;

	// 2. Material conversion
	std::vector<MaterialDescription> materials;
	std::vector<std::string>& materialNames = ourScene.materialNames_;

	std::vector<std::string> files;
	std::vector<std::string> opacityMaps;

	for (unsigned int m = 0; m < scene->mNumMaterials; m++)
	{
		aiMaterial* mm = scene->mMaterials[m];

		printf("Material [%s] %u\n", mm->GetName().C_Str(), m);
		materialNames.push_back(std::string(mm->GetName().C_Str()));

		MaterialDescription D = MaterialAssimpConvert::aimaterial_to_gmaterial(mm, files, opacityMaps);
		materials.push_back(D);
		dumpMaterial(files, D);
	}

	// 3. Texture processing, rescaling and packing
	MaterialAssimpConvert::convertAndDownscaleAllTextures(materials, basePath, files, opacityMaps);

	MaterialAssimpConvert::saveMaterials(cfg.outputMaterials.c_str(), materials, files);

	// 4. Scene hierarchy conversion
	traverse(scene, ourScene, scene->mRootNode, -1, 0);

	Scene::save_the_scene(ourScene, cfg.outputScene.c_str());
}

glm::mat4 convertMatrix(const aiMatrix4x4& aiMat)
{
	return {
	aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1,
	aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2,
	aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3,
	aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4
	};
}
MeshData* SceneConverter::load_all_meshes(const char* path, std::vector<MaterialDescription>& desc, std::unordered_map<uint32_t, std::unordered_map<TEXTURE_MAP_TYPE, std::string>>& textureFiles, Scene** virtualScene)
{
	MeshData* meshData = new MeshData();
	uint32_t indexOffset = 0;
	uint32_t vertexOffset = 0;

	const unsigned int flags = 0 |
		aiProcess_JoinIdenticalVertices |
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_LimitBoneWeights |
		aiProcess_SplitLargeMeshes |
		aiProcess_CalcTangentSpace |
		aiProcess_ImproveCacheLocality |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_FindDegenerates |
		aiProcess_FindInvalidData |
		aiProcess_GenUVCoords;

	printf("Loading meshes from '%s'...\n", path);

	const aiScene* scene = aiImportFile(path, flags);

	if (!scene || !scene->HasMeshes())
	{
		printf("Unable to load '%s'\n", path);
		delete meshData;
		return nullptr;
	}
	
	meshData->meshes_.reserve(scene->mNumMeshes);
	meshData->boxes_.reserve(scene->mNumMeshes);
	
	for (unsigned int i = 0; i != scene->mNumMeshes; i++)
	{
		printf("\nConverting meshes %u/%u...", i + 1, scene->mNumMeshes);
		auto aimsh = scene->mMeshes[i];
		if (i == 102)
		{
			int a = 5;
		}
		GMesh mesh = GMeshEncoder::ai_mesh_to_gmesh(aimsh, false, 1, *meshData, indexOffset, vertexOffset);
		meshData->meshes_.push_back(mesh);
	}
	
	for (unsigned int m = 0; m < scene->mNumMaterials; m++)
	{
		aiMaterial* mm = scene->mMaterials[m];
		std::unordered_map<TEXTURE_MAP_TYPE, std::string> textures;
		MaterialDescription D = MaterialAssimpConvert::aimaterial_to_gmaterial(mm, textures);
		desc.push_back(D);
		textureFiles.emplace(m, textures);
	}

	recalculateBoundingBoxes(*meshData);
	std::vector<MaterialDescription> materialDescs;
	Scene* gscene = Scene::create_scene_with_default_material(materialDescs);

	std::queue<aiNode*> queue;
	if (auto meshCount = scene->mRootNode->mNumMeshes;meshCount != 0)
	{
		assert(scene->mRootNode->mChildren == 0);
		uint32_t parent = 0;
		
		for (int i = 0; i < meshCount; i++)
		{
			auto glmT = glm::transpose(glm::make_mat4(&scene->mRootNode->mTransformation.a1));
			uint32_t nodeIndex = gscene->add_node(*gscene, parent, 1);
			gscene->localTransform_[nodeIndex] = glmT;
			gscene->meshes_.emplace(nodeIndex, i);
			gscene->materialForNode_.emplace(nodeIndex, scene->mMeshes[i]->mMaterialIndex);
		}
	}
	queue.push(scene->mRootNode);
	int level = 1;
	std::unordered_map<aiNode*,uint32_t> ai_to_scene;
	while (!queue.empty())
	{
		auto iter = queue.front();
		queue.pop();
		
		//X Iterate childrens
		for (int i = 0; i < iter->mNumChildren; i++)
		{
			int meshCount = iter->mChildren[i]->mNumMeshes;
			queue.push(iter->mChildren[i]);
			

			uint32_t parent = 0;
			if (auto parentItr = ai_to_scene.find(iter->mChildren[i]); parentItr != ai_to_scene.end())
			{
				parent = parentItr->second;
			}
			auto glmT = glm::transpose(glm::make_mat4(&iter->mChildren[i]->mTransformation.a1));
			
			uint32_t nodeIndex = gscene->add_node(*gscene, parent, level);

			ai_to_scene.emplace(iter->mChildren[i], nodeIndex);
			
			gscene->localTransform_[nodeIndex] = glmT;
			if (meshCount != 0)
			{
				auto meshIndex = iter->mChildren[i]->mMeshes[0];
				gscene->meshes_.emplace(nodeIndex, meshIndex);
				gscene->materialForNode_.emplace(nodeIndex, scene->mMeshes[meshIndex]->mMaterialIndex);
			}
			
			for (int j = 0; j < iter->mChildren[i]->mNumChildren; j++)
			{
				queue.push(iter->mChildren[i]->mChildren[j]);
				ai_to_scene.emplace(iter->mChildren[i]->mChildren[j], nodeIndex);
			}
		}
		level++;
	}
	*virtualScene = gscene;
	delete scene;
	return meshData;
}

GMeshletData* SceneConverter::load_all_meshes_meshlet(const char* path, std::vector<MaterialDescription>& desc, std::unordered_map<uint32_t, std::unordered_map<TEXTURE_MAP_TYPE, std::string>>& textureFiles, Scene** virtualScene)
{
	GMeshletData* meshletData = new GMeshletData();
	uint32_t indexOffset = 0;
	uint32_t vertexOffset = 0;

	const unsigned int flags = 0 |
		aiProcess_JoinIdenticalVertices |
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_LimitBoneWeights |
		aiProcess_SplitLargeMeshes |
		aiProcess_CalcTangentSpace |
		aiProcess_ImproveCacheLocality |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_FindDegenerates |
		aiProcess_FindInvalidData |
		aiProcess_GenUVCoords;

	printf("Loading meshes from '%s'...\n", path);

	const aiScene* scene = aiImportFile(path, flags);

	if (!scene || !scene->HasMeshes())
	{
		printf("Unable to load '%s'\n", path);
		delete meshletData;
		return nullptr;
	}

	meshletData->gmeshMeshlets_.reserve(scene->mNumMeshes);
	uint32_t meshletOffset = 0;
	uint32_t meshletVertexOffset = 0;
	uint32_t meshletTriangleOffset = 0;
	for (unsigned int i = 0; i != scene->mNumMeshes; i++)
	{
		printf("\nConverting meshes %u/%u...", i + 1, scene->mNumMeshes);
		auto aimsh = scene->mMeshes[i];
		if (i == 102)
		{
			int a = 5;
		}
		auto meshlet = GMeshEncoder::ai_mesh_to_gmesh_meshlet(aimsh, false, 1, *meshletData,  vertexOffset, indexOffset, meshletOffset, meshletVertexOffset,
			meshletTriangleOffset);
		meshletData->gmeshMeshlets_.push_back(meshlet);
	}

	for (unsigned int m = 0; m < scene->mNumMaterials; m++)
	{
		aiMaterial* mm = scene->mMaterials[m];
		std::unordered_map<TEXTURE_MAP_TYPE, std::string> textures;
		MaterialDescription D = MaterialAssimpConvert::aimaterial_to_gmaterial(mm, textures);
		desc.push_back(D);
		textureFiles.emplace(m, textures);
	}

	//recalculateBoundingBoxes(*meshData);
	std::vector<MaterialDescription> materialDescs;
	Scene* gscene = Scene::create_scene_with_default_material(materialDescs);

	std::queue<aiNode*> queue;
	if (auto meshCount = scene->mRootNode->mNumMeshes; meshCount != 0)
	{
		assert(scene->mRootNode->mChildren == 0);
		uint32_t parent = 0;

		for (int i = 0; i < meshCount; i++)
		{
			auto glmT = glm::transpose(glm::make_mat4(&scene->mRootNode->mTransformation.a1));
			uint32_t nodeIndex = gscene->add_node(*gscene, parent, 1);
			gscene->localTransform_[nodeIndex] = glmT;
			gscene->meshes_.emplace(nodeIndex, i);
			gscene->materialForNode_.emplace(nodeIndex, scene->mMeshes[i]->mMaterialIndex);
		}
	}
	queue.push(scene->mRootNode);
	int level = 1;
	std::unordered_map<aiNode*, uint32_t> ai_to_scene;
	while (!queue.empty())
	{
		auto iter = queue.front();
		queue.pop();

		//X Iterate childrens
		for (int i = 0; i < iter->mNumChildren; i++)
		{
			int meshCount = iter->mChildren[i]->mNumMeshes;
			queue.push(iter->mChildren[i]);


			uint32_t parent = 0;
			if (auto parentItr = ai_to_scene.find(iter->mChildren[i]); parentItr != ai_to_scene.end())
			{
				parent = parentItr->second;
			}
			auto glmT = glm::transpose(glm::make_mat4(&iter->mChildren[i]->mTransformation.a1));

			uint32_t nodeIndex = gscene->add_node(*gscene, parent, level);

			ai_to_scene.emplace(iter->mChildren[i], nodeIndex);

			gscene->localTransform_[nodeIndex] = glmT;
			if (meshCount != 0)
			{
				auto meshIndex = iter->mChildren[i]->mMeshes[0];
				gscene->meshes_.emplace(nodeIndex, meshIndex);
				gscene->materialForNode_.emplace(nodeIndex, scene->mMeshes[meshIndex]->mMaterialIndex);
			}

			for (int j = 0; j < iter->mChildren[i]->mNumChildren; j++)
			{
				queue.push(iter->mChildren[i]->mChildren[j]);
				ai_to_scene.emplace(iter->mChildren[i]->mChildren[j], nodeIndex);
			}
		}
		level++;
	}
	*virtualScene = gscene;
	delete scene;
	return meshletData;

}
