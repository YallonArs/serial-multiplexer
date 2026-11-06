cmake_minimum_required(VERSION 3.16)

project(serial-multiplexer
	VERSION 1.0.0
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp
)
list(FILTER SOURCES EXCLUDE REGEX ".*/src/plugins/[^/]+/.*")

add_executable(serial-multiplexer
	main.cpp
	${SOURCES}
)

target_include_directories(serial-multiplexer PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	target_compile_options(serial-multiplexer PRIVATE -Wall -Wextra -Wpedantic)
endif()

file(GLOB PLUGIN_CMAKE_FILES CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/src/plugins/*/CMakeLists.txt
)
foreach(PLUGIN_CMAKE_FILE IN LISTS PLUGIN_CMAKE_FILES)
	get_filename_component(PLUGIN_DIR "${PLUGIN_CMAKE_FILE}" DIRECTORY)
	add_subdirectory("${PLUGIN_DIR}")
endforeach()
