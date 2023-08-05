#ifndef GIMGUI_JSON_THEME_H
#define GIMGUI_JSON_THEME_H

#include "editor/igimgui_theme.h"
#include "engine/resource/json/igjson.h"
#include <imgui/imgui.h>

#include <string>
#include <vector>
#include <utility>

class GImGuiJsonTheme : public IGImguiTheme
{
public:
	GImGuiJsonTheme(const std::string& jsonPath);

	virtual bool is_valid() override;
	virtual bool init() override;
	virtual void destroy() override;
	// Inherited via IGImguiTheme
	virtual const char* get_theme_name() override;
	virtual void setImGuiTheme(ImGuiStyle& style) override;

	void json_value_key(std::string_view key,IGJsonValue* val);
private:
	std::vector<std::pair<ImGuiCol_,double>> m_imguiValueVector;

	bool m_is_valid = false;
	IGJson* m_jsonFile;
};

#endif // GIMGUI_JSON_THEME_H