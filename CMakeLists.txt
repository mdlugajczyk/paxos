cmake_minimum_required(VERSION 3.5)

project(paxos)

set (CMAKE_CXX_FLAGS "-g")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11
set(CMAKE_CXX_COMPILER "clang++")

enable_testing()

file(GLOB SRCS src/*.cpp)

add_library(libpaxos SHARED
    ${SRCS}
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(libpaxos PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(test)
