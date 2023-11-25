#ifndef IGSCRIPT_H
#define IGSCRIPT_H

#include "engine/GEngine_EXPORT.h"
#include "entt/entt.hpp"
#include "engine/scene/gentity.h"

class ScriptComponent;

class ENGINE_API IGScript
{
	friend class ScriptComponent;
public:
	IGScript();
	virtual ~IGScript() = default;

	virtual void update(float dt) = 0;

protected:
	virtual GEntity* get_bounded_entity() const noexcept;
private:
	GEntity* m_entityID;
};

#endif // IGSCRIPT_H