#ifndef GMESHLET_H
#define GMESHLET_H

#include <cstdint>

struct GMeshlet
{
	uint32_t vertexOffset;
	uint32_t triangleOffset;
	uint32_t vertexCount;
	uint32_t triangleCount;

	float center[3];
	float radius;
	signed char coneAxis[3];
	signed char coneCutoff;
};

#endif // GMESHLET_H