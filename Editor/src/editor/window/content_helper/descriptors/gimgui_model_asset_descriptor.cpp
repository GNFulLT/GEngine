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
GImGuiModelAssetDescriptor::GImGuiModelAssetDescriptor()
{
	m_encoder.reset(new GMeshEncoder(EditorApplicationImpl::get_instance()->get_editor_log_window_logger().get()));
	m_supportedTypes.push_back(FILE_TYPE_GLB);
	m_supportedTypes.push_back(FILE_TYPE_OBJ);
	m_supportedTypes.push_back(FILE_TYPE_GLTF);

}

const std::vector<FILE_TYPE>* GImGuiModelAssetDescriptor::get_file_types()
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
