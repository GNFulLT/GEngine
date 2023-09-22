#ifndef GMESH_FILE_H
#define GMESH_FILE_H

#include "engine/rendering/mesh/mesh_constants.h"

//X  File layout :  
//X  GMeshFileHeader
//X  Indices  (Size is indicated in the header)
//X  Vertexes (Size is indicated in the header) 

//X IndexDataSize and VertexDataSize indicates in byte size not uint32_t word count !!!!!
struct GMeshFileHeader final {
	uint32_t magicValue = MeshConstants::MESH_FILE_HEADER_MAGIC_VALUE;
	uint32_t meshCount;
	uint32_t dataBlockStartOffset;
	uint32_t indexDataSize;
	uint32_t vertexDataSize;

};


#endif // GMESH_DECODER_H