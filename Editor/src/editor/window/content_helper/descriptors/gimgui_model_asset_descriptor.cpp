#include "internal/window/content_helper/descriptors/gimgui_model_asset_descriptor.h"
#include <imgui/imgui.h>
#include "editor/editor_application_impl.h"
#include "engine/io/iowning_glogger.h"
#include "internal/rendering/scene/scene_converter.h"
#include "engine/gengine.h"
#include "engine/imanager_table.h"
#include "public/core/templates/shared_ptr.h"
#include "engine/manager/igresource_manager.h"
#include "engine/resource/igtexture_resource.h"
#include "volk.h"
#include "engine/manager/igscene_manager.h"
#include "engine/gengine.h"
#include "engine/imanager_table.h"
#include <unordered_map>
#include "engine/manager/igscene_manager.h"

GImGuiModelAssetDescriptor::GImGuiModelAssetDescriptor()
{
	m_encoder.reset(new GMeshEncoder(EditorApplicationImpl::get_instance()->get_editor_log_window_logger().get()));
	m_supportedTypes.push_back(".glb");
	m_supportedTypes.push_back(".obj");
	m_supportedTypes.push_back(".gltf");
	m_supportedTypes.push_back(".blend");
	m_supportedTypes.push_back(".dae");
	m_sceneManager = ((GSharedPtr<IGSceneManager>*)EditorApplicationImpl::get_instance()->m_engine->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_SCENE))->get();
}

