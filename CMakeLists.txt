cmake_minimum_required(VERSION 3.5)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(bouncy)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)

# Create your game executable target as usual
add_executable(bouncy WIN32 bouncy.cpp)

target_link_libraries(bouncy SDL3::SDL3 m)
  
