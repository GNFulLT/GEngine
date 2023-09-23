#ifndef GMESH_H
#define GMESH_H

#include "engine/rendering/mesh/mesh_constants.h"
#include <expected>
#include <vector>
#include "engine/GEngine_EXPORT.h"

enum GMESH_DECODE_ERROR
{
	GMESH_DECODE_ERROR_UNKNOWN,
	GMESH_DECODE_ERROR_FILE_NOT_EXIST,
	GMESH_DECODE_ERROR_FILE_FORMAT_UNKNOWN,
	GMESH_DECODE_ERROR_FILE_CORRUPTED
};


struct GMesh final
{
	uint32_t lodCount = 1;
	uint32_t streamCount = 0;

	/* The total count of all previous vertices in this mesh file */
	uint32_t indexOffset = 0;
	uint32_t vertexOffset = 0;

	/* Vertex count (for all LODs) */
	uint32_t vertexCount = 0;
	/* Offsets to LOD data. Last offset is used as a marker to calculate the size */
	uint32_t lodOffset[MeshConstants::MAX_LOD_COUNT] = { 0 };

	/* All the data "pointers" for all the streams */
	uint32_t streamOffset[MeshConstants::MAX_STREAM_COUNT] = { 0 };

	/* Information about stream element (size pretty much defines everything else, the "semantics" is defined by the shader) */
	uint32_t streamElementSize[MeshConstants::MAX_STREAM_COUNT] = { 0 };


	//X TODO : Stream Data Type will be added

	inline uint32_t getLODIndicesCount(uint32_t lod) const { return lodOffset[lod + 1] - lodOffset[lod]; }

};

#endif // GMESH