cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(logger)

add_library(${PROJECT_NAME} SHARED src/logger_impl.cpp)
