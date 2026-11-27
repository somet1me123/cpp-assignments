cmake_minimum_required(VERSION 3.16)
project(DragonFractal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(FLTK REQUIRED)

add_executable(dragon main.cpp)
target_include_directories(dragon PRIVATE ${FLTK_INCLUDE_DIR})
target_link_libraries(dragon PRIVATE ${FLTK_LIBRARIES})
