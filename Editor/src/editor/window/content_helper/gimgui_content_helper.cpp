#include "internal/window/content_helper/gimgui_content_helper.h"

GImGuiContentHelper::GImGuiContentHelper()
{
}

void GImGuiContentHelper::destroy()
{
	for (int i = 0; i < m_allDescriptors.size(); i++)
	{
		m_allDescriptors[i]->destroy();
		delete m_allDescriptors[i];
	}
}

void GImGuiContentHelper::register_descriptor(IGImGuiContentDescriptorImpl* impl)
{
	//X TODO : CHECK NAME

	auto descriptor = new GImGuiContentDescriptor(impl);
	
	auto types = descriptor->get_file_type();

	for (auto type : *types)
	{
		auto iter = m_descriptorMap.find(type);
		if (iter == m_descriptorMap.end())
		{
			m_descriptorMap.emplace(type, std::vector<GImGuiContentDescriptor*>());
		}
		iter = m_descriptorMap.find(type);
		iter->second.push_back(descriptor);
	}

	m_allDescriptors.emplace_back(descriptor);
}

const std::vector<GImGuiContentDescriptor*>* GImGuiContentHelper::get_descriptor_of_type_if_any(const std::string& extension)
{
	auto iter = m_descriptorMap.find(std::string(extension));
	return iter == m_descriptorMap.end() ? nullptr : &iter->second;
}
