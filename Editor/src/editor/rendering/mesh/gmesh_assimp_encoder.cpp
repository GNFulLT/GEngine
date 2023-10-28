#include "internal/rendering/mesh/gmesh_assimp_encoder.h"
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <spdlog/fmt/fmt.h>
#include "engine/rendering/mesh/gmesh_file.h"
#include <fstream>
#include <meshoptimizer.h>
#include <filesystem>

GMeshEncoder::GMeshEncoder(IOwningGLogger* logger)
{
	m_logger = logger;
}

std::expected<int, GMESH_ENCODER_ERROR> GMeshEncoder::decode_and_encode_to_gmesh(std::filesystem::path path)
{
	auto fileName = path.string();
	if (verbose) {
		m_logger->log_d(fmt::format("Loading file : {}",fileName).c_str());
	}
	const unsigned int flags = aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_PreTransformVertices | aiProcess_RemoveRedundantMaterials | aiProcess_FindDegenerates | aiProcess_FindInvalidData | aiProcess_FindInstances | aiProcess_OptimizeMeshes;
	const aiScene* scene = aiImportFile(fileName.c_str(), flags);
	if (!scene || !scene->HasMeshes()) {
		m_logger->log_e(fmt::format("Couldn't decode the file : {}", fileName).c_str());
		return std::unexpected(GMESH_ENCODER_ERROR_UNKNOWN);
	}

	g_MeshData.meshes_.reserve(scene->mNumMeshes);
	for (size_t i = 0; i != scene->mNumMeshes; i++)
		g_MeshData.meshes_.push_back(ai_mesh_to_gmesh(scene->mMeshes[i],false,1));

	return scene->mNumMeshes;

}

std::expected<int, GMESH_ENCODER_SAVE_ERROR> GMeshEncoder::save_to_file_and_reset(const char* path)
{
	const GMeshFileHeader header = { .magicValue = MeshConstants::MESH_FILE_HEADER_MAGIC_VALUE,    .meshCount = (uint32_t)g_MeshData.meshes_.size(),    .dataBlockStartOffset = (uint32_t)(sizeof(GMeshFileHeader) + g_MeshData.meshes_.size() * sizeof(GMesh)),
		.indexDataSize = std::uint32_t(g_MeshData.indexData_.size()) * sizeof(uint32_t),    .vertexDataSize = uint32_t(g_MeshData.vertexData_.size()) * sizeof(float) };

	std::string pth(path);
	pth += MeshConstants::MESH_FILE_EXTENSION;
	std::ofstream ofstream(pth, std::ios::trunc | std::ios::binary | std::ios::out);
	m_logger->log_e(fmt::format("Saving the file : {}", pth.c_str()).c_str());

	bool failed = false;
	ofstream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		//X Write header
		ofstream.write((char*)&header,sizeof(GMeshFileHeader));
		//X Write meshes
		ofstream.write((char*)g_MeshData.meshes_.data(), sizeof(GMesh) * g_MeshData.meshes_.size());
		//X Write Indices
		ofstream.write((char*)g_MeshData.indexData_.data(), header.indexDataSize);
		//X Write vertices
		ofstream.write((char*)g_MeshData.vertexData_.data(), header.vertexDataSize);
		ofstream.flush();
	}
	catch (const std::ofstream::failure& e)
	{
		m_logger->log_e(fmt::format("Error while saving the file : {}", e.code().message()).c_str());
		failed = true;
	}
	catch (const std::exception& e)
	{
		failed = true;
	}

	ofstream.close();
	int meshCount = g_MeshData.meshes_.size();
	reset();
	if (failed)
	{
		return std::unexpected(GMESH_ENCODER_SAVE_ERROR_UNKNOWN);
	}
	return meshCount;
	
}

