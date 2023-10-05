#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include "glm/glm.hpp"
#include "engine/rendering/scene/scene_node.h"
#include <unordered_map>
#include <string>
#include <string_view>
#include "engine/GEngine_EXPORT.H"
#include <expected>
#include "engine/rendering/scene/scene_config.h"
#include "engine/rendering/material/gmaterial.h"
//X Will be changed
#include <queue>
enum SCENE_DECODE_ERROR
{
	SCENE_DECODE_ERROR_UNKNOWN
};



struct Scene {
	inline constexpr static const std::string_view EXTENSION_NAME = ".gscene";
	inline constexpr static const int MAX_NODE_LEVEL = 16;

	std::vector<glm::mat4> localTransform_;
	std::vector<glm::mat4> globalTransform_;
	std::vector<Hierarchy> hierarchy;

	//X Maps
	std::unordered_map<uint32_t, uint32_t> meshes_;
	std::unordered_map<uint32_t, uint32_t> materialForNode_;
	std::unordered_map<uint32_t, uint32_t> nameForNode_;
	std::unordered_map<uint32_t, uint32_t> drawIdForNode;
	std::vector<std::string> names_;
	std::vector<std::string> materialNames_;

	std::vector<int> changedAtThisFrame_[MAX_NODE_LEVEL];
	std::queue<int> changedNodesAtThisFrame_;

	ENGINE_API void mark_as_changed(int nodeId);
	ENGINE_API bool recalculate_transforms();

	//X Discouraged way to change matrix
	ENGINE_API glm::mat4* get_matrix_of(uint32_t nodeID);
	ENGINE_API static Scene* create_scene_with_default_material(std::vector<MaterialDescription>& mat);
	ENGINE_API static int add_node(Scene& scene, int parent, int level);
	ENGINE_API static bool save_the_scene(const Scene& scene, const char* filePath);
	ENGINE_API static std::expected<Scene*, SCENE_DECODE_ERROR> load_the_scene(const char* filePath);
};

#endif // SCENE_H