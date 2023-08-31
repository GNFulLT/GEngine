cmake_minimum_required(VERSION 3.5)

project(GEngine_Engine LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



add_library(GEngine_Engine SHARED "src/engine/gengine.cpp" "include/engine/gapplication_impl.h" "include/engine/globals.h" "src/engine/globals.cpp"
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
 "src/engine/shader/gspirv_shader.cpp" "include/engine/manager/ijob_manager.h" "internal/engine/manager/gjob_manager.h" "src/engine/manager/gjob_manager.cpp" "include/engine/time_unit.h" "internal/engine/shader/gspirv_byte_shader.h" "src/engine/shader/gspirv_byte_shader.cpp")

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

set(TF_BUILD_TESTS 0)
set(BUILD_TESTING 0)
set(ENABLE_HLSL 1)

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

include(GenerateExportHeader)

GENERATE_EXPORT_HEADER(GEngine_Engine
    BASE_NAME GEngine_Engine
    EXPORT_MACRO_NAME ENGINE_API
    EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/include/engine/GEngine_EXPORT.h"
    STATIC_DEFINE ENGINE_AS_STATIC
)