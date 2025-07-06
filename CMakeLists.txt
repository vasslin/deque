cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 20)

project(
    deque
    LANGUAGES CXX
)

include_directories(lib)
add_subdirectory(bin)
