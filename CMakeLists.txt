# Project setup
cmake_minimum_required(VERSION 3.14)
project(SQLiteCPP)
set(CMAKE_CXX_STANDARD 20)

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        b796f7d 
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Compile src files into a static library
add_library(classes STATIC
    src/buffer.cpp
    src/data.cpp
    src/statement.cpp
    src/database.cpp
    src/pager.cpp
    src/node.cpp
)
target_include_directories(classes PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/includes)

# Add main executable and link it with classes library
add_executable(db src/main.cpp)
target_link_libraries(db classes)

# Add test executable and link it with classes library and gtest
add_executable(tests test/tests.cpp)
target_link_libraries(tests 
    classes 
    gtest_main
    gmock_main
)
