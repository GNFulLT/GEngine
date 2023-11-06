#include "engine/rendering/scene/scene.h"
#include <filesystem>
#include <fstream>
#include <map>

#include "internal/engine/scene/gregistry.h"
#include "internal/engine/scene/component/transform_component.h"


void save_scene_map(std::ofstream& ofstream,const Scene& scene,const std::unordered_map<uint32_t,uint32_t>& map)
{
	std::vector<uint32_t> ms;
	ms.reserve(map.size() * 2);
	for (const auto& m : map)
	{
		ms.push_back(m.first);
		ms.push_back(m.second);
	}
	const uint32_t size = static_cast<uint32_t>(ms.size());
	ofstream.write((char*)&size, sizeof(std::uint32_t));
	ofstream.write((char*)ms.data(), ms.size() * sizeof(std::uint32_t));
}

void load_scene_map(std::ifstream& ifstream, const Scene& scene, std::unordered_map<uint32_t, uint32_t>& map)
{
	std::vector<uint32_t> ms;

	uint32_t sz = 0;
	ifstream.read((char*)&sz, sizeof(uint32_t));
	ms.resize(sz);
	ifstream.read((char*)ms.data(), sz * sizeof(uint32_t));
	for (size_t i = 0; i < (sz / 2); i++)
		map[ms[i * 2 + 0]] = ms[i * 2 + 1];
}

Scene::Scene()
{
	m_registry = new GRegistry();
}

Scene::~Scene()
{
	delete m_registry;
}

ENGINE_API glm::mat4* Scene::get_global_transform(uint32_t nodeID)
{
	if (nodeID >= hierarchy.size())
		return nullptr;
	auto entity = m_registry->get_entity_by_index(nodeID);
	assert(entity != nullptr);
	if (auto transform = entity->get_or_null_component<TransformComponent>(); transform != nullptr)
	{
		return &transform->get_global_transform();
	}
}

ENGINE_API void Scene::mark_as_changed(int nodeId)
{
	int level = this->hierarchy[nodeId].level;
	this->changedAtThisFrame_[level].push_back(nodeId);
	for (int s = hierarchy[nodeId].firstChild; s != -1; s = hierarchy[s].nextSibling)
		mark_as_changed(s);
}

ENGINE_API bool Scene::recalculate_transforms()
{
	if (!changedAtThisFrame_[0].empty()) {
		int c = changedAtThisFrame_[0][0];
		auto entity = m_registry->get_entity_by_index(c);
		assert(entity != nullptr);
		if (entity->has_component<TransformComponent>())
		{
			auto& transform = entity->get_component<TransformComponent>();
			auto& globalTransform = transform.get_global_transform();
			globalTransform = transform.get_local_transform();
		}
		changedAtThisFrame_[0].clear();
	}
	// X Calculate matrixes per level
	bool isChanged = false;
	for (int i = 1; i < MAX_NODE_LEVEL && !changedAtThisFrame_[i].empty(); i++)
	{
		for (int c : changedAtThisFrame_[i]) {
			int p = hierarchy[c].parent;
			auto childEntity = m_registry->get_entity_by_index(c);
			auto parentEntity = m_registry->get_entity_by_index(p);
			assert(childEntity != nullptr && parentEntity != nullptr);
			if (childEntity->has_component<TransformComponent>())
			{
				auto& childTransform = childEntity->get_component<TransformComponent>();
				auto& cglobal = childTransform.get_global_transform();
				auto val = childTransform.get_local_transform();
				if (parentEntity->has_component<TransformComponent>())
				{
					val = parentEntity->get_component<TransformComponent>().get_global_transform() * val;
				}
				cglobal = val;
			}
			//globalTransform_[c] = globalTransform_[p] * localTransform_[c];
			changedNodesAtThisFrame_.push(c);
		}
		isChanged = true;
		changedAtThisFrame_[i].clear();
	}
	return isChanged;
	
}

ENGINE_API void Scene::set_transform_of(uint32_t nodeID, const glm::mat4& t)
{
	if (nodeID >= hierarchy.size())
		return;
	auto entity = m_registry->get_entity_by_index(nodeID);
	assert(entity != nullptr);
	if (auto transform = entity->get_or_null_component<TransformComponent>(); transform != nullptr)
	{
		auto& loc = transform->get_local_transform();
		loc = t;
	}
}

ENGINE_API glm::mat4* Scene::get_matrix_of(uint32_t nodeID)
{
	if (nodeID >= hierarchy.size())
		return nullptr;
	auto entity = m_registry->get_entity_by_index(nodeID);
	assert(entity != nullptr);
	if (auto transform = entity->get_or_null_component<TransformComponent>(); transform != nullptr)
	{
		return &transform->get_local_transform();
	}
	return nullptr;
}

ENGINE_API Scene* Scene::create_scene_with_default_material(std::vector<MaterialDescription>& mat)
{
	Scene* scene = new Scene();
	//X Root node
	add_node(*scene, -1, 0);
	MaterialDescription desc = {};
	desc.albedoColor_.x = 1.f;
	desc.albedoColor_.y = 0.f;
	desc.albedoColor_.z = 0.f;
	desc.albedoColor_.w = 1.f;

	mat.push_back(desc);
	return scene;
}

