cmake_minimum_required(VERSION 3.5)

project(GEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



add_library(GEditor SHARED  "src/editor/editor_application_impl.cpp" "src/editor/imgui_wrapper.cpp" "internal/imgui_layer.h" "src/editor/imgui_layer.cpp"
"internal/imgui_window_manager.h" "include/editor/igimgui_window_impl.h"  "src/editor/imgui_window_manager.cpp" "include/editor/igimgui_theme.h" 
"src/editor/gimgui_window.cpp" "include/editor/igimgui_menu_impl.h" "internal/gimgui_menu.h" "src/editor/gimgui_menu.cpp" "src/editor/menu/gtheme_menu.cpp"
"internal/gimgui_json_theme.h" "src/editor/gimgui_json_theme.cpp"  "internal/theme/gimgui_default_theme.h"  "src/editor/theme/gimgui_default_theme.cpp" "internal/theme/gimgui_bright_dark_theme.h" "src/editor/theme/gimgui_bright_dark_theme.cpp" "internal/theme/gimgui_theme_1.h" "src/editor/theme/gimgui_theme_1.cpp")

target_include_directories(GEditor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../Core/include")
target_include_directories(GEditor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/include")
target_include_directories(GEditor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(GEditor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_include_directories(GEditor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/imgui")
target_include_directories(GEditor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty")
target_include_directories(GEditor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/glfw/include")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../Engine" ENGINE)

target_link_libraries(GEditor PRIVATE GEngine_Core)
target_link_libraries(GEditor PRIVATE GEngine_Engine)
target_link_libraries(GEditor PRIVATE glfw)

include(GenerateExportHeader)

GENERATE_EXPORT_HEADER(GEditor
    BASE_NAME GEditor
    EXPORT_MACRO_NAME EDITOR_API
    EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/include/editor/GEngine_EXPORT.h"
    STATIC_DEFINE EDITOR_AS_STATIC
)