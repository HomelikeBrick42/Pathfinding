cmake_minimum_required(VERSION 3.20)
project(Pathfinding C)

set(CMAKE_C_STANDARD 11)

set(SDL2_INCLUDE_DIR C:/Dev/SDL/include)
set(SDL2_LIB_DIR C:/Dev/SDL/lib/x64)

include_directories(${SDL2_INCLUDE_DIR})
link_directories(${SDL2_LIB_DIR})

include_directories(src)
add_executable(Pathfinding src/Main.c src/Typedefs.h)

target_link_libraries(${PROJECT_NAME} SDL2main SDL2)
