cmake_minimum_required(VERSION 3.10)
project(main VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(
    "$<$<CONFIG:DEBUG>:-fsanitize=address,undefined>"
    "-Wall"
    "-Wextra"
)
add_link_options("$<$<CONFIG:DEBUG>:-fsanitize=address,undefined>")

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(main main.cpp ByteStr.cpp)
target_link_libraries(main gtest_main)
include(GoogleTest)
gtest_discover_tests(main)

