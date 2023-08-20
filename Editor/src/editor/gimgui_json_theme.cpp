#include "internal/gimgui_json_theme.h"
#include "engine/resource/json/gjson_utils.h"
#include "public/core/string/os_str.h"
#include <string>
#include <filesystem>
#define IMGUI_PAIR(element) {#element, ImGuiCol_##element} 

static ankerl::unordered_dense::segmented_map<std::string, ImGuiCol_> s_str_to_imgui_color = {
    IMGUI_PAIR(Text),
    IMGUI_PAIR(TextDisabled),
    IMGUI_PAIR(WindowBg),              // Background of normal windows
    IMGUI_PAIR(ChildBg),               // Background of child windows
    IMGUI_PAIR(PopupBg),               // Background of popups, menus, tooltips windows
    IMGUI_PAIR(Border),
    IMGUI_PAIR(BorderShadow),
    IMGUI_PAIR(FrameBg),               // Background of checkbox, radio button, plot, slider, text input
    IMGUI_PAIR(FrameBgHovered),
    IMGUI_PAIR(FrameBgActive),
    IMGUI_PAIR(TitleBg),
    IMGUI_PAIR(TitleBgActive),
    IMGUI_PAIR(TitleBgCollapsed),
    IMGUI_PAIR(MenuBarBg),
    IMGUI_PAIR(ScrollbarBg),
    IMGUI_PAIR(ScrollbarGrab),
    IMGUI_PAIR(ScrollbarGrabHovered),
    IMGUI_PAIR(ScrollbarGrabActive),
    IMGUI_PAIR(CheckMark),
    IMGUI_PAIR(SliderGrab),
    IMGUI_PAIR(SliderGrabActive),
    IMGUI_PAIR(Button),
    IMGUI_PAIR(ButtonHovered),
    IMGUI_PAIR(ButtonActive),
    IMGUI_PAIR(Header),                // Header* colors are used for CollapsingHeader, TreeNode, Selectable, MenuItem
    IMGUI_PAIR(HeaderHovered),
    IMGUI_PAIR(HeaderActive),
    IMGUI_PAIR(Separator),
    IMGUI_PAIR(SeparatorHovered),
    IMGUI_PAIR(SeparatorActive),
    IMGUI_PAIR(ResizeGrip),            // Resize grip in lower-right and lower-left corners of windows.
    IMGUI_PAIR(ResizeGripHovered),
    IMGUI_PAIR(ResizeGripActive),
    IMGUI_PAIR(Tab),                   // TabItem in a TabBar
    IMGUI_PAIR(TabHovered),
    IMGUI_PAIR(TabActive),
    IMGUI_PAIR(TabUnfocused),
    IMGUI_PAIR(TabUnfocusedActive),
    IMGUI_PAIR(DockingPreview),        // Preview overlay color when about to docking something
    IMGUI_PAIR(DockingEmptyBg),        // Background color for empty node (e.g. CentralNode with no window docked into it)
    IMGUI_PAIR(PlotLines),
    IMGUI_PAIR(PlotLinesHovered),
    IMGUI_PAIR(PlotHistogram),
    IMGUI_PAIR(PlotHistogramHovered),
    IMGUI_PAIR(TableHeaderBg),         // Table header background
    IMGUI_PAIR(TableBorderStrong),     // Table outer and header borders (prefer using Alpha=1.0 here)
    IMGUI_PAIR(TableBorderLight),      // Table inner borders (prefer using Alpha=1.0 here)
    IMGUI_PAIR(TableRowBg),            // Table row background (even rows)
    IMGUI_PAIR(TableRowBgAlt),         // Table row background (odd rows)
    IMGUI_PAIR(TextSelectedBg),
    IMGUI_PAIR(DragDropTarget),        // Rectangle highlighting a drop target
    IMGUI_PAIR(NavHighlight),          // Gamepad/keyboard: current highlighted item
    IMGUI_PAIR(NavWindowingHighlight), // Highlight window when using CTRL+TAB
    IMGUI_PAIR(NavWindowingDimBg),     // Darken/colorize entire screen behind the CTRL+TAB window list, when active
    IMGUI_PAIR(ModalWindowDimBg),      // Darken/colorize entire screen behind a modal window, when one is active
};

GImGuiJsonTheme::~GImGuiJsonTheme()
{
    int a = 5;
}

GImGuiJsonTheme::GImGuiJsonTheme(const std::string& jsonPath)
{
	m_jsonFile = GJsonUtils::create_default_json(jsonPath);
    std::filesystem::path path(jsonPath);
    auto temp = path.filename().string();
    if (path.has_filename())
        m_fileName = temp.substr(0,temp.size()-5);
}

bool GImGuiJsonTheme::is_valid()
{
	return m_jsonFile->is_valid();
}

bool GImGuiJsonTheme::init()
{
	RESOURCE_INIT_CODE res = m_jsonFile->init();
    
    if (res != RESOURCE_INIT_CODE_OK)
        return false;

    return m_jsonFile->iterate_in(std::bind(&GImGuiJsonTheme::json_value_key, this, std::placeholders::_1, std::placeholders::_2));;
}

void GImGuiJsonTheme::destroy()
{
	m_jsonFile->destroy();
    delete m_jsonFile;
    m_jsonFile = nullptr;
}

const char* GImGuiJsonTheme::get_theme_name()
{
	// TODO: insert return statement here
	return m_fileName.c_str();
	
}

void GImGuiJsonTheme::setImGuiTheme(ImGuiStyle& style)
{
    auto colors = style.Colors;
    for (int i = 0; i < m_imguiValueVector.size(); i++)
    {
        const auto& [r, g, b, a] = m_imguiValueVector[i].second;
        colors[m_imguiValueVector[i].first] = ImVec4(r,g,b,a);
    }
}

void GImGuiJsonTheme::json_value_key(std::string_view key, IGJsonValue* val)
{  
    std::string keyV= std::string(key.begin(),key.end());
    if (auto value = s_str_to_imgui_color.find(keyV); value != s_str_to_imgui_color.end())
    {
        if (val->get_value_type() != JSON_VALUE_STRING)
            return;

        //X ALREADY ADDED CONTINUE
        if (auto sval = m_colSet.find(value->second); sval != m_colSet.end())
            return;

        //X TODO CONVERT EXCEPTION TO EXPECTED
        try
        {
            std::string_view str = val->try_to_get_as_string();
            int b = 5;
            auto res = GJsonUtils::string_to_rgba(str);
            //X TODO : Log
            if (!res.has_value())
                return;

            auto rgba = res.value();
            m_colSet.emplace(value->second);
            m_imguiValueVector.push_back(std::make_pair(value->second,rgba));
        }
        catch (std::exception& ex)
        {
            int a = 5;
        }
    }
    else
    {
        return;
    }
}
