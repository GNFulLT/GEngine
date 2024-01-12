#include "internal/engine/scene/scene_loader.h"


#include <assimp/cimport.h>
#include <assimp/material.h>
#include <assimp/pbrmaterial.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <meshoptimizer.h>
#include "engine/rendering/scene/scene.h"
#include <volk.h>
#include "engine/resource/igtexture_resource.h"
#include <fstream>
#include "engine/rendering/mesh/mesh_constants.h"
#include "engine/rendering/mesh/gmesh_file.h"


//X TODO MAKE GENERIC HERE
#define MESHLET_VERTEX_COUNT 64
#define MESHLET_TRIANGLE_COUNT 124
#define CONE_WEIGHT 0.0


glm::vec2 oct_wrap(const glm::vec2& v)
{
	return glm::vec2(abs(1.f - v.y), abs(1.f - v.x)) * glm::vec2(v.x >= 0.f ? 1.f : -1.f, v.y >= 0.f ? 1.f : -1.f);
}

glm::vec2 octahedral_encode(const glm::vec3& n) {

	glm::vec2 enc;

	// Project the 3D vector onto the octahedron
	enc.x = n.x * (1.0f / (abs(n.x) + abs(n.y) + abs(n.z)));
	enc.y = n.y * (1.0f / (abs(n.x) + abs(n.y) + abs(n.z)));

	return (n.z < 0.0f) ? oct_wrap(n) : n;
}

bool SceneLoader::load_scene(std::filesystem::path path,IGSceneManager* sceneManager, IGResourceManager* resourceMng, bool meshletEnable,bool createDraw)
{
	if (std::filesystem::is_directory(path))
		return false;
	return load_scene_meshlet(path, sceneManager,resourceMng,meshletEnable,createDraw);
}

bool SceneLoader::save_mesh(std::filesystem::path path, const std::vector<float>& vertices, const std::vector<uint32_t>& indices, const GMeshData& gmeshData)
{
	const GMeshFileHeader header = { .magicValue = MeshConstants::MESH_FILE_HEADER_MAGIC_VALUE,    .meshCount = 1,    .dataBlockStartOffset = (uint32_t)(sizeof(GMeshFileHeader) + 1 * sizeof(GMeshData)),
	.indexDataSize = std::uint32_t(indices.size()) * sizeof(uint32_t),    .vertexDataSize = uint32_t(vertices.size()) * sizeof(float) };


	std::ofstream ofstream(path, std::ios::trunc | std::ios::binary | std::ios::out);
	bool failed = false;
	ofstream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try 
	{
		ofstream.write((char*)&header, sizeof(GMeshFileHeader));
		ofstream.write((char*)&gmeshData, sizeof(GMeshData));
		ofstream.write((char*)vertices.data(), header.vertexDataSize);
		ofstream.write((char*)indices.data(), header.indexDataSize);
		ofstream.flush();

	}
	catch (const std::ofstream::failure& e)
	{
		failed = true;
	}
	catch (const std::exception& e)
	{
		failed = true;
	}
	ofstream.close();
	return true;
}

void SceneLoader::load_basic_meshes(IGSceneManager* scene,IGResourceManager* res, bool meshletEnable)
{
	//auto pth = std::filesystem::current_path() / "assets"/"Meshes"/"cube.obj";
	//load_scene(pth, scene, res, meshletEnable,false);
}

