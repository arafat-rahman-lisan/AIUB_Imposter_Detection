cmake_minimum_required(VERSION 3.16)
project(AIUB_Imposter_Detection LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

add_executable(AIUB_Imposter_Detection src/main.cpp)
target_include_directories(AIUB_Imposter_Detection PRIVATE ${GLUT_INCLUDE_DIRS})
target_link_libraries(AIUB_Imposter_Detection PRIVATE OpenGL::GL OpenGL::GLU GLUT::GLUT)
