cmake_minimum_required(VERSION 3.14)
project (rbfx_sample_anim)

set (CMAKE_CXX_STANDARD 17)

set (Urho3D_DIR "${CMAKE_SOURCE_DIR}/../rbfx/build/install/share/CMake")
set (Urho3D_Generated_DIR "${CMAKE_SOURCE_DIR}/../rbfx/build/install/share/CMake")

include (${CMAKE_SOURCE_DIR}/../rbfx/build/install/share/CMake/Urho3DConfig.cmake)
find_package(Urho3D REQUIRED)

add_subdirectory (${CMAKE_SOURCE_DIR}/Source)
