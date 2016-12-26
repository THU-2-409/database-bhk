cmake_minimum_required(VERSION 3.6)
project(database_bhk)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp record/Record.cpp)
add_executable(database_bhk ${SOURCE_FILES})