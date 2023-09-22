#ifndef SCENE_CONFIG_H
#define SCENE_CONFIG_H

#include <string>

struct SceneConfig
{
	std::string fileName;
	std::string outputMesh;
	std::string outputScene;
	std::string outputMaterials;
	float scale;
	bool calculateLODs;
	bool mergeInstances;
};

#endif // SCENE_CONFIG_H