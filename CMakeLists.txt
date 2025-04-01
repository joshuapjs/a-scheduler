# This is my First CMake file. Cool stuff.
set(CMAKE_CXX_STANDARD 20)
cmake_minimum_required(VERSION 3.15...3.30)
project(a-scheduler VERSION 1.0
    DESCRIPTION "A scheduler in cpp"
                  LANGUAGES CXX)

add_executable(main main.cpp)

add_library(boost INTERFACE)
target_include_directories(boost INTERFACE /opt/homebrew/include)

target_link_libraries(main PRIVATE boost)
