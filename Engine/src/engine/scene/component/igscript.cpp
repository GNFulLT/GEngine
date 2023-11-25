#include "engine/scene/component/igscript.h"
#include "engine/gengine.h"
#include "engine/imanager_table.h"
#include "engine/manager/igscene_manager.h"

IGScript::IGScript()
{
}

GEntity* IGScript::get_bounded_entity() const noexcept
{
	return m_entityID;
}
