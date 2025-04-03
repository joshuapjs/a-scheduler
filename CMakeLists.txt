# This is my First CMake file. Cool stuff.
set(CMAKE_CXX_STANDARD 20)
cmake_minimum_required(VERSION 3.15...3.30)
project(a-scheduler VERSION 1.0
    DESCRIPTION "A scheduler in cpp"
                  LANGUAGES CXX)

set(CMAKE_CXX_COMPILER "/usr/bin/g++")
#find_package(Boost REQUIRED COMPONENTS filesystem system)

add_executable(main main.cpp)

#if (Boost_FOUND)
#    include_directories(${Boost_INCLUDE_DIRS})
#    target_link_libraries(main ${Boost_LIBRARIES})
#endif()

