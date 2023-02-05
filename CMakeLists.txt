cmake_minimum_required(VERSION 3.24)
project(cardinal_rogue C)

set(CMAKE_C_STANDARD 11)

add_executable(${PROJECT_NAME} src/flecs.c src/main.c)

# raylib
find_package(raylib REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${raylib_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${raylib_LIBRARIES})

# frameworks
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/data)
configure_file(data/kenney_1bit.png ${CMAKE_CURRENT_BINARY_DIR}/data/kenney_1bit.png COPYONLY)
