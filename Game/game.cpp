#include "game.h"
#include "engine/gengine.h"
#include "engine/manager/igproject_manager.h"
#include "engine/imanager_table.h"

GGameInpl::GGameInpl(const std::filesystem::path& gprojectPath)
{
	m_gameProjectPath = gprojectPath;
}

bool GGameInpl::before_update()
{
	return false;
}

void GGameInpl::update()
{
}

void GGameInpl::after_update()
{
}

bool GGameInpl::before_render()
{
	return false;
}

void GGameInpl::render()
{
}

void GGameInpl::after_render()
{
}

bool GGameInpl::init(GEngine* engine)
{
	if (!std::filesystem::exists(m_gameProjectPath))
		return false;
	
	auto projManager = GET_MANAGER(IGProjectManager, ENGINE_MANAGER_PROJECT);
	auto proj = projManager->open_project(m_gameProjectPath);
	if (proj == nullptr)
		return false;


	return true;
}

void GGameInpl::destroy()
{
}
