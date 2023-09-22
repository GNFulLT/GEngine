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
ENGINE_API std::expected<MeshData*, GMESH_DECODE_ERROR> decode_file(const char* filePath);
ENGINE_API void recalculateBoundingBoxes(MeshData& m);


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
	inline uint32_t getLODIndicesCount(uint32_t lod) const { return lodOffset[lod + 1] - lodOffset[lod]; }

};
#endif // GMESH_DATA_H