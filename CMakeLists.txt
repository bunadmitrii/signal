cmake_minimum_required(VERSION 2.8)

project(signal)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_FLAGS "-Wall -Wshadow -Wextra -pedantic")

include_directories("${PROJECT_SOURCE_DIR}/util/include")
add_subdirectory(util)

include_directories("${PROJECT_SOURCE_DIR}/net/include")
add_subdirectory(net)

include_directories("${PROJECT_SOURCE_DIR}/sound/include")
add_subdirectory(sound)

add_executable(bin.out main.c application.c apperrors.c)
target_link_libraries(bin.out sound)
target_link_libraries(bin.out net)
target_link_libraries(bin.out asound)
target_link_libraries(bin.out util)