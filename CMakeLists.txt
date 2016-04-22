cmake_minimum_required(VERSION 3.5)
project(blockens)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

find_package(OpenGL)

set(GLFW_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/glfw)
set(FREETYPE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/freetype)

include_directories(${GLFW_DIR}/include)
include_directories(${FREETYPE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(PLATFORM_LIBS "")
if(APPLE)
    include_directories (/System/Library/Frameworks)
    find_library(OpenGL_LIBRARY OpenGL)
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(IOKIT_FRAMEWORK IOKit)
    find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
    find_library(CORE_VIDEO_FRAMEWORK CoreVideo)

    set(PLATFORM_LIBS
            ${COCOA_FRAMEWORK}
            ${IOKIT_FRAMEWORK}
            ${CORE_FOUNDATION_FRAMEWORK}
            ${CORE_VIDEO_FRAMEWORK}
            ${GLFW_DIR}/lib/libglfw3.a
            ${FREETYPE_DIR}/lib/libfreetyped.a
    )
endif()

set(SOURCE_FILES src/main.cpp)
add_executable(blockens ${SOURCE_FILES})
target_link_libraries(blockens ${OPENGL_LIBRARIES} ${PLATFORM_LIBS})
