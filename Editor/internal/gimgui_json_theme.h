#ifndef GIMGUI_JSON_THEME_H
#define GIMGUI_JSON_THEME_H

#include "editor/igimgui_theme.h"
#include "engine/resource/json/igjson.h"
#include <imgui/imgui.h>
#include "public/core/templates/unordered_dense.h"

#include <string>
#include <vector>
#include <utility>
#include <tuple>

class GImGuiJsonTheme : public IGImGuiTheme
{
public:
	~GImGuiJsonTheme();
	GImGuiJsonTheme(const std::string& jsonPath);

	virtual bool is_valid() override;
	virtual bool init() override;
	virtual void destroy() override;
	// Inherited via IGImguiTheme
	virtual const char* get_theme_name() override;
	virtual void setImGuiTheme(ImGuiStyle& style) override;

	void json_value_key(std::string_view key,IGJsonValue* val);
private:
	std::vector<std::pair<ImGuiCol_,std::tuple<float,float,float,float>>> m_imguiValueVector;
	ankerl::unordered_dense::segmented_set<ImGuiCol_> m_colSet;
	bool m_is_valid = false;
	IGJson* m_jsonFile;
	std::string m_fileName;
};

#endif // GIMGUI_JSON_THEME_H