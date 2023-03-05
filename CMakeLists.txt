cmake_minimum_required(VERSION 3.1)

project(ok)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Main program

add_executable(${PROJECT_NAME} ok.cpp)

# Depdencies

find_package(CURL REQUIRED) 
include_directories(${CURL_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES})

find_package(json-c CONFIG)
target_link_libraries(${PROJECT_NAME} json-c::json-c)
