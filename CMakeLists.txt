cmake_minimum_required(VERSION 3.20)
project(image_quantization)

set(CMAKE_CXX_STANDARD 20)

# source files
set(SOURCES
        src/main.cpp
        src/Image.cpp
        src/Image_color_quantization.cpp
)
include_directories(src)
# executable
add_executable(${PROJECT_NAME} ${SOURCES})