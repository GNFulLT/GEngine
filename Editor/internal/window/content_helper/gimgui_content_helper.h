#ifndef GIMGUI_CONTENT_HELPER_H
#define GIMGUI_CONTENT_HELPER_H



#include "public/core/templates/unordered_dense.h"
#include "gimgui_content_descriptor.h"
#include <vector>
#include <memory>
#include <unordered_map>

class GImGuiContentHelper
{
public:
	GImGuiContentHelper();

	void destroy();
	
	void register_descriptor(IGImGuiContentDescriptorImpl* impl);

	const std::vector<GImGuiContentDescriptor*>* get_descriptor_of_type_if_any(FILE_TYPE type);
private:
	ankerl::unordered_dense::segmented_map<FILE_TYPE, std::vector<GImGuiContentDescriptor*>> m_descriptorMap;

	std::vector<GImGuiContentDescriptor*> m_allDescriptors;

};

#endif // GIMGUI_CONTENT_HELPER_H