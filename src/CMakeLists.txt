cmake_minimum_required(VERSION 3.9)
project(first_try)

set(CMAKE_CXX_STANDARD 11)
set(EXTERNAL_LIBRARY_PATH "${CMAKE_SOURCE_DIR}/../3rd-party/lib")
set(EXTERNAL_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/../3rd-party/include")
set(EXTERNAL_SOURCES_PATH "${CMAKE_SOURCE_DIR}/../3rd-party/src")
file(GLOB EXTERNAL_SRC
        "${EXTERNAL_SOURCES_PATH}/*.c"
        "${EXTERNAL_SOURCES_PATH}/*.cc"
        "${EXTERNAL_SOURCES_PATH}/*.cpp"
    )

include_directories(${EXTERNAL_INCLUDE_PATH})
find_library(GLFW glfw3 HINTS ${EXTERNAL_LIBRARY_PATH})
find_library(ASSIMP assimp HINTS ${EXTERNAL_LIBRARY_PATH})

add_executable(first_try main.cpp ${EXTERNAL_SRC} shader.h camera.h mesh_copy.h model.h mesh.h material.h)
target_link_libraries(first_try ${ASSIMP} GL ${GLFW} Xxf86vm X11 pthread Xrandr Xi dl Xinerama Xcursor)