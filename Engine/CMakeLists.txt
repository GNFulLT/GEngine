cmake_minimum_required(VERSION 3.5)

project(GEngine_Engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



add_library(GEngine_Engine SHARED "src/engine/gengine.cpp" "include/engine/gapplication_impl.h" "include/engine/globals.h" "src/engine/globals.cpp"
"src/engine/rendering/vulkan/vulkan_app.cpp" "internal/engine/rendering/vulkan/vulkan_app.h" "internal/engine/rendering/vulkan/vulkan_utils.h" 
"include/engine/rendering/vulkan/ivulkan_device.h" "internal/engine/rendering/vulkan/vulkan_device.h" "src/engine/rendering/vulkan/vulkan_device.cpp" "internal/engine/rendering/vulkan/vulkan_pdevice.h"
"src/engine/rendering/vulkan/vulkan_pdevice.cpp" "internal/engine/rendering/vulkan/vulkan_ldevice.h" "src/engine/rendering/vulkan/vulkan_ldevice.cpp" "internal/engine/rendering/vulkan/vulkan_queue.h" 
"src/engine/rendering/vulkan/vulkan_queue.cpp" "include/engine/rendering/vulkan/vulkan_command_buffer.h" "src/engine/rendering/vulkan/vulkan_command_buffer.cpp" "include/engine/imanager_table.h" 
"internal/engine/manager_table.h" "src/engine/manager_table.cpp" "internal/engine/rendering/vulkan/vulkan_viewport.h" "src/engine/rendering/vulkan/vulkan_viewport.cpp"
 "internal/engine/rendering/vulkan/vulkan_swapchain.h" "src/engine/rendering/vulkan/vulkan_swapchain.cpp" "internal/engine/rendering/vulkan/vulkan_renderpass.h" "src/engine/rendering/vulkan/vulkan_renderpass.cpp" 
 "include/engine/rendering/vulkan/ivulkan_viewport.h" "internal/engine/rendering/vulkan/vulkan_memory.h" "src/engine/rendering/vulkan/vulkan_memory.cpp" "include/engine/rendering/vulkan/ivulkan_queue.h"
 "include/engine/resource/json/igjson.h" "internal/engine/resource/json/gjson_simdjson.h" "src/engine/rendering/resource/json/gjson_simdjson.cpp" "include/engine/resource/resource_init_code.h"
 "include/engine/resource/json/gjson_utils.h" "src/engine/rendering/resource/json/gjson_utils.cpp" "include/engine/resource/json/igjson_value.h" "internal/engine/resource/json/gjson_value_simdjson.h" 
 "src/engine/rendering/resource/json/gjson_value_simdjson.cpp" "include/engine/manager/iglogger_manager.h" "internal/engine/manager/glogger_manager.h" "src/engine/manager/glogger_manager.cpp" 
 "include/engine/io/iowning_glogger.h" "internal/engine/io/gowning_glogger.h" "src/engine/io/gowning_glogger.cpp" "internal/engine/io/logger_helper.h" "include/engine/resource/iresource_impl.h"
 "include/engine/resource/iresource_listener.h" "internal/engine/resource/gresource.h" "src/engine/resource/gresource.cpp" "internal/engine/manager/gresource_manager.h" "src/engine/manager/gresource_manager.cpp" "include/engine/manager/igresource_manager.h" "include/engine/io/gimage_descriptor.h")

target_include_directories(GEngine_Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../GObject/include")
target_include_directories(GEngine_Engine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(GEngine_Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/volk")
target_include_directories(GEngine_Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/vulkan/include")
target_include_directories(GEngine_Engine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/")
target_include_directories(GEngine_Engine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/simdjson/include")
target_include_directories(GEngine_Engine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/spdlog/include")
target_include_directories(GEngine_Engine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/vma/include")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../GObject" GOBJECT)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/volk" VOLK)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/vulkan" VULKAN)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/simdjson" SIMDJSON)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/spdlog" SPDLOG)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/vma" VMA)

target_compile_definitions(simdjson PUBLIC SIMDJSON_EXCEPTIONS=OFF)

target_link_libraries(GEngine_Engine PUBLIC volk)
target_link_libraries(GEngine_Engine PRIVATE simdjson)
target_link_libraries(GEngine_Engine PRIVATE GEngine_Core)
target_link_libraries(GEngine_Engine PUBLIC GEngine_GObject)
target_link_libraries(GEngine_Engine PRIVATE spdlog)
target_link_libraries(GEngine_Engine PRIVATE GPUOpen::VulkanMemoryAllocator)
include(GenerateExportHeader)

GENERATE_EXPORT_HEADER(GEngine_Engine
    BASE_NAME GEngine_Engine
    EXPORT_MACRO_NAME ENGINE_API
    EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/include/engine/GEngine_EXPORT.h"
    STATIC_DEFINE ENGINE_AS_STATIC
)