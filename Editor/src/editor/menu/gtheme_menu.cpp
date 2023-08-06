#include "internal/menu/gtheme_menu.h"
#include "imgui/imgui.h"
#include "internal/theme/gimgui_default_theme.h"
#include "internal/theme/gimgui_bright_dark_theme.h"
#include "internal/theme/gimgui_theme_1.h"
#include "internal/theme/gimgui_theme_2.h"
#include "internal/gimgui_json_theme.h"

#include "editor/files.h"

#include <cassert>
#include <filesystem>

GThemeMenu::~GThemeMenu()
{
	int a = 5;
}

bool GThemeMenu::add_theme(IGImGuiTheme* theme)
{
	std::string themeName = theme->get_theme_name();
	if (auto thm = m_themes.find(themeName); thm != m_themes.end())
		return false;

	bool inited = theme->init();
	if (!inited)
		return false;


	if (!theme->is_valid())
		return false;

	std::string name = theme->get_theme_name();
	m_themes.emplace(name, theme);
	m_themesVector.push_back(std::make_pair(false,theme));
	return true;	
}

bool GThemeMenu::init()
{
	//X TODO : GDNEWDA
	init_built_in_themes();

	load_json_themes();

	return true;
}

bool GThemeMenu::need_render()
{
	return true;
}

void GThemeMenu::render()
{
	for (int i = 0; i < m_themesVector.size(); i++)
	{
		std::string radio = "##RadioButton";
		radio += m_themesVector[i].second->get_theme_name();
		
		if (ImGui::Selectable(m_themesVector[i].second->get_theme_name(), &m_themesVector[i].first) && m_themesVector[i].first)
		{
			select_theme(i);
		}
	}
	
}

void GThemeMenu::on_resize()
{
}

void GThemeMenu::on_data_update()
{
}

const char* GThemeMenu::get_menu_name()
{
	return "Themes";
}

void GThemeMenu::select_theme(int index)
{
	if (m_selectedTheme)
	{
		m_selectedTheme->first = false;

		//X TODO : UNBOUND FUNC FOR THEME
	}
	m_selectedTheme = &m_themesVector[index];
	m_selectedTheme->first = true;
	m_selectedTheme->second->setImGuiTheme(ImGui::GetStyle());
}

void GThemeMenu::init_built_in_themes()
{
	IGImGuiTheme* theme = new GImGuiDefaultTheme();
	add_theme(theme);
	select_theme(m_themesVector.size() - 1);

	theme = new GImGuiBrightDarkTheme();
	add_theme(theme);

	theme = new GImGuiTheme1();
	add_theme(theme);

	theme = new GImGuiTheme2();
	add_theme(theme);
}

void GThemeMenu::load_json_themes()
{
	auto themePath = THEME_FOLDER_FULL_PATH;
	std::filesystem::path themeFolder(themePath);


	using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
	try
	{
		for (const auto& dirEntry : recursive_directory_iterator(themeFolder))
		{
			if (!dirEntry.path().has_extension())
				continue;

			auto extensionAsStr = dirEntry.path().extension().string();
			if (strcmp(extensionAsStr.c_str(), ".json") != 0)
				continue;

			GImGuiJsonTheme* jsonTheme = new GImGuiJsonTheme(dirEntry.path().string());
			bool inited = jsonTheme->init();

			if (!add_theme(jsonTheme))
			{
				delete jsonTheme;
			}
		}
	}
	catch (std::exception& ex)
	{

	}
	
}

void GThemeMenu::destroy()
{

	for (int i = 0; i < m_themesVector.size(); i++)
	{
		m_themesVector[i].second->destroy();
		delete m_themesVector[i].second;
	}
}
