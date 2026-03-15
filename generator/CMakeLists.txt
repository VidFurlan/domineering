# Project settings ============================================================
cmake_minimum_required(VERSION 3.30)
project("Generator" LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Build =======================================================================
include(FetchContent)
find_package(OpenMP REQUIRED)

FetchContent_Declare(
	nlohmann_json
	GIT_REPOSITORY https://github.com/nlohmann/json.git
	GIT_TAG v3.12.0
)

FetchContent_MakeAvailable(nlohmann_json)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)
target_include_directories(${PROJECT_NAME} PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/lib
)
target_link_libraries(${PROJECT_NAME} PRIVATE 
	nlohmann_json::nlohmann_json
	OpenMP::OpenMP_CXX
)

# Speeeed =====================================================================
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_ok OUTPUT ipo_err)
if(ipo_ok)
  set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Release>:-O3 -DNDEBUG -march=native -fomit-frame-pointer>
    $<$<CONFIG:RelWithDebInfo>:-O3 -g -DNDEBUG -march=native -fomit-frame-pointer>
  )

  target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Release>:-flto>
    $<$<CONFIG:RelWithDebInfo>:-flto>
  )
endif()


# LSP support =================================================================
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
add_custom_target(copy_compile_commands ALL
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${CMAKE_BINARY_DIR}/compile_commands.json"
	"${CMAKE_SOURCE_DIR}/compile_commands.json"
	DEPENDS  "${CMAKE_BINARY_DIR}/compile_commands.json"
	COMMENT "Sync compile_commands.json to project root"
)

# Require out-of-source builds ================================================
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
	message(FATAL_ERROR "You cannot build in a source directory (or any directory with "
		"CMakeLists.txt file). Please make a build subdirectory.")
endif()

# Build:
#	cmake -S .. -B build -DCMAKE_BUILD_TYPE=Release
#	cmake --build build -j