const std::vector<std::string>* GImGuiModelAssetDescriptor::get_file_types()
{
	return &m_supportedTypes;
}
static auto* con = new SceneConverter();
void GImGuiModelAssetDescriptor::draw_menu_for_file(std::filesystem::path path)
{
	if (ImGui::Selectable("Convert to gmesh and save"))
	{
		SceneConfig  conf;
		conf.calculateLODs = true;
		conf.fileName = path.string();
		conf.scale = 1;
		conf.outputMaterials = "./allmaterials.gmaterial";
		conf.outputScene = "./scene";
		conf.outputMesh = "./mesh.gmesh";
		con->process_scene(conf);
	}
	if (ImGui::Selectable("Add to scene"))
	{
		auto sceneManager = ((GSharedPtr<IGSceneManager>*)EditorApplicationImpl::get_instance()->m_engine->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_SCENE))->get();
		sceneManager->load_scene(path);
		return;

		std::unordered_map<uint32_t, std::unordered_map<TEXTURE_MAP_TYPE, std::string>> texturePaths;
		Scene* convertedScene;
		std::vector<MaterialDescription> materials;
		auto filePath = path.string();
		std::unordered_map<uint32_t, glm::mat4> outTransforms;
		
		auto mesh = con->load_all_meshes_meshlet(filePath.c_str(), materials, texturePaths, &convertedScene, outTransforms);
		auto lastMeshlet = mesh->gmeshlets_[mesh->gmeshlets_.size() - 1];
		//auto mesh = con->load_all_meshes(filePath.c_str(), materials, texturePaths,&convertedScene);

		auto& lastgmeshlet = mesh->gmeshMeshlets_[mesh->gmeshMeshlets_.size() - 1];
		
		auto beginVertexFloat = lastgmeshlet.vertexOffset;
		
		auto currentMeshlet = lastgmeshlet.meshletCount - 1;
		
		auto currentGMeshlet = mesh->gmeshlets_[lastgmeshlet.meshletOffset + currentMeshlet];
		auto currentIndex = mesh->meshletVertexData_[(currentGMeshlet.vertexOffset + currentGMeshlet.vertexCount - 1)];
		auto stride = currentIndex * 13;

		auto currentBeginFloat = beginVertexFloat + stride;

		size_t count = mesh->vertexData_.size() - currentBeginFloat;

		//m_sceneManager->add_node_with_mesh_and_defaults(meshIndex);
		//X Generate draw datas for mesh
		if (mesh != nullptr)
		{
			uint32_t meshIndex = m_sceneManager->add_meshlet_to_scene(mesh);
			std::unordered_map<uint32_t, uint32_t> aiMatToSceneMat;
			for (int i = 0; i < materials.size(); i++)
			{
				if (auto paths = texturePaths.find(i); paths != texturePaths.end())
				{
					//X Load the texture
					//X Albedo
					auto itr = paths->second.find(TEXTURE_MAP_TYPE_ALBEDO);
					if (itr != paths->second.end())
					{
						auto albedo = itr->second;
						std::string albedoTexturePath = "./";
						albedoTexturePath += albedo;
						auto resource = ((GSharedPtr<IGResourceManager>*)(EditorApplicationImpl::get_instance()->m_engine->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_RESOURCE)))->get();
						auto textureRes = resource->create_texture_resource(albedo, "editor", albedoTexturePath, nullptr, VK_FORMAT_R8G8B8A8_UNORM).value();
						assert(RESOURCE_INIT_CODE_OK == textureRes->load());
						//X Save the texture to the scene
						auto albedoTextureID = m_sceneManager->register_texture_to_scene(textureRes);
						materials[i].albedoMap_ = albedoTextureID;
					}
					//X Normal
					itr = paths->second.find(TEXTURE_MAP_TYPE_NORMAL);
					if (itr != paths->second.end())
					{
						auto normal = itr->second;
						std::string normalTexturePath = "./";
						normalTexturePath += normal;
						auto resource = ((GSharedPtr<IGResourceManager>*)(EditorApplicationImpl::get_instance()->m_engine->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_RESOURCE)))->get();
						auto textureRes = resource->create_texture_resource(normal, "editor", normalTexturePath, nullptr, VK_FORMAT_R8G8B8A8_UNORM).value();
						assert(RESOURCE_INIT_CODE_OK == textureRes->load());
						//X Save the texture to the scene
						auto normalTextureId = m_sceneManager->register_texture_to_scene(textureRes);
						materials[i].normalMap_ = normalTextureId;
					}
					//X AO
					itr = paths->second.find(TEXTURE_MAP_TYPE_AMBIENT_OCCLUSION);
					if (itr != paths->second.end())
					{
						auto ao = itr->second;
						std::string aoTexturePath = "./";
						aoTexturePath += ao;
						auto resource = ((GSharedPtr<IGResourceManager>*)(EditorApplicationImpl::get_instance()->m_engine->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_RESOURCE)))->get();
						auto textureRes = resource->create_texture_resource(ao, "editor", aoTexturePath, nullptr, VK_FORMAT_R8G8B8A8_UNORM).value();
						assert(RESOURCE_INIT_CODE_OK == textureRes->load());
						auto aoId = m_sceneManager->register_texture_to_scene(textureRes);
						materials[i].ambientOcclusionMap_ = aoId;
					}
					//X Roughness
					itr = paths->second.find(TEXTURE_MAP_TYPE_ROUGHNESS);
					if (itr != paths->second.end())
					{
						auto roughness = itr->second;
						std::string roughnessTexturePath = "./";
						roughnessTexturePath += roughness;
						auto resource = ((GSharedPtr<IGResourceManager>*)(EditorApplicationImpl::get_instance()->m_engine->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_RESOURCE)))->get();
						auto textureRes = resource->create_texture_resource(roughness, "editor", roughnessTexturePath, nullptr, VK_FORMAT_R8G8B8A8_UNORM).value();
						assert(RESOURCE_INIT_CODE_OK == textureRes->load());
						auto roughnesssId = m_sceneManager->register_texture_to_scene(textureRes);
						materials[i].roughnessMap_ = roughnesssId;
					}
					//X Metallic
					itr = paths->second.find(TEXTURE_MAP_TYPE_METALNESS);
					if (itr != paths->second.end())
					{
						auto metallic = itr->second;
						std::string metallicTexturePath = "./";
						metallicTexturePath += metallic;
						auto resource = ((GSharedPtr<IGResourceManager>*)(EditorApplicationImpl::get_instance()->m_engine->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_RESOURCE)))->get();
						auto textureRes = resource->create_texture_resource(metallic, "editor", metallicTexturePath, nullptr, VK_FORMAT_R8G8B8A8_UNORM).value();
						assert(RESOURCE_INIT_CODE_OK == textureRes->load());
						auto metallicId = m_sceneManager->register_texture_to_scene(textureRes);
						materials[i].metallicyMap_ = metallicId;
					}
					//X Emissive
					itr = paths->second.find(TEXTURE_MAP_TYPE_EMISSIVE);
					if (itr != paths->second.end())
					{
						auto emissive = itr->second;
						std::string emissiveTexturePath = "./";
						emissiveTexturePath += emissive;
						auto resource = ((GSharedPtr<IGResourceManager>*)(EditorApplicationImpl::get_instance()->m_engine->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_RESOURCE)))->get();
						auto textureRes = resource->create_texture_resource(emissive, "editor", emissiveTexturePath, nullptr, VK_FORMAT_R8G8B8A8_UNORM).value();
						assert(RESOURCE_INIT_CODE_OK == textureRes->load());
						auto emissiveId = m_sceneManager->register_texture_to_scene(textureRes);
						materials[i].emissiveMap_ = emissiveId;
					}
					auto materialIndex = m_sceneManager->add_material_to_scene(&materials[i]);
					aiMatToSceneMat.emplace(i, materialIndex);
				}

			}
		
			//X Add to the real scene virtual data
			std::queue<uint32_t> queue;
			std::unordered_map<uint32_t, uint32_t> virtual_to_scene;
			queue.push(0);

			while (!queue.empty())
			{
				auto iter = queue.front();
				queue.pop();
				uint32_t nodeIndex = -1;
				if (auto mesh = convertedScene->meshes_.find(iter); mesh != convertedScene->meshes_.end())
				{
					auto localMeshIndex = mesh->second;
					uint32_t material = 0;
					if (auto mat = convertedScene->materialForNode_.find(iter); mat != convertedScene->materialForNode_.end())
					{
						material = aiMatToSceneMat.find(mat->second)->second;
					}
					uint32_t parent = 0;
					if (auto parentIter = virtual_to_scene.find(iter); parentIter != virtual_to_scene.end())
					{
						parent = parentIter->second;
					}
					nodeIndex = m_sceneManager->add_child_node_with_mesh_and_material_and_transform(parent,localMeshIndex + meshIndex, material, &outTransforms.find(iter)->second);
				}
				// Enqueue children (if any)
				uint32_t child = convertedScene->hierarchy[iter].firstChild;
				while (child != UINT32_MAX)
				{
					queue.push(child);
					if (nodeIndex != -1)
					{
						virtual_to_scene.emplace(child, nodeIndex);
					}
					child = convertedScene->hierarchy[child].nextSibling;
					
				}
			}
			delete convertedScene;
		}
		/*std::unordered_map<uint32_t,std::unordered_map<TEXTURE_MAP_TYPE, std::string>> texturePaths;
		std::vector<MaterialDescription> materials;
		auto filePath = path.string();
		auto mesh = con->load_all_meshes(filePath.c_str(), materials,texturePaths);
		if (mesh != nullptr)
		{
			uint32_t beginIndex = EditorApplicationImpl::get_instance()->m_engine->add_mesh(mesh);
			uint32_t meshCount = mesh->meshes_.size();
			EditorApplicationImpl::get_instance()->get_editor_logger()->log_d(fmt::format("Loaded mesh begin index is : {}, Mesh Count : {}",beginIndex, meshCount).c_str());
			for (int i = 0; i < meshCount; i++)
			{
				auto index = beginIndex + i;
				auto nodeID = EditorApplicationImpl::get_instance()->m_engine->add_node_with_mesh(index);
				if (auto paths = texturePaths.find(i); paths != texturePaths.end())
				{
					auto itr = paths->second.find(TEXTURE_MAP_TYPE_ALBEDO);
					if (itr == paths->second.end())
						return;
					auto albedo = itr->second;
					std::string albedoTexturePath = "./";
					albedoTexturePath += albedo;
					auto resource = ((GSharedPtr<IGResourceManager>*)(EditorApplicationImpl::get_instance()->m_engine->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_RESOURCE)))->get();
					auto textureRes = resource->create_texture_resource(albedo, "editor", albedoTexturePath, nullptr, VK_FORMAT_R8G8B8A8_UNORM).value();
					assert(RESOURCE_INIT_CODE_OK == textureRes->load());
					auto textureID = EditorApplicationImpl::get_instance()->m_engine->add_texture(textureRes);
					materials[i].albedoMap_ = textureID;
					materials[i].albedoColor_.x = 0.f;
					materials[i].albedoColor_.z = 1.f;

					uint32_t materialIndex = EditorApplicationImpl::get_instance()->m_engine->add_material(materials[i]);
					EditorApplicationImpl::get_instance()->m_engine->set_material_for_node(nodeID, materialIndex);
				}

			}
			
		}*/
	}
}
