#ifndef GSCENE2_H
#define GSCENE2_H

#include "engine/GEngine_EXPORT.h"
#include <vector>
#include <glm/glm.hpp>
#include <cstdint>

#define NODE_HANDLER_NONE 0xFFFFFFFF

struct GNode_2
{
	uint32_t childrenCount;
	std::vector<uint32_t> childrens;
	//X Mesh contains the transform data
	uint32_t meshIndex = NODE_HANDLER_NONE;
	uint32_t level = 0;
};

class ENGINE_API GScene_2
{
public:

private:
	std::vector<GNode_2> _nodes;
};

#endif // GSCENE2_H