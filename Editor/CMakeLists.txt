cmake_minimum_required(VERSION 3.5)

project(GEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



add_library(GEditor SHARED  "src/editor/editor_application_impl.cpp" "src/editor/imgui_wrapper.cpp" "internal/imgui_layer.h" "src/editor/imgui_layer.cpp"
"internal/imgui_window_manager.h" "include/editor/igimgui_window_impl.h"  "src/editor/imgui_window_manager.cpp" "include/editor/igimgui_theme.h" 
"src/editor/gimgui_window.cpp" "include/editor/igimgui_menu_impl.h" "internal/gimgui_menu.h" "src/editor/gimgui_menu.cpp" "src/editor/menu/gtheme_menu.cpp"
"internal/gimgui_json_theme.h" "src/editor/gimgui_json_theme.cpp"  "internal/theme/gimgui_default_theme.h"  "src/editor/theme/gimgui_default_theme.cpp" "internal/theme/gimgui_bright_dark_theme.h"
"src/editor/theme/gimgui_bright_dark_theme.cpp" "internal/theme/gimgui_theme_1.h" "src/editor/theme/gimgui_theme_1.cpp" "include/editor/files.h" "internal/theme/gimgui_theme_2.h" 
"src/editor/theme/gimgui_theme_2.cpp" "internal/rendering/vulkan/gimgui_descriptor_creator.h" "src/editor/rendering/vulkan/gimgui_descriptor_creator.cpp" "internal/rendering/vulkan/gimgui_descriptor.h"
"src/editor/rendering/vulkan/gimgui_descriptor.cpp" "internal/window/gimgui_viewport_window.h" "src/editor/window/gimgui_viewport_window.cpp" "internal/rendering/vulkan/gscene_renderer.h" 
"src/editor/rendering/vulkan/gscene_renderer.cpp" "internal/window/gimgui_texteditor_window.h" "src/editor/window/gimgui_texteditor_window.cpp" "internal/window/gimgui_content_browser_window.h" 
"src/editor/window/gimgui_content_browser_window.cpp" "internal/utils.h" "src/editor/utils.cpp" "include/editor/file_type.h" "internal/window/content_helper/gimgui_content_helper.h"
"src/editor/window/content_helper/gimgui_content_helper.cpp" "internal/window/content_helper/gimgui_content_descriptor.h" "include/editor/igimgui_content_descriptor_impl.h" 
"src/editor/window/content_helper/gimgui_content_descriptor.cpp" "internal/window/content_helper/descriptors/gimgui_texteditor_descriptor.h" 
"src/editor/window/content_helper/descriptors/gimgui_texteditor_descriptor.cpp" "internal/window/content_helper/descriptors/gimgui_shader_descriptor.h"
"src/editor/window/content_helper/descriptors/gimgui_shader_descriptor.cpp" "internal/window/gimgui_log_window.h" "src/editor/window/gimgui_log_window.cpp" "internal/manager/geditor_shader_manager.h" 
"src/editor/manager/geditor_shader_manager.cpp" "src/editor/shader/spirv_shader_utils.cpp" "internal/shader/editor_debuggable_spirv_shader.h" "src/editor/shader/editor_debuggable_spirv_shader.cpp"
"src/editor/shader/gspirv_byte_shader.cpp" "internal/window/content_helper/descriptors/gimgui_spirv_descriptor.h" "src/editor/window/content_helper/descriptors/gimgui_spirv_descriptor.cpp"
"internal/shader/gspirv_shader_debugger.h" "src/editor/shader/gspirv_shader_debugger.cpp" "internal/rendering/vertex_types.h" "src/editor/rendering/renderable.cpp" 
"internal/rendering/vulkan/gdefault_pipeline_injector.h" "src/editor/rendering/vulkan/gdefault_pipeline_injector.cpp" "internal/rendering/vulkan/gvulkan_pipeline_layout_wrapper.h" 
"src/editor/rendering/vulkan/gvulkan_pipeline_layout_wrapper.cpp" "src/editor/rendering/gfps_camera_positioner.cpp" "internal/rendering/vulkan/gcube_renderer.h" "src/editor/rendering/vulkan/gcube_renderer.cpp" "internal/rendering/vulkan/gcube_pipeline_layout_creator.h" "src/editor/rendering/vulkan/gcube_pipeline_layout_creator.cpp" "internal/rendering/vulkan/ggrid_renderer.h" "src/editor/rendering/vulkan/ggrid_renderer.cpp" "internal/rendering/vulkan/ggrid_pipeline_layout_creator.h" "src/editor/rendering/vulkan/ggrid_pipeline_layout_creator.cpp" "internal/rendering/vulkan/gvulkan_pipeline_layout_ref.h" "src/editor/rendering/vulkan/gvulkan_pipeline_layout_ref.cpp" "internal/rendering/grid_spec.h" "internal/window/gimgui_grid_settings_window.h" "src/editor/window/gimgui_grid_settings_window.cpp")

target_include_directories(GEditor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../Core/include")
target_include_directories(GEditor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/include")
target_include_directories(GEditor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(GEditor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(GEditor PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/spdlog/include")

target_include_directories(GEditor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/imgui")
target_include_directories(GEditor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty")
target_include_directories(GEditor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/glfw/include")
target_include_directories(GEditor PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/glslang/glslang/")
target_include_directories(GEditor PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/glm/")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../Engine" ENGINE)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/glm" GLM)

target_link_libraries(GEditor PRIVATE GEngine_Core)
target_link_libraries(GEditor PRIVATE GEngine_Engine)
target_link_libraries(GEditor PRIVATE glfw)
target_link_libraries(GEditor PRIVATE spdlog)

include(GenerateExportHeader)

GENERATE_EXPORT_HEADER(GEditor
    BASE_NAME GEditor
    EXPORT_MACRO_NAME EDITOR_API
    EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/include/editor/GEngine_EXPORT.h"
    STATIC_DEFINE EDITOR_AS_STATIC
)