uint32_t SceneLoader::load_gmesh_file(IGSceneManager* mng,std::filesystem::path path,bool meshletEnable)
{
	if (std::filesystem::is_directory(path) || path.extension() != ".gmesh")
		return -1;

	std::ifstream ifstream(path,  std::ios::binary | std::ios::in);
	GMeshFileHeader header;
	GMeshData gmeshData;
	MeshData2 meshData;

	ifstream.read((char*)&header, sizeof(GMeshFileHeader));

	meshData.indexData_.resize(header.indexDataSize / sizeof(uint32_t));
	meshData.vertexData_.resize(header.vertexDataSize / sizeof(float));

	ifstream.read((char*)&gmeshData, sizeof(GMeshData));
	ifstream.read((char*)meshData.vertexData_.data(), header.vertexDataSize);
	ifstream.read((char*)meshData.indexData_.data(), header.indexDataSize);

	ifstream.close();
	uint32_t elementPerVertex = calculateVertexElementCount(gmeshData.meshFlag);

	std::vector<BoundingBox> boxes;
	boxes.push_back(calculate_aabb_for_mesh(meshData.vertexData_, meshData.indexData_, elementPerVertex));

	meshData.boxes_ = boxes;
	meshData.meshes_.push_back(gmeshData);

	uint32_t meshIndex = mng->add_mesh_to_scene(&meshData);

	if (meshletEnable)
	{
		
		uint32_t meshletVertOff = 0;
		uint32_t meshletTriangleOff = 0;
		GMeshletDataExtra meshlet;

		process_meshlets(meshData.indexData_, meshData.vertexData_, meshlet.gmeshlets_, meshlet.meshletVertexData_, meshlet.meshletTriangleData_,
			elementPerVertex, meshletVertOff, meshletTriangleOff);
		
		GMeshletExtra extra;
		extra.meshletCount = meshlet.gmeshlets_.size();
		extra.meshletOffset = 0;
		extra.meshletTrianglesCount = meshlet.meshletTriangleData_.size();
		extra.meshletTrianglesOffset = 0;
		extra.meshletVerticesCount = meshlet.meshletVertexData_.size();
		extra.meshletVerticesOffset = 0;

		meshlet.gmeshletExtra_.push_back(extra);

		uint32_t meshletIndex = mng->add_meshlet_to_scene(&meshlet);
		assert(meshIndex == meshletIndex);


	}
	return meshIndex;
}

struct MaterialHeader
{
	uint32_t hashSize;
};

std::vector<std::string> split(std::string s, std::string delimiter) {
	size_t pos_start = 0, pos_end, delim_len = delimiter.length();
	std::string token;
	std::vector<std::string> res;

	while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
		token = s.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		res.push_back(token);
	}

	res.push_back(s.substr(pos_start));
	return res;
}

uint32_t SceneLoader::load_gmaterial_file(IGSceneManager* scene, IGResourceManager* resourceMng, std::filesystem::path path)
{
	if (std::filesystem::is_directory(path) || path.extension() != ".gmaterial")
		return -1;
	
	MaterialHeader header;
	MaterialDescription desc;
	std::string texturePaths;
	
	std::ifstream ifstream(path, std::ios::binary | std::ios::in);
	
	ifstream.read((char*)&header, sizeof(MaterialHeader));
	
	texturePaths.resize(header.hashSize+1);

	ifstream.read((char*)&desc, sizeof(MaterialDescription));
	ifstream.read(texturePaths.data(), header.hashSize);

	ifstream.close();

	//X Load Textures
	
	if (texturePaths.size() != 1)
	{	
		if (texturePaths[0] == '\0')
		{
			texturePaths = std::string(texturePaths.begin()+1, texturePaths.end());
			texturePaths[texturePaths.size()-1] = 'g';
			texturePaths += '\0';
		}
		auto paths = split(texturePaths, "/");
		for (auto typeAndPath : paths)
		{
			auto typeAndPath2 = split(typeAndPath,"=");
			if (typeAndPath2.size() != 2)
				assert(false);

			auto& type = typeAndPath2[0];
			auto& tpath = typeAndPath2[1];

			auto textureRes = resourceMng->create_texture_resource(tpath, "SCENE_RESOURCE", tpath, nullptr, VK_FORMAT_R8G8B8A8_UNORM);
			if (!textureRes.has_value())
			{
				//X LOG
				continue;
			}

			auto texture = textureRes.value();
			assert(texture->load() == RESOURCE_INIT_CODE_OK);
			auto textureId = scene->register_texture_to_scene(texture);

			if (type == "ALBEDO")
			{
				desc.albedoMap_ = textureId;
			}
			else if (type == "AO")
			{
				desc.ambientOcclusionMap_ = textureId;

			}
			else if (type == "EMISSIVE")
			{
				desc.emissiveMap_ = textureId;
			}
			else if (type == "METALLIC")
			{
				desc.metallicyMap_ = textureId;
			}
			else if (type == "ROUGHNESS")
			{
				desc.roughnessMap_ = textureId;
			}
			else
			{
				assert(false);
			}
		}
	}

	//X Load description to scene

	auto materialId = scene->add_material_to_scene(&desc);

	return materialId;
}


bool SceneLoader::load_scene_mesh(std::filesystem::path path, IGSceneManager* sceneMng, IGResourceManager* resourceMng)
{
	return false;
}

