#ifndef GMESH_RENDERABLE_H
#define GMESH_RENDERABLE_H

#include <cstdint>


struct InstanceData {
	uint32_t mesh;
	uint32_t material;
	uint32_t lod;
	uint32_t indexOffset;
	uint32_t vertexOffset;
	uint32_t transformIndex;
};

struct DrawData
{
	uint32_t mesh;
	uint32_t material;
	uint32_t transformIndex;
};

#endif // GMESH_RENDERABLE_H