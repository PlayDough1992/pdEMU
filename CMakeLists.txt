cmake_minimum_required(VERSION 3.10)
project(mGBA_Frontend VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages
find_package(SDL2 REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    src/libretro_core.cpp
    src/video_renderer.cpp
    src/audio_renderer.cpp
    src/input_handler.cpp
    src/config_manager.cpp
    src/rom_manager.cpp
    src/gui_manager.cpp
    src/system_database.cpp
    src/core_manager.cpp
)

# Header files
set(HEADERS
    include/libretro.h
    include/libretro_core.h
    include/video_renderer.h
    include/audio_renderer.h
    include/input_handler.h
    include/config_manager.h
    include/rom_manager.h
    include/gui_manager.h
    include/system_database.h
    include/core_manager.h
)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${SDL2_LIBRARIES}
    dl
    pthread
)

# Installation
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
