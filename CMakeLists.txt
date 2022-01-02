cmake_minimum_required(VERSION 3.10)
project(main VERSION 0.1.0)

# Compiler configuration
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
add_compile_options("$<$<CONFIG:DEBUG>:-fsanitize=address,undefined>" "-Wall"
                    "-Wextra")
add_link_options("$<$<CONFIG:DEBUG>:-fsanitize=address,undefined>")

# Third-party libraries
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
  range-v3
  URL https://github.com/ericniebler/range-v3/archive/9aa032ccd0eb4bd77f58e5b181168f1038c222c6.zip
)
FetchContent_MakeAvailable(range-v3)

add_subdirectory(extern/cryptopp)

# Project configuration
include_directories(${CMAKE_SOURCE_DIR})

enable_testing()
add_executable(
  all_tests
  set1_tests.cpp
  set2_tests.cpp
  utils.cpp
  ByteStr.cpp
  scoring.cpp
  attacks_xor.cpp
  prim.cpp)
target_link_libraries(all_tests gtest_main range-v3 cryptopp-static)
include(GoogleTest)
gtest_discover_tests(all_tests)

add_executable(scratch scratch.cpp)
target_link_libraries(scratch range-v3 cryptopp-static)

set(data_files
    challenge-data/4.txt
    challenge-data/6.txt
    challenge-data/6.output.txt
    challenge-data/7.txt
    challenge-data/8.txt
    challenge-data/10.txt
    eng_web_2012_300K-sentences.txt)

foreach(data_file IN LISTS data_files)
    configure_file("${data_file}" "${CMAKE_BINARY_DIR}/${data_file}" COPYONLY)
endforeach()
