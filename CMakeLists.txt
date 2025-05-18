cmake_minimum_required(VERSION 3.15...3.30)

set(CMAKE_CXX_COMPILER "/opt/homebrew/bin/g++-14")
add_compile_options(-stdlib=libstdc++)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(a-scheduler VERSION 1.0
    DESCRIPTION "A scheduler in cpp"
                  LANGUAGES CXX)

add_executable(main main.cpp)
