cmake_minimum_required(VERSION 3.29)
project(SegmentTree)

set(CMAKE_CXX_STANDARD 20)

add_executable(SegmentTree main.cpp)
find_package(SFML 2.5 COMPONENTS graphics window REQUIRED)
if(SFML_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE sfml-graphics sfml-window)
else()
    message("SFML not found")
endif()
