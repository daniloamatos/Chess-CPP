cmake_minimum_required(VERSION 3.16)

project(chess LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(RAYLIB REQUIRED IMPORTED_TARGET raylib)
find_package(lunasvg REQUIRED)

add_executable(chess main.cpp chess.cpp)
target_link_libraries(chess PRIVATE PkgConfig::RAYLIB)
target_link_libraries(chess PRIVATE lunasvg::lunasvg)
