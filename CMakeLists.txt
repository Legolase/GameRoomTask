cmake_minimum_required(VERSION 3.28.3)
project(task)

# set(CMAKE_CXX_COMPILER g++-14)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O0")

option(BUILD_TEST "GTest turned on")

set(SOURCES
  src/base_parser/CharSource.cpp
  src/base_parser/BaseParser.cpp
  src/log.cpp
  src/RevenuerManager.cpp
  src/types/RevenuerManagerData.cpp
  src/types/InputEvent.cpp
)

if(BUILD_TEST)
  set(SOURCES ${SOURCES} test/test.cpp)
else()
  set(SOURCES ${SOURCES} src/main.cpp)
endif()

add_subdirectory(extern/googletest)

add_executable(${PROJECT_NAME} ${SOURCES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=leak")

target_include_directories(${PROJECT_NAME} PRIVATE include)

if(BUILD_TEST)
  target_link_libraries(${PROJECT_NAME} PRIVATE gtest_main)
endif()
