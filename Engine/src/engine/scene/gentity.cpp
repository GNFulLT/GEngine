#include "internal/engine/scene/gentity.h"

GEntity::GEntity()
{
	
}

GEntity::GEntity(entt::entity handler, entt::registry* registeryRef)
{
	m_entityHandler = handler;
	m_registeryRef = registeryRef;
}

GEntity::GEntity(entt::entity handler, entt::registry* registeryRef, const char* tag) : GEntity(handler,registeryRef)
{
	m_tag = tag;
}

bool GEntity::is_valid() const noexcept
{
	return m_registeryRef != nullptr && m_registeryRef->valid(m_entityHandler);
}
