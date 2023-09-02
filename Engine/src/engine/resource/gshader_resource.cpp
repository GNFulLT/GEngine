#include "volk.h"

#include "internal/engine/resource/gshader_resource.h"
#include "internal/engine/shader/spirv_shader_utils.h"
#include "engine/gengine.h"
#include "engine/imanager_table.h"
#include "engine/manager/igshader_manager.h"
#include "engine/rendering/vulkan/ivulkan_device.h"
#include "engine/manager/igresource_manager.h"

GShaderResource::GShaderResource(IGVulkanLogicalDevice* device, std::string_view path)
{
	m_setLayout = nullptr;
	m_path = path;
	m_vkShaderModule = nullptr;
	m_boundedDevice = device;
}

RESOURCE_INIT_CODE GShaderResource::prepare_impl()
{
	return RESOURCE_INIT_CODE_OK;
}

void GShaderResource::unprepare_impl()
{
	m_shaderText.clear();
}

RESOURCE_INIT_CODE GShaderResource::load_impl()
{
	std::filesystem::path path(m_path);
	auto shaderManager = ((GSharedPtr<IGShaderManager>*)GEngine::get_instance()->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_SHADER))->get();
	if (!std::filesystem::exists(path))
	{
		return RESOURCE_INIT_CODE_FILE_NOT_FOUND;
	}

	ISpirvShader* spirvShader = nullptr;
	if (endsWith(m_path.c_str(), ".spv"))
	{
		// Load Bytes
		auto res = read_shader_bytes(path);
		if (!res.has_value())
		{
			return RESOURCE_INIT_CODE_UNKNOWN_EX;
		}
		auto shader = res.value();
		
		auto stage = get_stage_from_spirv_file_name(m_path.c_str());

		if (stage == SPIRV_SHADER_STAGE_UNKNOWN)
		{
			return RESOURCE_INIT_CODE_UNKNOWN_EX;
		}

		auto spirvRes = shaderManager->load_shader_from_bytes(shader, stage);
		if (!spirvRes.has_value())
		{
			return RESOURCE_INIT_CODE_UNKNOWN_EX;
		}

		spirvShader = spirvRes.value();
	}
	else
	{
		//X LOAD FROM TEXT
		auto res = read_shader_file(m_path.c_str());
		if (!res.has_value())
		{
			RESOURCE_INIT_CODE_UNKNOWN_EX;
		}

		auto txt = res.value();
		
		auto stageType = shader_stage_from_file_name(m_path.c_str());

		if (stageType.first == SPIRV_SHADER_STAGE_UNKNOWN || stageType.second == SPIRV_SOURCE_TYPE_UNKNOWN)
		{
			return RESOURCE_INIT_CODE_UNKNOWN_EX;
		}

		auto spirvRes = shaderManager->compile_shader_text(txt, stageType.first, stageType.second);
		
		if (!spirvRes.has_value())
		{
			return RESOURCE_INIT_CODE_UNKNOWN_EX;
		}

		spirvShader = spirvRes.value();
	}
	auto bindingsRes = shaderManager->get_layout_bindings_from(spirvShader);
	
	if (!bindingsRes.has_value())
	{
		delete spirvShader;
		return RESOURCE_INIT_CODE_UNKNOWN_EX;
	}


	auto bindings = bindingsRes.value();

	if (bindings.size() > 0)
	{

		VkDescriptorSetLayoutCreateInfo crtInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.bindingCount = uint32_t(bindings.size()),
			.pBindings = bindings.data()
		};

		auto desSuc = vkCreateDescriptorSetLayout(m_boundedDevice->get_vk_device(), &crtInfo, nullptr, &m_setLayout);
		if (desSuc != VK_SUCCESS)
		{
			delete spirvShader;
			return RESOURCE_INIT_CODE_UNKNOWN_EX;
		}
	}


	VkShaderModuleCreateInfo createInfo = {};
	createInfo.pNext = nullptr;
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pCode = spirvShader->get_spirv_words();
	createInfo.codeSize = spirvShader->get_size();

	auto res = vkCreateShaderModule(m_boundedDevice->get_vk_device(), &createInfo, nullptr, &m_vkShaderModule);

	m_stage = spirvShader->get_spirv_stage();
	m_entyPointName = std::string(spirvShader->get_entry_point_name());



	delete spirvShader;

	if (res != VK_SUCCESS)
	{
		return RESOURCE_INIT_CODE_UNKNOWN_EX;
	}

	m_size = createInfo.codeSize;

	return RESOURCE_INIT_CODE_OK;
}

void GShaderResource::unload_impl()
{
	if (m_vkShaderModule != nullptr)
	{
		vkDestroyShaderModule(m_boundedDevice->get_vk_device(), m_vkShaderModule, nullptr);
	}
}

std::uint64_t GShaderResource::calculateSize() const
{
	return m_size;
}

std::string_view GShaderResource::get_resource_path() const
{
	return m_path.c_str();
}

void GShaderResource::destroy_impl()
{
	m_creatorOwner->destroy_shader_resource(this);
}

VkShaderModule_T* GShaderResource::get_vk_shader_module()
{
	return m_vkShaderModule;
}

SPIRV_SHADER_STAGE GShaderResource::get_shader_stage()
{
	return m_stage;
}

const char* GShaderResource::get_entry_point_name()
{
	return m_entyPointName.c_str();
}
