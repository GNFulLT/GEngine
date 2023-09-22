#ifndef SCENE_NODE_H
#define SCENE_NODE_H

#include <cstdint>

struct Hierarchy
{
	//X Hierarchy
	uint32_t parent;
	uint32_t firstChild;
	uint32_t nextSibling;
	uint32_t lastSibling;
	uint32_t level;
};


struct SceneNode
{
	uint32_t mesh;
	uint32_t material;

	
};

#endif // SCENE_NODE_H