#include "engine/scene/component/igscript.h"

IGScript::IGScript()
{
}

entt::entity IGScript::get_bounded_entity() const noexcept
{
	return m_entityID;
}