ENGINE_API int Scene::add_node(Scene& scene, int parent, int level)
{

	int node = (int)scene.hierarchy.size();
	{
		//X TODO: resize aux arrays (local/global etc.)
		//scene.localTransform_.push_back(glm::mat4(1.0f));
		//scene.globalTransform_.push_back(glm::mat4(1.0f));
		auto& entity = scene.m_registry->create_entity();
		entity.emplace_component<TransformComponent>();
	}

	Hierarchy hierarchy;
	hierarchy.parent = parent;
	hierarchy.lastSibling = -1;
	scene.hierarchy.push_back(hierarchy);
	if (parent > -1)
	{
		// find first item (sibling)
		int s = scene.hierarchy[parent].firstChild;
		if (s == -1)
		{
			scene.hierarchy[parent].firstChild = node;
			scene.hierarchy[node].lastSibling = node;
		}
		else
		{
			int dest = scene.hierarchy[s].lastSibling;
			if (dest <= -1)
			{
				// no cached lastSibling, iterate nextSibling indices
				for (dest = s; scene.hierarchy[dest].nextSibling != -1; dest = scene.hierarchy[dest].nextSibling);
			}
			scene.hierarchy[dest].nextSibling = node;
			scene.hierarchy[s].lastSibling = node;
		}
	}
	scene.hierarchy[node].level = level;
	scene.hierarchy[node].nextSibling = -1;
	scene.hierarchy[node].firstChild = -1;
	return node;
}

ENGINE_API bool Scene::save_the_scene(const Scene& scene, const char* filePath)
{
	std::string pth(filePath);
	pth += Scene::EXTENSION_NAME;
	std::ofstream ofstream(pth, std::ios::trunc | std::ios::binary | std::ios::out);
	ofstream.exceptions(std::ifstream::badbit);
	bool failed = false;
	try
	{
		uint32_t size = std::uint32_t(scene.hierarchy.size());
		ofstream.write((char*)&size, sizeof(std::uint32_t));
		//ofstream.write((char*)scene.localTransform_.data(),scene.localTransform_.size()*sizeof(glm::mat4));
		//ofstream.write((char*)scene.globalTransform_.data(), scene.globalTransform_.size() * sizeof(glm::mat4));
		ofstream.write((char*)scene.hierarchy.data(), scene.hierarchy.size() * sizeof(Hierarchy));

		save_scene_map(ofstream, scene,scene.materialForNode_);
		save_scene_map(ofstream, scene, scene.meshes_);

		//X TODO : SAVE NAMES
		ofstream.flush();
	}
	catch (const std::ofstream::failure& e)
	{
		failed = true;
	}
	catch (const std::exception& e)
	{
		failed = true;
	}

	ofstream.close();
	
	return !failed;
}

ENGINE_API std::expected<Scene*, SCENE_DECODE_ERROR> Scene::load_the_scene(const char* filePath)
{
	std::filesystem::path path(filePath);
	if (!std::filesystem::exists(path))
	{
		return std::unexpected(SCENE_DECODE_ERROR_UNKNOWN);
	}
	if (std::filesystem::is_directory(path) || strcmp(path.extension().string().c_str(), Scene::EXTENSION_NAME.data()) != 0)
	{
		return std::unexpected(SCENE_DECODE_ERROR_UNKNOWN);
	}

	std::ifstream ifstream(path, std::ios::in | std::ios::binary);

	//X TODO : GDNEWDA

	Scene* scene = new Scene();
	SCENE_DECODE_ERROR err = SCENE_DECODE_ERROR_UNKNOWN;
	bool failed = false;

	ifstream.exceptions(std::ifstream::badbit);
	//X TODO EXCEPTION HANDLING ETC
	try
	{
		uint32_t size = -1;
		ifstream.read((char*)&size, sizeof(uint32_t));
		//scene->localTransform_.resize(size);
		//scene->globalTransform_.resize(size);
		scene->hierarchy.resize(size);
		//ifstream.read((char*)scene->localTransform_.data(), scene->localTransform_.size() * sizeof(glm::mat4));
		//ifstream.read((char*)scene->globalTransform_.data(), scene->globalTransform_.size() * sizeof(glm::mat4));
		ifstream.read((char*)scene->hierarchy.data(), scene->hierarchy.size() * sizeof(Hierarchy));

		load_scene_map(ifstream, *scene, scene->materialForNode_);
		load_scene_map(ifstream, *scene, scene->meshes_);

		if (!ifstream.eof())
		{
			//X LOAD DEBUG NAMES 
		}
	}
	catch (const std::ifstream::failure& e)
	{
		failed = true;
	}
	catch (const std::exception& e)
	{
		failed = true;
	}

	ifstream.close();

	if (failed)
	{
		delete scene;
		return std::unexpected(err);
	}

	return scene;

}
