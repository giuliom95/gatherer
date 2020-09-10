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

set(MATH_INCLUDE "deps/math/include")

set(JSON_INCLUDE "deps/json/single_include/nlohmann")

set(IMGUI_DIR "deps/imgui")
set(IMGUI_INCLUDE 
  "${IMGUI_DIR}"
  "${IMGUI_DIR}/examples"
)
set(IMGUI_SRCS
  "${IMGUI_DIR}/imgui_draw.cpp"
  "${IMGUI_DIR}/imgui_widgets.cpp"
  "${IMGUI_DIR}/imgui.cpp"
  "${IMGUI_DIR}/examples/imgui_impl_glfw.cpp"
  "${IMGUI_DIR}/examples/imgui_impl_opengl3.cpp"
)

include_directories(
  include
  ${MATH_INCLUDE}
  ${JSON_INCLUDE}
  ${IMGUI_INCLUDE}
)

find_package(Boost COMPONENTS log filesystem REQUIRED)
find_package(glfw3 REQUIRED)
include(FindOpenGL)
include(FindGLEW)

set( GATHERER_CLIENT_SOURCES 
  src/client/utils.cpp
  src/client/gathereddata.cpp
  src/client/pathsrenderer.cpp
  src/client/axesvisualizer.cpp
  src/client/scenerenderer.cpp
  src/client/selectionstroke.cpp
  src/client/application.cpp
  src/client/client.cpp
  ${IMGUI_SRCS}
)
add_executable(${PROJECT_NAME}client ${GATHERER_CLIENT_SOURCES})
target_link_libraries(${PROJECT_NAME}client glfw)
target_link_libraries(${PROJECT_NAME}client OpenGL::GL)
target_link_libraries(${PROJECT_NAME}client GLEW::glew)
target_link_libraries(${PROJECT_NAME}client ${Boost_LIBRARIES})