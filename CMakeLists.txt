cmake_minimum_required(VERSION 3.26)
project(bouncy)

find_package(SDL3 REQUIRED)

add_executable(bouncy)

target_sources(bouncy PRIVATE bouncy.c)
target_link_libraries(bouncy SDL3::SDL3-static m)
  
