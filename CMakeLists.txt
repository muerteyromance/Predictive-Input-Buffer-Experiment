cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(PredictiveInputBuffer VERSION 0.5.0)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

# Files from older versions that may still be sitting in src/ are ignored.
list(FILTER SOURCES EXCLUDE REGEX "/(Hud|Situation|Config)\\.cpp$")

add_library(${PROJECT_NAME} SHARED ${SOURCES})

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

setup_geode_mod(${PROJECT_NAME})