void GMeshEncoder::reset()
{
	g_MeshData.meshes_.clear();
	g_MeshData.vertexData_.clear();
	g_MeshData.indexData_.clear();
	g_indexOffset = 0;
	g_vertexOffset = 0;
	
}
//X TODO MAKE GENERIC HERE
#define MESHLET_VERTEX_COUNT 64
#define MESHLET_TRIANGLE_COUNT 124
#define CONE_WEIGHT 0.0
void GMeshEncoder::process_lods(std::vector<uint32_t>& indices, std::vector<float>& vertices, std::vector<std::vector<uint32_t>>& outLods,uint32_t elementPerVertex)
{
	size_t verticesCountIn = vertices.size() / elementPerVertex;
	size_t targetIndicesCount = indices.size();

	uint8_t LOD = 4;

	printf("\n   LOD0: %i indices", int(indices.size()));

	outLods.push_back(indices);

	while (targetIndicesCount > 720 && LOD < 8)
	{
		targetIndicesCount = (3*indices.size()) / 4;

		bool sloppy = false;

		size_t numOptIndices = meshopt_simplify(
			indices.data(),
			indices.data(), (uint32_t)indices.size(),
			vertices.data(), verticesCountIn,
			sizeof(float) * elementPerVertex,
			targetIndicesCount, 0.02f);

		// cannot simplify further
		if (static_cast<size_t>(numOptIndices * 1.1f) > indices.size())
		{
			if (LOD > 1)
			{
				// try harder
				numOptIndices = meshopt_simplifySloppy(
					indices.data(),
					indices.data(), indices.size(),
					vertices.data(), verticesCountIn,
					sizeof(float) * 3,
					targetIndicesCount, 0.02f, nullptr);
				sloppy = true;
				if (numOptIndices == indices.size()) break;
			}
			else
				break;
		}

		indices.resize(numOptIndices);

		meshopt_optimizeVertexCache(indices.data(), indices.data(), indices.size(), verticesCountIn);

		printf("\n   LOD%i: %i indices %s", int(LOD), int(numOptIndices), sloppy ? "[sloppy]" : "");

		LOD++;

		outLods.push_back(indices);
	}

	//X Get Meshlet opt
	uint32_t maxMeshlet = meshopt_buildMeshletsBound(indices.size(), MESHLET_VERTEX_COUNT, MESHLET_TRIANGLE_COUNT);
	std::vector<meshopt_Meshlet> localMeshlet(maxMeshlet);
	std::vector<uint32_t> meshletVertices(maxMeshlet * MESHLET_VERTEX_COUNT);
	std::vector<unsigned char> meshletTriangles(maxMeshlet * MESHLET_TRIANGLE_COUNT);
	
	uint32_t meshletCount = meshopt_buildMeshlets(localMeshlet.data(), meshletVertices.data(), meshletTriangles.data(),indices.data(),indices.size(),vertices.data(),vertices.size(),
		sizeof(float) * elementPerVertex, MESHLET_VERTEX_COUNT, MESHLET_TRIANGLE_COUNT,CONE_WEIGHT);
}
std::expected<int, GMESH_ENCODER_SAVE_ERROR> GMeshEncoder::save_to_file_and_reset(const char* path, const MeshData& m)
{
	const GMeshFileHeader header = { .magicValue = MeshConstants::MESH_FILE_HEADER_MAGIC_VALUE,    .meshCount = (uint32_t)m.meshes_.size(),    .dataBlockStartOffset = (uint32_t)(sizeof(GMeshFileHeader) + m.meshes_.size() * sizeof(GMesh)),
			.indexDataSize = std::uint32_t(m.indexData_.size()) * sizeof(uint32_t),    .vertexDataSize = uint32_t(m.vertexData_.size()) * sizeof(float) };

	std::string pth(path);
	std::ofstream ofstream(pth, std::ios::trunc | std::ios::binary | std::ios::out);

	bool failed = false;
	ofstream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		//X Write header
		ofstream.write((char*)&header, sizeof(GMeshFileHeader));
		//X Write meshes
		ofstream.write((char*)m.meshes_.data(), sizeof(GMesh) * m.meshes_.size());
		ofstream.write((char*)m.boxes_.data(), sizeof(BoundingBox) * m.boxes_.size());
		//X Write Indices
		ofstream.write((char*)m.indexData_.data(), header.indexDataSize);
		//X Write vertices
		ofstream.write((char*)m.vertexData_.data(), header.vertexDataSize);
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

	if (failed)
	{
		return std::unexpected(GMESH_ENCODER_SAVE_ERROR_UNKNOWN);
	}
}

glm::vec2 oct_wrap(const glm::vec2& v)
{
	return glm::vec2(abs(1.f - v.y), abs(1.f - v.x))* glm::vec2(v.x >= 0.f ? 1.f : -1.f, v.y >= 0.f ? 1.f : -1.f);
}

glm::vec2 octahedral_encode(const glm::vec3& n) {

	glm::vec2 enc;

	// Project the 3D vector onto the octahedron
	enc.x = n.x * (1.0f / (abs(n.x) + abs(n.y) + abs(n.z)));
	enc.y = n.y * (1.0f / (abs(n.x) + abs(n.y) + abs(n.z)));

	return (n.z < 0.0f) ? oct_wrap(n) : n;
}

