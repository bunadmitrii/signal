cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_FLAGS "-Wall -Wshadow -Wextra -pedantic")

project(hello-c-vscode)

include_directories("${PROJECT_SOURCE_DIR}/net/include")
add_subdirectory(net)


include_directories("${PROJECT_SOURCE_DIR}/sound/include")
add_subdirectory(sound)

add_executable(bin.out main.c application.c)
target_link_libraries(bin.out sound)
target_link_libraries(bin.out net)
target_link_libraries(bin.out asound)