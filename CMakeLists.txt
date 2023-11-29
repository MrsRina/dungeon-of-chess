cmake_minimum_required(VERSION 3.0.0)
project(dungeon-of-chess VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

message("\n")
message("\t[WARNING] \nif the running OS is not win32, \ninstall the devel libraries, \nand the dynamic libraries necessary: \nSDL2, SDL2image, GLEW")
message("\n")

if (WIN32)
    message("-- Dungeon of Chess WIN32 building")
    set(EXECUTABLE_OUTPUT_PATH "../build/win32-build/")
    set(DUNGEON_OF_CHESS_LINK_LIBRARIES mingw32 SDL2main SDL2 SDL2_image opengl32 glew32)
else()
    message("-- Dungeon of Chess UNIX/Linux-based building")
    set(EXECUTABLE_OUTPUT_PATH "../build/linux-unix-build/")
    set(DUNGEON_OF_CHESS_LINK_LIBRARIES SDL2main SDL2 SDL2_image GL GLEW)
endif()

file(GLOB_RECURSE DUNGEON_OF_CHESS_SRC_FILES "src/*.cpp")
include_directories(src)

add_compile_options(-O3)
add_executable(dungeon-of-chess ${DUNGEON_OF_CHESS_SRC_FILES})
target_link_libraries(dungeon-of-chess ${DUNGEON_OF_CHESS_LINK_LIBRARIES}
                                       -static-libgcc -static-libstdc++) # static mingw stuff

message("-- Dungeon of Chess building done")