bool SceneLoader::load_scene_meshlet(std::filesystem::path path, IGSceneManager* sceneMng, IGResourceManager* resourceMng, bool meshletEnable, bool createDraw)
{
	auto scene = load_scene_ai(path);
	if (scene == nullptr)
		return false;

	MeshData2* meshletData = new MeshData2();
	meshletData->meshes_.reserve(scene->mNumMeshes);
	uint32_t indexOffset = 0;
	uint32_t vertexOffset = 0;
	uint32_t meshletOffset = 0;
	uint32_t meshletVertexOffset = 0;
	uint32_t meshletTriangleOffset = 0;

	MeshletGenerationData gdata;
	gdata.meshletOff = 0;
	gdata.meshletTriangleOff = 0;
	gdata.meshletVertOff = 0; 

	//X Load Meshlets
	
	for (unsigned int i = 0; i != scene->mNumMeshes; i++)
	{
		auto aimsh = scene->mMeshes[i];
		if (meshletEnable)
		{
			meshletData->meshes_.push_back(ai_mesh_to_gmesh(aimsh, false, 1, *meshletData, vertexOffset, indexOffset,true,&gdata));
		}
		else
		{
			meshletData->meshes_.push_back(ai_mesh_to_gmesh(aimsh, false, 1, *meshletData, vertexOffset, indexOffset));
		}
	}

	//X Load materials
	std::vector<MaterialDescription> desc;
	std::unordered_map<uint32_t, std::unordered_map<TEXTURE_MAP_TYPE, std::string>> textureFiles;

	for (unsigned int m = 0; m < scene->mNumMaterials; m++)
	{
		aiMaterial* mm = scene->mMaterials[m];
		std::unordered_map<TEXTURE_MAP_TYPE, std::string> textures;
		MaterialDescription D = aimaterial_to_gmaterial(mm, textures);
		desc.push_back(D);
		textureFiles.emplace(m, textures);
	}

	std::vector<MaterialDescription> materialDescs;
	Scene* gscene = Scene::create_scene_with_default_material(materialDescs);

	std::unordered_map<uint32_t, glm::mat4> transforms;

	std::queue<aiNode*> queue;
	if (auto meshCount = scene->mRootNode->mNumMeshes; meshCount != 0)
	{
		assert(scene->mRootNode->mChildren == 0);
		uint32_t parent = 0;

		for (int i = 0; i < meshCount; i++)
		{
			auto glmT = glm::transpose(glm::make_mat4(&scene->mRootNode->mTransformation.a1));
			uint32_t nodeIndex = gscene->add_node(*gscene, parent, 1);
			transforms.emplace(nodeIndex, glmT);
			//gscene->localTransform_[nodeIndex] = glmT;
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
			transforms.emplace(nodeIndex, glmT);

			//gscene->localTransform_[nodeIndex] = glmT;
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
	delete scene;


	//----------------------------------------
	if (meshletEnable)
	{
		transfer_to_gpu(path,gscene, *meshletData, transforms, desc, textureFiles, sceneMng,resourceMng,& gdata.gmeshletData,createDraw);
	}
	else
	{
		transfer_to_gpu(path,gscene, *meshletData, transforms, desc, textureFiles, sceneMng, resourceMng,nullptr, createDraw);

	}
}

const aiScene* SceneLoader::load_scene_ai(std::filesystem::path path)
{
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


	auto pathAsStr = path.string();
	const aiScene* scene = aiImportFile(pathAsStr.c_str(), flags);

	if (!scene || !scene->HasMeshes())
	{
		return nullptr;
	}

	return scene;
}

GMeshMeshlet SceneLoader::ai_mesh_to_gmeshlet(const aiMesh* m, bool loadLOD, uint32_t scaleFactor, GMeshletData& meshletData, uint32_t vertexOff, uint32_t indexOff, 
	uint32_t meshletOff, uint32_t meshletVertOff, uint32_t meshletTriangleOff, bool remapEnable)
{
	uint32_t numElementsToStore = 3;
	uint64_t meshFlag = 0;
	const bool hasTexCoords = m->HasTextureCoords(0);
	if (hasTexCoords)
	{
		meshFlag |= GMESH_COMPONENT_HAS_UV;
		numElementsToStore += 2;
	}
	if (m->HasNormals())
	{
		meshFlag |= GMESH_COMPONENT_HAS_NORMAL;
		numElementsToStore += 2;
		if (m->HasTangentsAndBitangents())
		{
			meshFlag |= GMESH_COMPONENT_HAS_TANGENTS_BITANGENTS;
			numElementsToStore += 6;
		}
	}
	const uint32_t streamElementSize = static_cast<uint32_t>(numElementsToStore * sizeof(float));

	GMeshMeshlet result = {
	.streamCount = 1,
	.indexOffset = indexOff,
	.vertexOffset = vertexOff,
	.vertexCount = m->mNumVertices,
	.streamOffset = { vertexOff * streamElementSize },
	.streamElementSize = { streamElementSize },
	.meshletOffset = meshletOff,
	.meshletVerticesOffset = meshletVertOff,
	.meshletTrianglesOffset = meshletTriangleOff
	};

	// Original data for LOD calculation
	std::vector<float> srcVertices;
	std::vector<uint32_t> srcIndices;

	std::vector<std::vector<uint32_t>> outLods;

	std::vector<float> vertices;


	for (size_t i = 0; i != m->mNumVertices; i++)
	{
		const aiVector3D v = m->mVertices[i];
		const aiVector3D n = m->mNormals[i];
		const aiVector3D tangent = m->mTangents[i];
		const aiVector3D bitangent = m->mBitangents[i];
		aiVector3D t = hasTexCoords ? m->mTextureCoords[0][i] : aiVector3D();


		srcVertices.push_back(v.x);
		srcVertices.push_back(v.y);
		srcVertices.push_back(v.z);


		vertices.push_back(v.x * scaleFactor);
		vertices.push_back(v.y * scaleFactor);
		vertices.push_back(v.z * scaleFactor);
		vertices.push_back(t.x);
		vertices.push_back(1.0f - t.y);


		glm::vec2 och = octahedral_encode(glm::vec3(n.x, n.y, n.z));
		vertices.push_back(och.x);
		vertices.push_back(och.y);
		if (m->HasTangentsAndBitangents())
		{
			vertices.push_back(tangent.x);
			vertices.push_back(tangent.y);
			vertices.push_back(tangent.z);
			vertices.push_back(bitangent.x);
			vertices.push_back(bitangent.y);
			vertices.push_back(bitangent.z);
		}
	}


	//X Push Indexes
	for (size_t i = 0; i != m->mNumFaces; i++)
	{
		if (m->mFaces[i].mNumIndices != 3)
			continue;
		for (unsigned j = 0; j != m->mFaces[i].mNumIndices; j++)
			srcIndices.push_back(m->mFaces[i].mIndices[j]);
	}

	if (remapEnable)
	{
		//X Now remap cache
		result.vertexCount = remap_vertex_index(vertices,srcIndices,vertices.size() / numElementsToStore , numElementsToStore * sizeof(float));
	}

	meshletData.vertexData_.insert(meshletData.vertexData_.end(), vertices.begin(), vertices.end());

	std::vector<GMeshlet>& meshlets = meshletData.gmeshlets_;
	std::vector<uint32_t>& meshletVertOffset = meshletData.meshletVertexData_;
	std::vector<uint8_t>& meshletTriangles = meshletData.meshletTriangleData_;

	auto prevMeshletSize = meshlets.size();
	auto prevMeshletVertSize = meshletVertOffset.size();
	auto prevMeshletTriangleSize = meshletTriangles.size();

	process_meshlets(srcIndices, srcVertices, meshlets, meshletVertOffset, meshletTriangles, 3, meshletVertOff, meshletTriangleOff);

	if (loadLOD)
		assert(false);
	outLods.push_back(srcIndices);

	uint32_t numIndices = 0;

	for (size_t l = 0; l < outLods.size(); l++)
	{
		for (size_t i = 0; i < outLods[l].size(); i++)
			meshletData.indexData_.push_back(outLods[l][i]);

		result.lodOffset[l] = numIndices;
		numIndices += (int)outLods[l].size();
	}

	result.lodOffset[outLods.size()] = numIndices;
	result.lodCount = (uint32_t)outLods.size();

	indexOff += numIndices;
	vertexOff += m->mNumVertices * numElementsToStore;
	result.meshletCount = (meshlets.size() - prevMeshletSize);
	result.meshletVerticesCount = (meshletVertOffset.size() - prevMeshletVertSize);
	result.meshletTrianglesCount = (meshletTriangles.size() - prevMeshletTriangleSize);
	meshletOff += result.meshletCount;
	meshletVertOff += result.meshletVerticesCount;
	meshletTriangleOff += result.meshletTrianglesCount;



	result.meshFlag = meshFlag;
	return result;
}

GMeshData SceneLoader::ai_mesh_to_gmesh(const aiMesh* m, bool loadLOD, uint32_t scaleFactor, MeshData2& meshData, uint32_t& vertexOffset, uint32_t& indexOffset, bool remapEnable, MeshletGenerationData* meshletGeneration)
{
	uint32_t numElementsToStore = 3;
	uint64_t meshFlag = 0;
	const bool hasTexCoords = m->HasTextureCoords(0);
	if (hasTexCoords)
	{
		meshFlag |= GMESH_COMPONENT_HAS_UV;
		numElementsToStore += 2;
	}
	if (m->HasNormals())
	{
		meshFlag |= GMESH_COMPONENT_HAS_NORMAL;
		numElementsToStore += 2;
		if (m->HasTangentsAndBitangents())
		{
			meshFlag |= GMESH_COMPONENT_HAS_TANGENTS_BITANGENTS;
			numElementsToStore += 6;
		}
	}
	const uint32_t streamElementSize = static_cast<uint32_t>(numElementsToStore * sizeof(float));

	GMeshData result = {
		.indexOffset = indexOffset,
		.vertexOffset = vertexOffset,
		.vertexCount = m->mNumVertices,
	};

	// Original data for LOD calculation
	std::vector<float> srcVertices;
	std::vector<uint32_t> srcIndices;

	std::vector<std::vector<uint32_t>> outLods;

	std::vector<float> vertices;

	for (size_t i = 0; i != m->mNumVertices; i++)
	{
		const aiVector3D v = m->mVertices[i];
		const aiVector3D n = m->mNormals[i];
		const aiVector3D tangent = m->mTangents[i];
		const aiVector3D bitangent = m->mBitangents[i];
		aiVector3D t = hasTexCoords ? m->mTextureCoords[0][i] : aiVector3D();


		srcVertices.push_back(v.x);
		srcVertices.push_back(v.y);
		srcVertices.push_back(v.z);


		vertices.push_back(v.x * scaleFactor);
		vertices.push_back(v.y * scaleFactor);
		vertices.push_back(v.z * scaleFactor);
		vertices.push_back(t.x);
		vertices.push_back(1.0f - t.y);


		glm::vec2 och = octahedral_encode(glm::vec3(n.x, n.y, n.z));
		vertices.push_back(och.x);
		vertices.push_back(och.y);
		if (m->HasTangentsAndBitangents())
		{
			vertices.push_back(tangent.x);
			vertices.push_back(tangent.y);
			vertices.push_back(tangent.z);
			vertices.push_back(bitangent.x);
			vertices.push_back(bitangent.y);
			vertices.push_back(bitangent.z);
		}
	}


	//X Push Indexes
	for (size_t i = 0; i != m->mNumFaces; i++)
	{
		if (m->mFaces[i].mNumIndices != 3)
			continue;
		for (unsigned j = 0; j != m->mFaces[i].mNumIndices; j++)
			srcIndices.push_back(m->mFaces[i].mIndices[j]);
	}

	if (remapEnable)
	{
		//X Now remap cache
		result.vertexCount = remap_vertex_index(vertices, srcIndices, vertices.size() / numElementsToStore, numElementsToStore * sizeof(float));
	}

	//X Calculate AABB
	meshData.boxes_.push_back(calculate_aabb_for_mesh(vertices,srcIndices,numElementsToStore));
	
	//X LOD Process
	if (loadLOD)
	{
		assert(false);
	}
	else
	{
		outLods.push_back(srcIndices);
	}

	//X Generate Meshlet Datas
	if (meshletGeneration != nullptr)
	{
		std::vector<GMeshlet>& meshlets = meshletGeneration->gmeshletData.gmeshlets_;
		std::vector<uint32_t>& meshletVertOffset = meshletGeneration->gmeshletData.meshletVertexData_;
		std::vector<uint8_t>& meshletTriangles = meshletGeneration->gmeshletData.meshletTriangleData_;

		auto prevMeshletSize = meshlets.size();
		auto prevMeshletVertSize = meshletVertOffset.size();
		auto prevMeshletTriangleSize = meshletTriangles.size();

		process_meshlets(srcIndices, srcVertices, meshlets, meshletVertOffset, meshletTriangles, 3, meshletGeneration->meshletVertOff
			, meshletGeneration->meshletTriangleOff);

		GMeshletExtra extra;
		extra.meshletOffset = meshletGeneration->meshletOff;
		extra.meshletTrianglesOffset = meshletGeneration->meshletTriangleOff;
		extra.meshletVerticesOffset = meshletGeneration->meshletVertOff;
		extra.meshletCount = (meshlets.size() - prevMeshletSize);
		extra.meshletVerticesCount = (meshletVertOffset.size() - prevMeshletVertSize);
		extra.meshletTrianglesCount = (meshletTriangles.size() - prevMeshletTriangleSize);

		meshletGeneration->meshletVertOff += extra.meshletVerticesCount;
		meshletGeneration->meshletTriangleOff += extra.meshletTrianglesCount;
		meshletGeneration->meshletOff += extra.meshletCount;
		meshletGeneration->gmeshletData.gmeshletExtra_.push_back(extra);
	}

	
	//X Add vertex and index to meshData
	meshData.vertexData_.insert(meshData.vertexData_.end(), vertices.begin(), vertices.end());
	
	uint32_t numIndices = 0;

	for (size_t l = 0; l < outLods.size(); l++)
	{
		for (size_t i = 0; i < outLods[l].size(); i++)
			meshData.indexData_.push_back(outLods[l][i]);

		result.lodOffset[l] = numIndices;
		numIndices += (int)outLods[l].size();
	}

	result.lodOffset[outLods.size()] = numIndices;
	result.lodCount = (uint32_t)outLods.size();

	indexOffset += numIndices;
	vertexOffset += m->mNumVertices * numElementsToStore;
	result.meshFlag = meshFlag;
	return result;

}

BoundingBox SceneLoader::calculate_aabb_for_mesh(std::vector<float>& vertices, std::vector<uint32_t>& indices, uint32_t elementPerVertex)
{
	auto numIndices = indices.size();

	glm::vec3 vmin(std::numeric_limits<float>::max());
	glm::vec3 vmax(std::numeric_limits<float>::lowest());

	for (auto i = 0; i != numIndices; i++)
	{
		auto vtxOffset = (elementPerVertex * indices[i]);
		const float* vf = &vertices[vtxOffset];
		vmin = glm::min(vmin, glm::vec3(vf[0], vf[1], vf[2]));
		vmax = glm::max(vmax, glm::vec3(vf[0], vf[1], vf[2]));
	}

	return BoundingBox(vmin, vmax);
}

uint32_t SceneLoader::remap_vertex_index(std::vector<float>& vertices, std::vector<uint32_t>& indices, uint32_t vertexCount, uint32_t vertexSize)
{
	std::vector<uint32_t> remapTable(indices.size());

	//size_t newVertexCount = meshopt_generateVertexRemap(remapTable.data(), indices.data(), remapTable.size(),
		//vertices.data(), vertexCount, vertexSize);
	//X T0D0 ALL ELEMENT FOR VERTEX MUST BE FLOAT
	//vertices.resize(newVertexCount * (vertexSize / sizeof(float)));

	//meshopt_remapVertexBuffer(vertices.data(), vertices.data(), newVertexCount, vertexSize, remapTable.data());
	//meshopt_remapIndexBuffer(indices.data(), nullptr, indices.size(), remapTable.data());

	meshopt_optimizeVertexCache(indices.data(), indices.data(), indices.size(), vertexCount);
	meshopt_optimizeOverdraw(indices.data(), indices.data(), indices.size(), vertices.data(), vertexCount, vertexSize, 1.01f);
	meshopt_optimizeVertexFetch(vertices.data(), indices.data(), indices.size(), vertices.data(), vertexCount, vertexSize);

	return vertexCount;
}

void SceneLoader::process_meshlets(std::vector<uint32_t>& indices, std::vector<float>& vertices, std::vector<GMeshlet>& outMeshlet, std::vector<uint32_t>& outMeshletVer, std::vector<uint8_t>& outMeshletTriangle, uint32_t elementPerVertex, uint32_t meshletVertexOffset, uint32_t meshletTriangleOffset)
{
	size_t verticesCountIn = vertices.size() / elementPerVertex;

	//X Get Meshlet opt
	uint32_t maxMeshlet = meshopt_buildMeshletsBound(indices.size(), MESHLET_VERTEX_COUNT, MESHLET_TRIANGLE_COUNT);
	std::vector<meshopt_Meshlet> localMeshlet(maxMeshlet);
	std::vector<uint32_t> meshletVert(maxMeshlet * MESHLET_VERTEX_COUNT);
	std::vector<uint8_t> meshletTriangle(maxMeshlet * MESHLET_TRIANGLE_COUNT);

	uint32_t meshletCount = meshopt_buildMeshlets(localMeshlet.data(), meshletVert.data(), meshletTriangle.data(), indices.data(), indices.size(), vertices.data(), verticesCountIn,
		sizeof(float) * elementPerVertex, MESHLET_VERTEX_COUNT, MESHLET_TRIANGLE_COUNT, CONE_WEIGHT);


	std::vector<GMeshlet> gmeshlets(meshletCount);
	auto last = localMeshlet[meshletCount - 1];
	meshletTriangle.resize(localMeshlet[meshletCount - 1].triangle_offset + ((last.triangle_count * 3 + 3) & ~3));
	meshletVert.resize(localMeshlet[meshletCount - 1].vertex_offset + localMeshlet[meshletCount - 1].vertex_count);
	for (int i = 0; i < meshletCount; i++)
	{
		auto& currentMeshlet = localMeshlet[i];
		auto meshletBound = meshopt_computeMeshletBounds(&meshletVert[currentMeshlet.vertex_offset], &meshletTriangle[currentMeshlet.triangle_offset], currentMeshlet.triangle_count, vertices.data(), verticesCountIn, sizeof(float) * elementPerVertex);
		GMeshlet& gmeshlet = gmeshlets[i];
		memcpy(gmeshlet.center, meshletBound.center, sizeof(float) * 3);
		memcpy(gmeshlet.coneAxis, meshletBound.cone_axis_s8, sizeof(char) * 3);
		gmeshlet.coneCutoff = meshletBound.cone_cutoff_s8;
		gmeshlet.radius = meshletBound.radius;
		gmeshlet.triangleCount = currentMeshlet.triangle_count;
		gmeshlet.triangleOffset = currentMeshlet.triangle_offset + meshletTriangleOffset;
		gmeshlet.vertexCount = currentMeshlet.vertex_count;
		gmeshlet.vertexOffset = currentMeshlet.vertex_offset + meshletVertexOffset;
	}
	auto currentSize = outMeshlet.size();
	outMeshlet.resize(currentSize + meshletCount);
	memcpy(&outMeshlet[currentSize], gmeshlets.data(), sizeof(GMeshlet) * meshletCount);

	currentSize = outMeshletVer.size();
	outMeshletVer.resize(currentSize + meshletVert.size());
	memcpy(&outMeshletVer[currentSize], meshletVert.data(), meshletVert.size() * sizeof(uint32_t));

	currentSize = outMeshletTriangle.size();
	outMeshletTriangle.resize(currentSize + meshletTriangle.size());
	memcpy(&outMeshletTriangle[currentSize], meshletTriangle.data(), meshletTriangle.size() * sizeof(uint8_t));
}

MaterialDescription SceneLoader::aimaterial_to_gmaterial(const aiMaterial* m, std::unordered_map<TEXTURE_MAP_TYPE, std::string>& textureFiles)
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

void SceneLoader::transfer_to_gpu(std::filesystem::path base,Scene* scene, MeshData2& meshData, std::unordered_map<uint32_t, glm::mat4>& outTransforms, std::vector<MaterialDescription>& materials, const std::unordered_map<uint32_t, std::unordered_map<TEXTURE_MAP_TYPE, std::string>>& texturePaths, IGSceneManager* sceneManager, IGResourceManager* resourceManager, GMeshletDataExtra* meshlet,bool createDraw)
{
	//X First load all meshes

	uint32_t meshIndex = sceneManager->add_mesh_to_scene(&meshData);
	if (meshlet != nullptr)
	{
		uint32_t meshletIndex = sceneManager->add_meshlet_to_scene(meshlet);
		assert(meshIndex == meshletIndex);
	}


	//X Load Materials
	std::unordered_map<uint32_t, uint32_t> aiMatToSceneMat;
	for (int i = 0; i < materials.size(); i++)
	{

		if (auto paths = texturePaths.find(i); paths != texturePaths.end())
		{
			auto resource = resourceManager;

			//X Load the texture
			//X Albedo
			auto itr = paths->second.find(TEXTURE_MAP_TYPE_ALBEDO);
			std::filesystem::path basePath = base.parent_path();

			if (itr != paths->second.end())
			{
				auto albedo = itr->second;
				std::string albedoTexturePath = (basePath / albedo).string();
				auto textureRes = resource->create_texture_resource(albedo, "editor", albedoTexturePath, nullptr, VK_FORMAT_R8G8B8A8_UNORM).value();
				assert(RESOURCE_INIT_CODE_OK == textureRes->load());
				//X Save the texture to the scene
				auto albedoTextureID = sceneManager->register_texture_to_scene(textureRes);
				materials[i].albedoMap_ = albedoTextureID;
			}
			//X Normal
			itr = paths->second.find(TEXTURE_MAP_TYPE_NORMAL);
			if (itr != paths->second.end())
			{
				auto normal = itr->second;
				std::string normalTexturePath = (basePath / normal).string();
				auto textureRes = resource->create_texture_resource(normal, "editor", normalTexturePath, nullptr, VK_FORMAT_R8G8B8A8_UNORM).value();
				assert(RESOURCE_INIT_CODE_OK == textureRes->load());
				//X Save the texture to the scene
				auto normalTextureId = sceneManager->register_texture_to_scene(textureRes);
				materials[i].normalMap_ = normalTextureId;
			}
			//X AO
			itr = paths->second.find(TEXTURE_MAP_TYPE_AMBIENT_OCCLUSION);
			if (itr != paths->second.end())
			{
				auto ao = itr->second;
				std::string aoTexturePath = (basePath / ao).string();
				auto textureRes = resource->create_texture_resource(ao, "editor", aoTexturePath, nullptr, VK_FORMAT_R8G8B8A8_UNORM).value();
				assert(RESOURCE_INIT_CODE_OK == textureRes->load());
				auto aoId = sceneManager->register_texture_to_scene(textureRes);
				materials[i].ambientOcclusionMap_ = aoId;
			}
			//X Roughness
			itr = paths->second.find(TEXTURE_MAP_TYPE_ROUGHNESS);
			if (itr != paths->second.end())
			{
				auto roughness = itr->second;
				std::string roughnessTexturePath = (basePath / roughness).string();
				auto textureRes = resource->create_texture_resource(roughness, "editor", roughnessTexturePath, nullptr, VK_FORMAT_R8G8B8A8_UNORM).value();
				assert(RESOURCE_INIT_CODE_OK == textureRes->load());
				auto roughnesssId = sceneManager->register_texture_to_scene(textureRes);
				materials[i].roughnessMap_ = roughnesssId;
			}
			//X Metallic
			itr = paths->second.find(TEXTURE_MAP_TYPE_METALNESS);
			if (itr != paths->second.end())
			{
				auto metallic = itr->second;
				std::string metallicTexturePath = (basePath / metallic).string();
				auto textureRes = resource->create_texture_resource(metallic, "editor", metallicTexturePath, nullptr, VK_FORMAT_R8G8B8A8_UNORM).value();
				assert(RESOURCE_INIT_CODE_OK == textureRes->load());
				auto metallicId = sceneManager->register_texture_to_scene(textureRes);
				materials[i].metallicyMap_ = metallicId;
			}
			//X Emissive
			itr = paths->second.find(TEXTURE_MAP_TYPE_EMISSIVE);
			if (itr != paths->second.end())
			{
				auto emissive = itr->second;
				std::string emissiveTexturePath = (basePath / emissive).string();
				auto textureRes = resource->create_texture_resource(emissive, "editor", emissiveTexturePath, nullptr, VK_FORMAT_R8G8B8A8_UNORM).value();
				assert(RESOURCE_INIT_CODE_OK == textureRes->load());
				auto emissiveId = sceneManager->register_texture_to_scene(textureRes);
				materials[i].emissiveMap_ = emissiveId;
			}
			auto materialIndex = sceneManager->add_material_to_scene(&materials[i]);
			aiMatToSceneMat.emplace(i, materialIndex);
		}
	}
	//X TODO : RETURN MESH MATERIAL BEGIN
	if (!createDraw)
		return;
	//X NOW CREATE NODES
	std::queue<uint32_t> queue;
	std::unordered_map<uint32_t, uint32_t> virtual_to_scene;
	queue.push(0);

	while (!queue.empty())
	{
		auto iter = queue.front();
		queue.pop();
		uint32_t nodeIndex = -1;
		if (auto mesh = scene->meshes_.find(iter); mesh != scene->meshes_.end())
		{
			auto localMeshIndex = mesh->second;
			uint32_t material = 0;
			if (auto mat = scene->materialForNode_.find(iter); mat != scene->materialForNode_.end())
			{
				material = aiMatToSceneMat.find(mat->second)->second;
			}
			uint32_t parent = 0;
			if (auto parentIter = virtual_to_scene.find(iter); parentIter != virtual_to_scene.end())
			{
				parent = parentIter->second;
			}
			nodeIndex = sceneManager->add_child_node_with_mesh_and_material_and_transform(parent, localMeshIndex + meshIndex, material, &outTransforms.find(iter)->second);
		}
		// Enqueue children (if any)
		uint32_t child = scene->hierarchy[iter].firstChild;
		while (child != UINT32_MAX)
		{
			queue.push(child);
			if (nodeIndex != -1)
			{
				virtual_to_scene.emplace(child, nodeIndex);
			}
			child = scene->hierarchy[child].nextSibling;
		}
	}
	
}
