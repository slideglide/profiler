cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(fxprof VERSION 1.0.0)

add_library(${PROJECT_NAME} STATIC src/fxprof.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC include)

include(cmake/CPM.cmake)

if (NOT TARGET mat-json)
    CPMAddPackage("gh:geode-sdk/json@3.2.1")
endif()

if (NOT TARGET fmt::fmt)
    CPMAddPackage("gh:fmtlib/fmt#11.1.4")
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC mat-json fmt::fmt)

add_executable(fxprof-test tests/main.cpp)
target_link_libraries(fxprof-test PRIVATE fxprof mat-json-impl)