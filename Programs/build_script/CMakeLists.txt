cmake_minimum_required(VERSION 3.5)

include(${CMAKE_CURRENT_SOURCE_DIR}/gscript.cmake)

project(${GSCRIPT_PROJECT_NAME} LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#execute_process(COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../Programs/reflect_script/reflect_generator.py ./ WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
#include(${CMAKE_CURRENT_SOURCE_DIR}/greflect.cmake)
add_library(${GSCRIPT_PROJECT_NAME} SHARED ${GSCRIPT_TARGETED_CPP})

# Only Windows
set_property(TARGET GEngine_Engined PROPERTY IMPORTED_LOCATION "${GENGINE_DIR}/GEngine_Engined.dll")

set_property(TARGET GEngine_Engined PROPERTY IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/GEngine_Engined.lib")

target_include_directories(${GSCRIPT_PROJECT_NAME} PUBLIC "${GENGINE_DIR}/editor/thirdparty/GEngine/include")
target_include_directories(${GSCRIPT_PROJECT_NAME} PUBLIC "${GENGINE_DIR}/editor/thirdparty/spdlog/include")
target_include_directories(${GSCRIPT_PROJECT_NAME} PUBLIC "${GENGINE_DIR}/editor/thirdparty/GObject/include")
target_include_directories(${GSCRIPT_PROJECT_NAME} PUBLIC "${GENGINE_DIR}/editor/thirdparty/entt/single_include")
target_include_directories(${GSCRIPT_PROJECT_NAME} PUBLIC "${GENGINE_DIR}/editor/thirdparty/glm")
target_include_directories(${GSCRIPT_PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_compile_definitions(${GSCRIPT_PROJECT_NAME} PRIVATE GSCRIPT_SPACE_NAME=${GSCRIPT_NAMESPACE_NAME})

GENERATE_EXPORT_HEADER(${GSCRIPT_PROJECT_NAME}
    BASE_NAME ${GSCRIPT_PROJECT_NAME}
    EXPORT_MACRO_NAME ${GSCRIPT_PROJECT_NAME}_API
    EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/include/${GSCRIPT_PROJECT_NAME}/${GSCRIPT_PROJECT_NAME}_EXPORT.h"
    STATIC_DEFINE ${GSCRIPT_PROJECT_NAME}_AS_STATIC
)

#${GREFLECT_GENERATED_CPP}
