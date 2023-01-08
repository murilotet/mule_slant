cmake_minimum_required(VERSION 3.2)

project(mule-slant)

if(CMAKE_CONFIGURATION_TYPES)
    message("Multi-configuration generator")
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "My multi config types" FORCE)
else()
    message("Single-configuration generator")
endif()

enable_language(C CXX)

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -O3")

add_subdirectory(src/encoder)
add_subdirectory(src/decoder)