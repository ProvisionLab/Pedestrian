cmake_minimum_required(VERSION 3.5)
project(annotation)

find_package(OpenCV REQUIRED)

add_executable(${PROJECT_NAME} opencv_annotation.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})