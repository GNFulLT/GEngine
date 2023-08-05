enable_testing()

add_executable(GEngine_test "${CMAKE_CURRENT_SOURCE_DIR}/smart_ptr_test.cpp")

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Define the test executable and link with the library
target_link_libraries(GEngine_test PRIVATE GEngine_Core)
target_link_libraries(GEngine_test PRIVATE gtest gtest_main)

target_include_directories(GEngine_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../include" "${CMAKE_CURRENT_SOURCE_DIR}/../internal")

# Add a test case
include(GoogleTest)
gtest_discover_tests(GEngine_test)