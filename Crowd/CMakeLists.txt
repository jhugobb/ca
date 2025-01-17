cmake_minimum_required(VERSION 2.8)

cmake_policy(SET CMP0015 NEW)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall")

set(appName BaseCode)
set(CAL3D_LIB_DIR /usr/lib/)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(Freetype REQUIRED)
find_package (Eigen3 3.3 REQUIRED NO_MODULE)

include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${GLUT_INCLUDE_DIRS})
include_directories("${GLEW_INCLUDE_DIRS}")
include_directories("${CMAKE_SOURCE_DIR}/glm")
include_directories(${FREETYPE_INCLUDE_DIRS})
include_directories("/usr/include/cal3d")

link_directories(${OPENGL_LIBRARY_DIRS})
link_directories(${GLUT_LIBRARY_DIRS})
link_directories(${GLEW_LIBRARY_DIRS})
link_directories(${FREETYPE_LIBRARY_DIRS})
link_directories(${CAL3D_LIB_DIR})

add_executable(${appName} main.cpp)
target_link_libraries(${appName} ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} ${GLEW_LIBRARIES} ${FREETYPE_LIBRARIES} ${CAL3D_LIB_DIR}/libcal3d.so ${CAL3D_LIB_DIR}/libcal3d.a)

