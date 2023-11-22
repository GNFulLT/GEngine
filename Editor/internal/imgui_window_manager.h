#ifndef IMGUI_WINDOW_MANAGER_H
#define IMGUI_WINDOW_MANAGER_H

#include "public/core/templates/unordered_dense.h"
#include "internal/gimgui_window.h"
#include "internal/gimgui_menu.h"
#include "public/core/templates/shared_ptr.h"
#include "public/platform/imouse_manager.h"
#include <filesystem>
#include <string_view>
#include <mutex>
#include <string>
#include <vector>
#include <atomic>
#include <stack>
#include "internal/gimgui_modal.h"

class IGTextureResource;
class Window;

#include <queue>
#include <utility>

enum WINDOW_OP
{
	WINDOW_OP_ADD,
	WINDOW_OP_EXTRACT
};



class ImGuiWindowManager
{
public:
	inline constexpr static std::string_view VIEWPORT_NAME = "Viewport";

	~ImGuiWindowManager();
	// Multi-thread safe
	bool create_imgui_window(IGImGuiWindowImpl* impl, GIMGUIWINDOWDIR dir = GIMGUIWINDOWDIR_NONE);

	bool create_imgui_menu(IGImGuiMenuImpl* impl);

	bool init();

	void destroy();

	void render_windows();

	void render_main_menu();

	void try_to_open_file_in_new_editor(std::filesystem::path path);

	void try_to_show_string_in_new_editor(const std::string& content, std::string_view name, std::string_view id, bool isReadOnly = true);

	GImGuiWindow* get_window_if_exist(std::string_view name);

	bool add_modal_to_queue(IGImGuiModalImpl* impl);

	void set_modal_setter(std::function<bool()> modalSetter);

	std::function<bool()> get_modal_setter();
private:
	void render_main_dockspace();

	void before_render();

	void build_nodes();

	void draw_main_menu_bar();

	void dock_the_window_if_needs(GImGuiWindow* win);

	void extract_window(GImGuiWindow* win);

	void safe_add_window(GImGuiWindow* win);

	void unsafe_add_window(GImGuiWindow* window);

	void safe_extract_window(GImGuiWindow* win);

	void handle_modals();
private:
	std::queue<GImGuiModal*> m_modalStack;
	std::function<bool()> m_modalSetter;
	//X TODO : USE SHARED OR UNIQUE PTR
	ankerl::unordered_dense::segmented_map<std::string, GImGuiWindow*> m_windowMap;
	std::vector<GImGuiWindow*> m_windowVector;
	ankerl::unordered_dense::segmented_map<std::string,GImGuiMenu*> m_menuMap;
	std::vector<GImGuiMenu*> m_menuVector;

	uint32_t m_dock_id;

	// Nodes
	uint32_t dock_id_left_top;
	uint32_t dock_id_right_top;
	uint32_t dock_id_bottom;
	uint32_t dock_id_middle;
	uint32_t dock_id_left_bottom;
	uint32_t dock_id_right_bottom;

	bool m_isDockDirty = true;

	uint64_t m_windowId = 0;

	std::mutex m_windowCreationMutex;


	float m_buttonWidth = 40.f;
	float m_leftGap = 20.f;
	float m_leftAlignmentForId = 0;
	float m_topAlignmentForId = 0;


	GSharedPtr<IGTextureResource> m_smallBiggerTexture;
	GSharedPtr<IGTextureResource> m_minusTexture;
	GSharedPtr<IGTextureResource> m_xTexture;
	GSharedPtr<IGTextureResource> m_editorIcon;

	Window* m_window;


	std::queue<std::pair<GImGuiWindow*,WINDOW_OP>> m_waitingWindows;

	bool m_isDragging = false;
	bool m_isClickedOutside = false;
	std::atomic_bool m_isInRender = false;
	int cp_x = 0;
	int cp_y = 0 ;
	int offset_cpx = 0;
	int offset_cpy = 0;
};

#endif // IMGUI_WINDOW_MANAGER_H