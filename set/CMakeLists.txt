cmake_minimum_required(VERSION 3.15)

project(set)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} SHARED src/set_impl.cpp include/ISet.h)

find_library(LLOGGER_FOUND NAMES logger PATHS libs)
find_library(LVECTOR_FOUND NAMES vector PATHS libs)

if (NOT LLOGGER_FOUND OR NOT LVECTOR_FOUND)
	message(SEND_ERROR "Libraries not found")
else()
	target_link_libraries(${PROJECT_NAME} PUBLIC ${LLOGGER_FOUND} ${LVECTOR_FOUND})
endif()

