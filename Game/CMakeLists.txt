cmake_minimum_required(VERSION 4.1)

project(GEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(GEngine "main.cpp" "game.h" "game.cpp")

set(SPIRV_REFLECT_EXAMPLES 0)
set(BUILD_UNIT_TESTS  OFF)

target_include_directories(GEngine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../Core/include")
target_include_directories(GEngine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/include")
target_include_directories(GEngine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/include")


add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../Editor" EDITOR)

target_link_libraries(GEngine PRIVATE GEditor)
target_link_libraries(GEngine PRIVATE GEngine_Engine)
