#include "internal/engine/resource/gresource.h"

GResource::GResource(ResourceImplUniquePtr resource)
{
	m_loadingState.store(RESOURCE_LOADING_STATE_UNLOADED);
	m_implementer = std::move(resource);
	m_size = 0;
}
