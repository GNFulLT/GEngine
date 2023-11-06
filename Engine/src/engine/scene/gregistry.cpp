#include "internal/engine/scene/gregistry.h"

GEntity& GRegistry::create_entity(const char* name)
{
	uint32_t id = m_entityMap.size();
	if (name != nullptr)
		m_entityMap.emplace_back(m_entityRegistry.create(), &m_entityRegistry, name);
	else
		m_entityMap.emplace_back(m_entityRegistry.create(), &m_entityRegistry);
	return m_entityMap[id];
}

GEntity* GRegistry::get_entity_by_index(uint32_t index)
{
	if (index >= m_entityMap.size())
		return nullptr;
	return &m_entityMap[index];
}
