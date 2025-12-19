cmake_minimum_required(VERSION 3.15...3.30)

project(a-scheduler VERSION 1.0
    DESCRIPTION "A scheduler in cpp"
                  LANGUAGES CXX)

set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Catch2 3 REQUIRED)
find_package(Threads REQUIRED)
 
add_executable(tests tests.cpp)
add_executable(example example.cpp)
 
target_link_libraries(tests PRIVATE Threads::Threads PRIVATE Catch2::Catch2WithMain)
target_link_libraries(example PRIVATE Threads::Threads )
