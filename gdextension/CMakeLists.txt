cmake_minimum_required(VERSION 3.10)

project(fps_gdextension LANGUAGES CXX)

# Default API version to match the installed Godot editor (4.7), without
# overriding an explicit -DGODOTCPP_API_VERSION=... passed on the command
# line. This mirrors how godot-cpp's own CMakeLists.txt sets a default for
# its integration-testing target.
set(GODOTCPP_DEFAULT_API_VERSION "4.7")

add_subdirectory(godot-cpp)

add_library(fps SHARED
	src/register_types.cpp
	src/player.cpp
)

target_include_directories(fps PRIVATE src)
target_link_libraries(fps PRIVATE godot-cpp)

get_target_property(GODOTCPP_SUFFIX godot-cpp GODOTCPP_SUFFIX)

set(GAME_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../game/bin")

set_target_properties(fps PROPERTIES
	CXX_STANDARD 17
	CXX_EXTENSIONS OFF
	POSITION_INDEPENDENT_CODE ON
	PREFIX "lib"
	OUTPUT_NAME "fps${GODOTCPP_SUFFIX}"
	LIBRARY_OUTPUT_DIRECTORY "${GAME_BIN_DIR}"
	RUNTIME_OUTPUT_DIRECTORY "${GAME_BIN_DIR}"
)
