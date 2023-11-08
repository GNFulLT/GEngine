cmake_minimum_required(VERSION 3.5)

project(GEngine_Engine LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

execute_process(COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../Programs/reflect_script/reflect_generator.py ./ WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
include(${CMAKE_CURRENT_SOURCE_DIR}/greflect.cmake)


add_library(GEngine_Engine SHARED ${GREFLECT_GENERATED_CPP} "src/engine/gengine.cpp" "include/engine/gapplication_impl.h" "include/engine/globals.h" "src/engine/globals.cpp"
"src/engine/rendering/vulkan/vulkan_app.cpp" "internal/engine/rendering/vulkan/vulkan_app.h" "internal/engine/rendering/vulkan/vulkan_utils.h" 
"include/engine/rendering/vulkan/ivulkan_device.h" "internal/engine/rendering/vulkan/vulkan_device.h" "src/engine/rendering/vulkan/vulkan_device.cpp" "internal/engine/rendering/vulkan/vulkan_pdevice.h"
"src/engine/rendering/vulkan/vulkan_pdevice.cpp" "internal/engine/rendering/vulkan/vulkan_ldevice.h" "src/engine/rendering/vulkan/vulkan_ldevice.cpp" "internal/engine/rendering/vulkan/vulkan_queue.h" 
"src/engine/rendering/vulkan/vulkan_queue.cpp" "include/engine/rendering/vulkan/vulkan_command_buffer.h" "src/engine/rendering/vulkan/vulkan_command_buffer.cpp" "include/engine/imanager_table.h" 
"internal/engine/manager_table.h" "src/engine/manager_table.cpp" "internal/engine/rendering/vulkan/vulkan_main_viewport.h" "src/engine/rendering/vulkan/vulkan_viewport.cpp"
 "internal/engine/rendering/vulkan/vulkan_swapchain.h" "src/engine/rendering/vulkan/vulkan_swapchain.cpp" "internal/engine/rendering/vulkan/vulkan_renderpass.h" "src/engine/rendering/vulkan/vulkan_renderpass.cpp" 
 "include/engine/rendering/vulkan/ivulkan_viewport.h" "include/engine/rendering/vulkan/vulkan_memory.h" "src/engine/rendering/vulkan/vulkan_memory.cpp" "include/engine/rendering/vulkan/ivulkan_queue.h"
 "include/engine/resource/json/igjson.h" "internal/engine/resource/json/gjson_simdjson.h" "src/engine/resource/json/gjson_simdjson.cpp" "include/engine/resource/resource_init_code.h"
 "include/engine/resource/json/gjson_utils.h" "src/engine/resource/json/gjson_utils.cpp" "include/engine/resource/json/igjson_value.h" "internal/engine/resource/json/gjson_value_simdjson.h" 
 "src/engine/resource/json/gjson_value_simdjson.cpp" "include/engine/manager/iglogger_manager.h" "internal/engine/manager/glogger_manager.h" "src/engine/manager/glogger_manager.cpp" 
 "include/engine/io/iowning_glogger.h" "internal/engine/io/gowning_glogger.h" "src/engine/io/gowning_glogger.cpp" "internal/engine/io/logger_helper.h" "include/engine/resource/iresource.h"
 "include/engine/resource/iresource_listener.h"   "internal/engine/manager/gresource_manager.h" "src/engine/manager/gresource_manager.cpp" "include/engine/manager/igresource_manager.h" 
 "include/engine/io/gimage_descriptor.h" "include/engine/resource/igresource_loader.h" "internal/engine/io/stb_image_loader.h" "src/engine/io/stb_image_loader.cpp" "include/engine/resource/igtexture_resource.h"
 "internal/engine/resource/gtexture_resource.h" "src/engine/resource/gtexture_resource.cpp" "include/engine/io/iimage_loader.h" "src/engine/resource/gresource.cpp" "include/engine/rendering/vulkan/ivulkan_buffer.h"
 "internal/engine/rendering/vulkan/gvulkan_buffer.h" "src/engine/rendering/vulkan/gvulkan_buffer.cpp" "include/engine/rendering/vulkan/ivulkan_image.h" "internal/engine/rendering/vulkan/gvulkan_image.h" 
 "src/engine/rendering/vulkan/gvulkan_image.cpp" "include/engine/rendering/vulkan/transfer/itransfer_op.h" "internal/engine/rendering/vulkan/transfer/transfer_op_transfer_queue.h" 
 "src/engine/rendering/vulkan/transfer/transfer_op_transfer_queue.cpp" "include/engine/rendering/vulkan/transfer/itransfer_handle.h" "internal/engine/rendering/vulkan/vma_importer.h"
 "src/engine/rendering/vulkan/vma_importer.cpp" "include/engine/rendering/vulkan/ivulkan_sampler.h" "include/engine/rendering/vulkan/ivulkan_sampler_creator.h"
 "internal/engine/rendering/vulkan/gvulkan_default_sampler_creator.h" "src/engine/rendering/vulkan/gvulkan_default_sampler_creator.cpp" "src/engine/rendering/vulkan/gvulkan_sampler.cpp"
 "internal/engine/rendering/vulkan/gvulkan_sampler.h" "internal/engine/rendering/vulkan/gvulkan_offscreen_viewport.h" "src/engine/rendering/vulkan/gvulkan_offscreen_viewport.cpp" 
 "include/engine/rendering/vulkan/ivulkan_descriptor_creator.h" "include/engine/rendering/vulkan/ivulkan_descriptor.h" "include/engine/manager/iinject_manager_helper.h" 
 "internal/engine/manager/ginjection_manager.h" "src/engine/manager/ginjection_manager.cpp" "include/engine/shader/ispirv_shader.h" "include/engine/resource/igshader_resource.h" 
 "include/engine/manager/igshader_manager.h" "internal/engine/manager/gshader_manager.h" "src/engine/manager/gshader_manager.cpp" "internal/engine/shader/spirv_shader_utils.h" "src/engine/utils.cpp"
 "internal/engine/utils.h" "src/engine/shader/spirv_shader_utils.cpp" "internal/engine/resource/gshader_resource.h" "src/engine/resource/gshader_resource.cpp" "internal/engine/shader/gspirv_shader.h" 
 "src/engine/shader/gspirv_shader.cpp" "include/engine/manager/ijob_manager.h" "internal/engine/manager/gjob_manager.h" "src/engine/manager/gjob_manager.cpp" "include/engine/time_unit.h" 
 "internal/engine/shader/gspirv_byte_shader.h" "src/engine/shader/gspirv_byte_shader.cpp" "internal/engine/rendering/vulkan/gvulkan_graphic_pipeline.h" 
 "src/engine/rendering/vulkan/gvulkan_graphic_pipeline.cpp" "internal/engine/rendering/vulkan/gvulkan_shader_stage.h" "src/engine/rendering/vulkan/gvulkan_shader_stage.cpp" 
 "include/engine/rendering/vulkan/ivulkan_shader_stage.h" "include/engine/rendering/vulkan/ivulkan_graphic_pipeline.h" "include/engine/rendering/vulkan/ivulkan_graphic_pipeline_state.h" 
 "include/engine/rendering/vulkan/ivulkan_renderpass.h" "include/engine/rendering/vulkan/ivulkan_pipeline_layout.h" "internal/engine/rendering/vulkan/gvulkan_pipeline_layout.h" 
 "src/engine/rendering/vulkan/gvulkan_pipeline_layout.cpp" "include/engine/rendering/vulkan/ivulkan_descriptor_pool.h" "internal/engine/rendering/vulkan/gvulkan_descriptor_pool.h" 
 "src/engine/rendering/vulkan/gvulkan_descriptor_pool.cpp" "include/engine/rendering/vulkan/ivulkan_swapchain.h"   "internal/engine/shader/spirv_parser.h" "src/engine/shader/spirv_parser.cpp"
 "internal/engine/rendering/vulkan/gvulkan_vectorized_descriptor_pool.h" "src/engine/rendering/vulkan/gvulkan_vectorized_descriptor_pool.cpp"
 "internal/engine/rendering/vulkan/gvulkan_vectorized_pipeline_layout.h" "src/engine/rendering/vulkan/gvulkan_vectorized_pipeline_layout.cpp" "include/engine/rendering/vulkan/ivulkan_shader_info.h" 
 "internal/engine/rendering/vulkan/gvulkan_shader_info.h" "src/engine/rendering/vulkan/gvulkan_shader_info.cpp" "internal/engine/rendering/vulkan/gvulkan_uniform_buffer.h" 
 "src/engine/rendering/vulkan/gvulkan_uniform_buffer.cpp" "include/engine/rendering/vulkan/igvulkan_uniform_buffer.h" "internal/engine/rendering/vulkan/gvulkan_vertex_state.h"
 "src/engine/rendering/vulkan/gvulkan_vertex_state.cpp" "internal/engine/rendering/vulkan/gvulkan_input_assembly_state.h" "src/engine/rendering/vulkan/gvulkan_input_assembly_state.cpp"
 "internal/engine/rendering/vulkan/gvulkan_rasterization_state.h" "src/engine/rendering/vulkan/gvulkan_rasterization_state.cpp" "internal/engine/rendering/vulkan/gvulkan_multisample_state.h"
 "src/engine/rendering/vulkan/gvulkan_multisample_state.cpp" "internal/engine/rendering/vulkan/gvulkan_color_blend_state.h" "src/engine/rendering/vulkan/gvulkan_color_blend_state.cpp" 
 "internal/engine/rendering/vulkan/gvulkan_viewport_state.h" "src/engine/rendering/vulkan/gvulkan_viewport_state.cpp" "internal/engine/manager/gcamera_manager.h"
 "include/engine/rendering/icamera_positioner.h" "src/engine/manager/gcamera_manager.cpp"    "internal/engine/rendering/vulkan/gvulkan_vertex_buffer.h" 
 "include/engine/rendering/vulkan/igvulkan_vertex_buffer.h" "src/engine/rendering/vulkan/gvulkan_vertex_buffer.cpp" "include/engine/rendering/vulkan/igvulkan_graphic_pipeline_layout_creator.h"
 "internal/engine/rendering/vulkan/gvulkan_graphic_pipeline_custom_layout.h" "src/engine/rendering/vulkan/gvulkan_graphic_pipeline_custom_layout.cpp" 
 "internal/engine/rendering/vulkan/gvulkan_offscreen_depth_viewport.h" "src/engine/rendering/vulkan/gvulkan_offscreen_depth_viewport.cpp" 
 "internal/engine/rendering/vulkan/gvulkan_depth_stencil_state.h" "src/engine/rendering/vulkan/gvulkan_depth_stencil_state.cpp" "internal/engine/rendering/gvulkan_frame_data.h"
 "src/engine/rendering/vulkan/gvulkan_frame_data.cpp" "include/engine/rendering/igvulkan_frame_data.h" "internal/engine/rendering/vulkan/gvulkan_chained_viewport.h" 
 "src/engine/rendering/vulkan/gvulkan_chained_viewport.cpp" "include/engine/rendering/vulkan/igvulkan_chained_viewport.h" "include/engine/manager/igcamera_manager.h"
 "internal/engine/rendering/vulkan/gvulkan_camera_pipeline_layout_creator.h" "src/engine/rendering/vulkan/gvulkan_camera_pipeline_layout_creator.cpp"
 "src/engine/rendering/vulkan/gvulkan_basic_pipeline_layout.cpp" "internal/engine/rendering/gvulkan_fps_camera_positioner.h" 
 "src/engine/rendering/gvulkan_fps_camera_positioner.cpp" "src/engine/manager/gtimer_manager.cpp" "internal/engine/io/cube_image_loader.h" "src/engine/io/cube_image_loader.cpp" 
 "internal/engine/io/bitmap.h" "internal/engine/io/cubemap_utils.h" "src/engine/io/cubemap_utils.cpp" "internal/engine/manager/gpipeline_object_manager.h" "src/engine/manager/gpipeline_object_manager.cpp"
 "internal/engine/rendering/vulkan/named/gvulkan_named_renderpass.h" "src/engine/rendering/vulkan/gvulkan_named_renderpass.cpp" "include/engine/rendering/vulkan/named/igvulkan_named_renderpass.h" 
 "include/engine/manager/igpipeline_object_manager.h" "internal/engine/rendering/vulkan/gvulkan_render_target.h" "src/engine/rendering/vulkan/gvulkan_render_target.cpp"
 "include/engine/rendering/vulkan/named/igvulkan_named_sampler.h" "internal/engine/rendering/vulkan/named/gvulkan_named_sampler.h" "src/engine/rendering/vulkan/named/gvulkan_named_sampler.cpp" 
 "internal/engine/rendering/vulkan/named/pipelayouts/gvulkan_named_pipeline_layout_camera.h" "src/engine/rendering/vulkan/named/pipelayouts/gvulkan_named_pipeline_layout.cpp"
 "include/engine/rendering/vulkan/named/igvulkan_named_pipeline_layout.h" "internal/engine/rendering/vulkan/named/gvulkan_named_set_layout.h" "src/engine/rendering/vulkan/named/gvulkan_named_set_layout.cpp"
 "include/engine/rendering/vulkan/named/igvulkan_named_set_layout.h" "include/engine/rendering/mesh/mesh_constants.h" "src/engine/rendering/mesh/gmesh.cpp" "include/engine/rendering/mesh/gmesh.h" 
 "include/engine/rendering/mesh/gmesh_file.h"  "internal/engine/rendering/mesh/gmesh_renderable.h" "src/engine/rendering/mesh/gmesh_renderable.cpp"
 "internal/engine/rendering/renderer/gmulti_mesh_renderer.h" "src/engine/rendering/renderer/gmulti_mesh_renderer.cpp"   "include/engine/rendering/vulkan/igvulkan_storage_buffer.h" 
 "include/engine/rendering/vulkan/igvulkan_indirect_buffer.h" "internal/engine/rendering/vulkan/gvulkan_indirect_buffer.h" "src/engine/rendering/vulkan/gvulkan_indirect_buffer.cpp"
 "internal/engine/rendering/vulkan/gvulkan_storage_buffer.h" "src/engine/rendering/vulkan/gvulkan_storage_buffer.cpp" "internal/engine/rendering/anim/flipbook/flipbook.h" 
 "src/engine/rendering/anim/flipbook/flipbook.cpp" "internal/engine/rendering/renderer/gmulti_mesh_renderer_layout.h" "src/engine/rendering/renderer/gmulti_mesh_renderer_layout.cpp" 
 "include/engine/rendering/scene/scene_node.h" "include/engine/rendering/scene/scene.h" "src/engine/rendering/scene/scene.cpp" "include/engine/rendering/gputypes.h" 
 "include/engine/rendering/material/gmaterial.h" "src/engine/rendering/material/gmaterial.cpp" "include/engine/rendering/scene/scene_config.h" "include/engine/manager/igscene_manager.h" 
 "internal/engine/manager/gscene_manager.h" "src/engine/manager/gscene_manager.cpp" "src/engine/rendering/mesh_data.cpp" "include/engine/rendering/wireframe_spec.h" 
 "internal/engine/rendering/renderer/gscene_renderer.h" "src/engine/rendering/renderer/gscene_renderer.cpp" "internal/engine/rendering/renderer/gscene_wireframe_renderer_layout.h"
 "src/engine/rendering/renderer/gscene_wireframe_renderer_layout.cpp"  "include/engine/scene/gscene_2.h" "src/engine/scene/gscene_2.cpp"
 "internal/engine/rendering/renderer/gscene_material_renderer_layout.h" "src/engine/rendering/renderer/gscene_material_renderer_layout.cpp" 
 "internal/engine/rendering/vulkan/named/gvulkan_named_deferred_viewport.h" "src/engine/rendering/vulkan/named/gvulkan_named_deferred_viewport.cpp" 
 "include/engine/rendering/vulkan/named/igvulkan_named_viewport.h" "internal/engine/rendering/renderer/gdeferred_renderer_layout.h"
 "src/engine/rendering/renderer/gdeferred_renderer_layout.cpp"  "internal/engine/rendering/renderer/gscene_composition_renderer_layout.h"
 "src/engine/rendering/renderer/gscene_composition_renderer_layout.cpp" "internal/engine/rendering/gpu_resources.h" "include/engine/rendering/igpu_resources.h" 
 "src/engine/rendering/gpu_resources.cpp" "internal/engine/rendering/gpu_mesh_stream_resources.h" "src/engine/rendering/gpu_mesh_stream_resouces.cpp"
 "include/engine/rendering/vulkan/named/igvulkan_named_set.h" "internal/engine/rendering/renderer/gscene_renderer2.h" "src/engine/rendering/renderer/gscene_renderer2.cpp" 
 "internal/engine/rendering/vulkan/named/gvulkan_named_graphic_pipeline.h" "src/engine/rendering/vulkan/named/gvulkan_named_graphic_pipeline.cpp" 
 "include/engine/rendering/vulkan/named/viewports/igvulkan_named_deferred_viewport.h" "internal/engine/rendering/vulkan/named/gvulkan_named_pipeline_layout.h"
 "src/engine/rendering/vulkan/named/gvulkan_named_pipeline_layout.cpp" "include/engine/rendering/vulkan/named/viewports/igvulkan_named_composition_viewport.h"
 "include/engine/rendering/renderer/igvulkan_deferred_renderer.h" "internal/engine/rendering/vulkan/named/viewports/gvulkan_named_base_deferred_viewport.h" 
 "src/engine/rendering/vulkan/named/viewports/gvulkan_named_base_deferred_viewport.cpp" "include/engine/rendering/vulkan/named/igvulkan_named_graphic_pipeline.h" 
 "include/engine/rendering/point_light.h" "internal/engine/rendering/gpu_meshlet_stream_resources.h" "internal/engine/rendering/gpu_helpers.h" 
 "src/engine/rendering/gpu_meshlet_stream_resources.cpp" "src/engine/scene/gentity.cpp" "include/engine/scene/gentity.h" "internal/engine/scene/component/transform_component.h" "src/engine/scene/component/transform_component.cpp" "internal/engine/scene/gregistry.h" "src/engine/scene/gregistry.cpp" "internal/engine/scene/component/script_component.h" "include/engine/rendering/scene/iserializable.h")

target_include_directories(GEngine_Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../GObject/include")
target_include_directories(GEngine_Engine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(GEngine_Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/volk")
target_include_directories(GEngine_Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/vulkan/include")
target_include_directories(GEngine_Engine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/")
target_include_directories(GEngine_Engine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/simdjson/include")
target_include_directories(GEngine_Engine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/spdlog/include")
target_include_directories(GEngine_Engine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/vma/include")
target_include_directories(GEngine_Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/glslang/glslang/Include")
target_include_directories(GEngine_Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/taskflow")
target_include_directories(GEngine_Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/spirv_cross/include")
target_include_directories(GEngine_Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/spirv-reflect/")
target_include_directories(GEngine_Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/entt/single_include")

set(TF_BUILD_TESTS 0)
set(BUILD_TESTING 0)
set(ENABLE_HLSL 1)
set(SPIRV_REFLECT_EXAMPLES 0)
set(SPIRV_REFLECT_STATIC_LIB 1)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../GObject" GOBJECT)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/volk" VOLK)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/vulkan" VULKAN)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/simdjson" SIMDJSON)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/spdlog" SPDLOG)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/vma" VMA)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/glslang" GLSLANG)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/taskflow" TASKFLOW)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/spirv-headers" SPIRV_HEADERS)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/spirv-tools" SPIRV-TOOLS)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/spirv_cross" SPIRV-CROSS)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/spirv-reflect" SPIRV-REFLECT)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/entt/" ENTT)

target_compile_definitions(simdjson PUBLIC SIMDJSON_EXCEPTIONS=OFF)
target_compile_definitions(glslang PRIVATE BUILD_TESTING=OFF ENABLE_HLSL=ON ENABLE_OPT=ON)


target_link_libraries(GEngine_Engine PUBLIC volk)
target_link_libraries(GEngine_Engine PRIVATE simdjson)
target_link_libraries(GEngine_Engine PRIVATE GEngine_Core)
target_link_libraries(GEngine_Engine PUBLIC GEngine_GObject)
target_link_libraries(GEngine_Engine PRIVATE spdlog)
target_link_libraries(GEngine_Engine PRIVATE GPUOpen::VulkanMemoryAllocator VulkanMemoryAllocator)
target_link_libraries(GEngine_Engine PUBLIC glslang MachineIndependent glslang-default-resource-limits SPIRV)
target_link_libraries(GEngine_Engine PUBLIC SPIRV-Tools-static)
target_link_libraries(GEngine_Engine PUBLIC spirv-cross-core spirv-cross-hlsl spirv-cross-hlsl)
target_link_libraries(GEngine_Engine PUBLIC spirv-reflect-static)

include(GenerateExportHeader)

GENERATE_EXPORT_HEADER(GEngine_Engine
    BASE_NAME GEngine_Engine
    EXPORT_MACRO_NAME ENGINE_API
    EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/include/engine/GEngine_EXPORT.h"
    STATIC_DEFINE ENGINE_AS_STATIC
)