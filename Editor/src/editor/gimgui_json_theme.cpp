#include "internal/gimgui_json_theme.h"
#include "engine/resource/json/gjson_utils.h"
#include "public/core/string/os_str.h"
#include <string>
#include "public/core/templates/unordered_dense.h"

#define IMGUI_PAIR(element) {#element, ImGuiCol_##element} 

static ankerl::unordered_dense::segmented_map<std::string, ImGuiCol_> m_strToImGuiCol = {
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

GImGuiJsonTheme::GImGuiJsonTheme(const std::string& jsonPath)
{
	m_jsonFile = GJsonUtils::create_default_json(jsonPath);
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

    m_jsonFile->iterate_in(std::bind(&GImGuiJsonTheme::json_value_key, this, std::placeholders::_1, std::placeholders::_2));

    return true;
}

void GImGuiJsonTheme::destroy()
{
	m_jsonFile->destroy();
}

const char* GImGuiJsonTheme::get_theme_name()
{
	// TODO: insert return statement here
	return m_jsonFile->get_file_path_c_str();
	
}

void GImGuiJsonTheme::setImGuiTheme(ImGuiStyle& style)
{
	
}

void GImGuiJsonTheme::json_value_key(std::string_view key, IGJsonValue* val)
{  
    
}
