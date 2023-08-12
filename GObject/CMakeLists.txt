cmake_minimum_required(VERSION 3.5)

project(GEngine_GObject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



add_library(GEngine_GObject SHARED "src/gobject/gtype.cpp" "include/gobject/gtype_info.h" "include/gobject/gproperty.h" "include/gobject/gfunction.h"  
"include/gobject/gfunction_wrapper.h" "src/gobject/gfunction.cpp" "include/gobject/ginstance.h" "src/gobject/ginstance.cpp" "include/gobject/gobject.h" "include/gobject/gobject_db.h"
"src/gobject/gobject_db.cpp" "include/gobject/gobject_utils.h" "src/gobject/gobject_utils.cpp" "include/gobject/gvariant.h" "src/gobject/gvariant.cpp" "src/gobject/gfunction_wrapper.cpp" "include/gobject/gfunction_execution_error.h" "include/gobject/gtype_utils.h" "internal/gobject_db_data.h")

target_include_directories(GEngine_GObject PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(GEngine_GObject PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/")
target_include_directories(GEngine_GObject PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../Core/include")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../Core" CORE)
target_link_libraries(GEngine_GObject PUBLIC GEngine_Core)


include(GenerateExportHeader)

GENERATE_EXPORT_HEADER(GEngine_GObject
    BASE_NAME GEngine_GObject
    EXPORT_MACRO_NAME GOBJECT_API
    EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/include/gobject/GEngine_EXPORT.h"
    STATIC_DEFINE GOBJECT_AS_STATIC
)