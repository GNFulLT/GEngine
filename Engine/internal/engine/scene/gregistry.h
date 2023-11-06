#ifndef GREGISTRY_H
#define GREGISTRY_H

#include <entt/entt.hpp>
#include "engine/GEngine_EXPORT.h"
#include "internal/engine/scene/gentity.h"

class GRegistry
{
public:
	GEntity& create_entity(const char* name = nullptr);

	GEntity* get_entity_by_index(uint32_t index);
private:
	std::vector<GEntity> m_entityMap;
	entt::registry m_entityRegistry;

};


#endif // GREGISTRY_H