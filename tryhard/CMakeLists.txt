cmake_minimum_required(VERSION 3.20)
project(SigmaSolver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -fomit-frame-pointer -flto=auto")
endif()

add_executable(Solver src/main.cpp)
target_include_directories(Solver PRIVATE src)
