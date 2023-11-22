cmake_minimum_required(VERSION 3.5)

include(${CMAKE_CURRENT_SOURCE_DIR}/gscript.cmake)

project(${GSCRIPT_PROJECT_NAME} LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#execute_process(COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../Programs/reflect_script/reflect_generator.py ./ WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
#include(${CMAKE_CURRENT_SOURCE_DIR}/greflect.cmake)

target_include_directories(${GSCRIPT_PROJECT_NAME} PUBLIC "${GENGINE_DIR}/editor/thirdparty/GObject/include")
target_include_directories(${GSCRIPT_PROJECT_NAME} PUBLIC "${GENGINE_DIR}/editor/thirdparty/entt/single_include")
target_include_directories(${GSCRIPT_PROJECT_NAME} PUBLIC "${GENGINE_DIR}/editor/thirdparty/glm")
target_include_directories(${GSCRIPT_PROJECT_NAME} PUBLIC "include/")

#${GREFLECT_GENERATED_CPP}
add_library(${GSCRIPT_PROJECT_NAME} SHARED ${GSCRIPT_TARGETED_CPP})