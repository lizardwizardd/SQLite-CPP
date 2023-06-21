# Project setup
cmake_minimum_required(VERSION 3.14)
project(SQLiteCPP)
set(CMAKE_CXX_STANDARD 20)

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Fetch if not fetched previously
#if (NOT googletest_POPULATED)
#    FetchContent_Populate(googletest)
#    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BUILD_DIR})
#endif()


# Compile src files into a static library
add_library(classLib STATIC
    src/buffer.cpp
    src/data.cpp
    src/statement.cpp
    src/database.cpp
)
target_include_directories(classLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/includes)

# Add main executable and link it with classes library
add_executable(mainExe src/main.cpp)
target_link_libraries(mainExe classLib)
#target_include_directories(mainExe ${CMAKE_CURRENT_SOURCE_DIR}/includes)

# Test executable
add_executable(testRunner test/tests.cpp)
target_link_libraries(testRunner 
    classLib gtest_main gmock_main
)

