cmake_minimum_required(VERSION 3.5)

project(GEngine_Core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_library(GEngine_Core SHARED "${CMAKE_CURRENT_SOURCE_DIR}/src/public/gstring.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/public/platform/window.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/public/core/globals.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/src/public/os_str.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/internal/platform/window_glfw.cpp" "include/public/globals.h" "include/public/core/templates/signal/gsignal.h" "include/public/core/templates/better_enum.h" "include/public/core/string/gstring_utils.h")


set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/glfw" GLFW)

include(GenerateExportHeader)

GENERATE_EXPORT_HEADER(GEngine_Core
    BASE_NAME GEngine_Core
    EXPORT_MACRO_NAME CORE_API
    EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/include/public/GEngine_EXPORT.h"
    STATIC_DEFINE CORE_AS_STATIC
)


target_include_directories(GEngine_Core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(GEngine_Core glfw)
add_subdirectory(test)


install(TARGETS GEngine_Core
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})