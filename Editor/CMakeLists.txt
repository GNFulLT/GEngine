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
"src/editor/rendering/vulkan/gvulkan_pipeline_layout_wrapper.cpp" "src/editor/rendering/gfps_camera_positioner.cpp" "internal/rendering/vulkan/gcube_renderer.h" "src/editor/rendering/gcube_renderer.cpp" 
"internal/rendering/vulkan/gcube_pipeline_layout_creator.h" "src/editor/rendering/vulkan/gcube_pipeline_layout_creator.cpp" "internal/rendering/vulkan/ggrid_renderer.h"
"src/editor/rendering/vulkan/ggrid_renderer.cpp" "internal/rendering/vulkan/ggrid_pipeline_layout_creator.h" "src/editor/rendering/vulkan/ggrid_pipeline_layout_creator.cpp" 
"internal/rendering/vulkan/gvulkan_pipeline_layout_ref.h" "src/editor/rendering/vulkan/gvulkan_pipeline_layout_ref.cpp" "internal/rendering/grid_spec.h" "internal/window/gimgui_grid_settings_window.h"
"src/editor/window/gimgui_grid_settings_window.cpp" "include/editor/igimgui_window_source_impl.h" "internal/rendering/mesh/gmesh_assimp_encoder.h" "src/editor/rendering/mesh/gmesh_assimp_encoder.cpp" 
"internal/window/content_helper/descriptors/gimgui_model_asset_descriptor.h" "src/editor/window/content_helper/descriptors/gimgui_model_asset_descriptor.cpp"
"internal/window/content_helper/descriptors/gimgui_gmesh_descriptor.h" "src/editor/window/content_helper/descriptors/gimgui_gmesh_descriptor.cpp" "internal/rendering/scene/scene_converter.h"
"src/editor/rendering/scene/scene_converter.cpp" "internal/rendering/material/material_assimp_converter.h" "src/editor/rendering/material/material_assimp_converter.cpp" 
 "internal/window/gimgui_properties_window.h" "src/editor/window/gimgui_properties_window.cpp" "internal/window/gimgui_material_window.h" "src/editor/window/gimgui_material_window.cpp" "internal/imguizmo.h" "src/editor/imguizmo.cpp" 
 "internal/window/gimgui_scene_window.h" "src/editor/window/gimgui_scene_window.cpp" "internal/window/gimgui_position_port.h"
 "src/editor/window/gimgui_position_port.cpp" "internal/window/gimgui_normal_port.h" "internal/window/gimgui_albedo_port.h" 
 "src/editor/window/gimgui_albedo_port.cpp" "src/editor/window/gimgui_normal_port.cpp" "internal/window/gimgui_composition_window.h" 
 "src/editor/window/gimgui_composition_window.cpp" "internal/manager/geditor_texture_debug_manager.h" 
 "src/editor/manager/geditor_texture_debug_manager.cpp" "internal/window/gimgui_pbr_window.h" "src/editor/window/gimgui_pbr_window.cpp"
 "internal/window/gimgui_sunshadow_window.h" "src/editor/window/gimgui_sunshadow_window.cpp" 
 "internal/window/content_helper/descriptors/gimgui_project_descriptor.h" "src/editor/window/content_helper/descriptors/gimgui_project_descriptor.cpp"
 "internal/manager/gproject_manager.h" "src/editor/manager/gproject_manager.cpp" 
 "internal/menu/gproject_menu.h" "src/editor/menu/gproject_menu.cpp" "internal/window/gimgui_file_dialog_window.h" 
 "src/editor/window/gimgui_file_dialog_window.cpp" "internal/gimgui_modal.h" "include/editor/igimgui_modal_impl.h" "src/editor/gimgui_modal.cpp" "internal/modal/gimgui_function_modal.h" "src/editor/modal/gimgui_function_modal.cpp")

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
target_include_directories(GEditor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/assimp/include")
target_include_directories(GEditor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/meshoptimizer/src")


add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../Engine" ENGINE)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/glm" GLM)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/meshoptimizer" MESHOPTIMIZER)

set(BUILD_SHARED_LIBS ON)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/assimp" ASSIMP)


target_link_libraries(GEditor PRIVATE GEngine_Core)
target_link_libraries(GEditor PRIVATE GEngine_Engine)
target_link_libraries(GEditor PRIVATE glfw)
target_link_libraries(GEditor PRIVATE spdlog)
target_link_libraries(GEditor PRIVATE assimp)
target_link_libraries(GEditor PRIVATE meshoptimizer)

include(GenerateExportHeader)

GENERATE_EXPORT_HEADER(GEditor
    BASE_NAME GEditor
    EXPORT_MACRO_NAME EDITOR_API
    EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/include/editor/GEngine_EXPORT.h"
    STATIC_DEFINE EDITOR_AS_STATIC
)