GMesh GMeshEncoder::ai_mesh_to_gmesh(const aiMesh* m,bool loadLods,int scale)
{
	const bool hasTexCoords = m->HasTextureCoords(0);
	const uint32_t streamElementSize = static_cast<uint32_t>(g_numElementsToStore * sizeof(float));

	GMesh result = {
		.streamCount = 1,
		.indexOffset = g_indexOffset,
		.vertexOffset = g_vertexOffset,
		.vertexCount = m->mNumVertices,
		.streamOffset = { g_vertexOffset * streamElementSize },
		.streamElementSize = { streamElementSize }
	};

	// Original data for LOD calculation
	std::vector<float> srcVertices;
	std::vector<uint32_t> srcIndices;

	std::vector<std::vector<uint32_t>> outLods;

	auto& vertices = g_MeshData.vertexData_;

	for (size_t i = 0; i != m->mNumVertices; i++)
	{
		const aiVector3D v = m->mVertices[i];
		const aiVector3D n = m->mNormals[i];
		const aiVector3D t = hasTexCoords ? m->mTextureCoords[0][i] : aiVector3D();

		if (loadLods)
		{
			srcVertices.push_back(v.x);
			srcVertices.push_back(v.y);
			srcVertices.push_back(v.z);
		}

		vertices.push_back(v.x * scale);
		vertices.push_back(v.y * scale);
		vertices.push_back(v.z * scale);

		vertices.push_back(t.x);
		vertices.push_back(1.0f - t.y);

		glm::vec2 och = octahedral_encode(glm::vec3(n.x,n.y,n.z));
		vertices.push_back(och.x);
		vertices.push_back(och.y);
	}

	for (size_t i = 0; i != m->mNumFaces; i++)
	{
		if (m->mFaces[i].mNumIndices != 3)
			continue;
		for (unsigned j = 0; j != m->mFaces[i].mNumIndices; j++)
			srcIndices.push_back(m->mFaces[i].mIndices[j]);
	}

	if (!loadLods)
		outLods.push_back(srcIndices);
	else
		process_lods(srcIndices, srcVertices, outLods,7);

	printf("\nCalculated LOD count: %u\n", (unsigned)outLods.size());

	uint32_t numIndices = 0;

	for (size_t l = 0; l < outLods.size(); l++)
	{
		for (size_t i = 0; i < outLods[l].size(); i++)
			g_MeshData.indexData_.push_back(outLods[l][i]);

		result.lodOffset[l] = numIndices;
		numIndices += (int)outLods[l].size();
	}

	result.lodOffset[outLods.size()] = numIndices;
	result.lodCount = (uint32_t)outLods.size();

	g_indexOffset += numIndices;
	g_vertexOffset += m->mNumVertices;

	return result;

}

GMesh GMeshEncoder::ai_mesh_to_gmesh(const aiMesh* m, bool loadLods, int scale, MeshData& mesh, uint32_t& vertexOffset, uint32_t& indexOffset)
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

	GMesh result = {
		.streamCount = 1,
		.indexOffset = indexOffset,
		.vertexOffset = vertexOffset,
		.vertexCount = m->mNumVertices,
		.streamOffset = { vertexOffset * streamElementSize },
		.streamElementSize = { streamElementSize }
	};


	// Original data for LOD calculation
	std::vector<float> srcVertices;
	std::vector<uint32_t> srcIndices;

	std::vector<std::vector<uint32_t>> outLods;

	auto& vertices = mesh.vertexData_;

	for (size_t i = 0; i != m->mNumVertices; i++)
	{
		const aiVector3D v = m->mVertices[i];
		const aiVector3D n = m->mNormals[i];
		const aiVector3D tangent = m->mTangents[i];
		const aiVector3D bitangent = m->mBitangents[i];
		aiVector3D t = hasTexCoords ? m->mTextureCoords[0][i] : aiVector3D();

		if (loadLods)
		{
			srcVertices.push_back(v.x);
			srcVertices.push_back(v.y);
			srcVertices.push_back(v.z);
		}

		vertices.push_back(v.x * scale);
		vertices.push_back(v.y * scale);
		vertices.push_back(v.z * scale);
		vertices.push_back(t.x);
		vertices.push_back(1.0f - t.y);

		if (n.x > 1.f || n.x < -1.f || n.y > 1.f || n.y < -1.f || n.z > 1.f || n.z < -1.f)
		{
			assert(false);
		}
		glm::vec2 och = octahedral_encode(glm::vec3(n.x, n.y, n.z));
		vertices.push_back(och.x);
		vertices.push_back(och.y);
		if(m->HasTangentsAndBitangents())
		{ 
			vertices.push_back(tangent.x);
			vertices.push_back(tangent.y);
			vertices.push_back(tangent.z);
			vertices.push_back(bitangent.x);
			vertices.push_back(bitangent.y);
			vertices.push_back(bitangent.z);
		}
	}

	for (size_t i = 0; i != m->mNumFaces; i++)
	{
		if (m->mFaces[i].mNumIndices != 3)
			continue;
		for (unsigned j = 0; j != m->mFaces[i].mNumIndices; j++)
			srcIndices.push_back(m->mFaces[i].mIndices[j]);
	}

	if (!loadLods)
		outLods.push_back(srcIndices);
	else
		process_lods(srcIndices, srcVertices, outLods, numElementsToStore);

	printf("\nCalculated LOD count: %u\n", (unsigned)outLods.size());

	uint32_t numIndices = 0;

	for (size_t l = 0; l < outLods.size(); l++)
	{
		for (size_t i = 0; i < outLods[l].size(); i++)
			mesh.indexData_.push_back(outLods[l][i]);

		result.lodOffset[l] = numIndices;
		numIndices += (int)outLods[l].size();
	}

	result.lodOffset[outLods.size()] = numIndices;
	result.lodCount = (uint32_t)outLods.size();

	indexOffset += numIndices;
	vertexOffset += m->mNumVertices*numElementsToStore;
	result.meshFlag = meshFlag;
	return result;
}
