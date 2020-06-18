cmake_minimum_required(VERSION 3.15)
project(UI_lab)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} SHARED dll_code/IVectorImpl.cpp dll_code/ILoggerImpl.cpp dll_code/ISetImpl.cpp dll_code/ICompactImpl.cpp)