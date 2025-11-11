cmake_minimum_required(VERSION 3.28)
project(result_hpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.17.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# Function to register test modules
function(register_test name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE GTest::gtest_main)
    gtest_discover_tests(${name})
endfunction()

register_test(result_tests tests/result_tests.cpp)