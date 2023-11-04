#ifndef GMESH_DATA_H
#define GMESH_DATA_H

#include <vector>
#include <cstdint>

#include "engine/rendering/mesh/gmesh.h"
#include "engine/rendering/bounding_box.h"

struct MeshData
{
	std::vector<uint32_t> indexData_;
	std::vector<float> vertexData_;
	std::vector<GMesh> meshes_;
	std::vector<BoundingBox> boxes_;
};

struct GMeshletData
{

	std::vector<uint32_t> indexData_;
	std::vector<float> vertexData_;
	std::vector<uint32_t> meshletVertexData_;
	std::vector<uint8_t> meshletTriangleData_;
	std::vector<GMeshlet> gmeshlets_;
	std::vector<GMeshMeshlet> gmeshMeshlets_;
};

ENGINE_API std::expected<MeshData*, GMESH_DECODE_ERROR> decode_file(const char* filePath);
ENGINE_API void recalculateBoundingBoxes(MeshData& m);
ENGINE_API uint32_t calculateVertexElementCount(uint64_t meshFlag);

struct GMeshData
{
	BoundingBox boundingBox;
	glm::vec4 extent;
	uint32_t lodCount = 1;
	/* Beginning of the index in the array */
	uint32_t indexOffset = 0;
	/* Beginning of the vertex in the array */
	uint32_t vertexOffset = 0;
	/* Vertex count (for all LODs) */
	uint32_t vertexCount = 0;
	/* Offsets to LOD data. Last offset is used as a marker to calculate the size */
	/* Calculating global index for this lod should be like : GMeshData::indexOffset */
	uint32_t lodOffset[MeshConstants::MAX_LOD_COUNT] = { 0 };

	uint64_t meshFlag = 0;
	inline uint32_t getLODIndicesCount(uint32_t lod) const { return lodOffset[lod + 1] - lodOffset[lod]; }

};

struct GMeshMeshletData
{
	uint32_t lodCount = 1;
	/* The total count of all previous vertices in this mesh file */
	uint32_t indexOffset = 0;
	uint32_t vertexOffset = 0;

	/* Vertex count (for all LODs) */
	uint32_t vertexCount = 0;
	/* Offsets to LOD data. Last offset is used as a marker to calculate the size */
	uint32_t lodOffset[MeshConstants::MAX_LOD_COUNT] = { 0 };

	//X TODO : Stream Data Type will be added
	uint64_t meshFlag = 0;

	uint32_t meshletOffset = 0;
	uint32_t meshletVerticesOffset = 0;
	uint32_t meshletTrianglesOffset = 0;

	uint32_t meshletCount = 0;
	uint32_t meshletVerticesCount = 0;
	uint32_t meshletTrianglesCount = 0;

	inline uint32_t getLODIndicesCount(uint32_t lod) const { return lodOffset[lod + 1] - lodOffset[lod]; }

};
#endif // GMESH_DATA_H