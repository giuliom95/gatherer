cmake_minimum_required (VERSION 3.10)

project(gatherer)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(GATHERER_MAJOR_VERSION 0)
set(GATHERER_MINOR_VERSION 1)
set(GATHERER_VERSION ${GATHERER_MAJOR_VERSION}.${GATHERER_MINOR_VERSION})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if(NOT MATH_INCLUDE)
  message(FATAL_ERROR "Please define MATH_INCLUDE")
endif(NOT MATH_INCLUDE)
get_filename_component(MATH_INCLUDE_ABS "${MATH_INCLUDE}"
                       REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")

include_directories(
  include
  ${MATH_INCLUDE_ABS}
)

find_package(Boost COMPONENTS log filesystem REQUIRED)
find_package(glfw3 REQUIRED)
include(FindOpenGL)
include(FindGLEW)

#set( GATHERER_SOURCES 
#)
#add_library(${PROJECT_NAME} SHARED ${GATHERER_SOURCES})
#target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

set( GATHERER_CLIENT_SOURCES 
  src/client/client.cpp
)
add_executable(${PROJECT_NAME}client ${GATHERER_CLIENT_SOURCES})
target_link_libraries(${PROJECT_NAME}client glfw)
target_link_libraries(${PROJECT_NAME}client OpenGL::GL)
target_link_libraries(${PROJECT_NAME}client GLEW::glew)
#target_link_libraries(${PROJECT_NAME}client ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}client ${Boost_LIBRARIES})