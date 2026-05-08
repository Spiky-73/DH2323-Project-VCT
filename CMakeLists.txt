cmake_minimum_required(VERSION 3.23)

project(DH2323Project)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_FLAGS "-O2 -Wall")
set(CMAKE_BUILD_TYPE Release)

IF(APPLE)
  SET(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures for Mac OS X" FORCE)
ENDIF(APPLE)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
find_package(SDL2 REQUIRED)

add_executable(DH2323Project)
target_include_directories(DH2323Project
  PUBLIC
    ${SDL2_INCLUDE_DIRS}
    vendor/glm
    vendor/SDL2Auxiliary
)
target_sources(DH2323Project
  PRIVATE
    src/main.cpp
    src/Renderer.cpp
    src/utility.cpp
    src/Shaders.cpp
    src/ShadowMap.cpp
    vendor/SDL2Auxiliary/SDL2Auxiliary.cpp
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS src
    FILES
      src/Renderer.hpp
      src/TestModel.hpp
      src/utility.hpp
      src/Objects.hpp
      src/Shaders.hpp
      src/ShadowMap.hpp

)

target_link_libraries(DH2323Project
  ${SDL2_LIBRARIES}
)