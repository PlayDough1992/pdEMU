cmake_minimum_required(VERSION 3.10)
project(pdEMU VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages
find_package(SDL2 REQUIRED)

# Find OpenGL
find_package(OpenGL REQUIRED)

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
    src/splash_screen.cpp
    src/video_renderer_gl.cpp
    src/rom_config.cpp
    src/cheat_database.cpp
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
    include/splash_screen.h
    include/video_renderer_gl.h
    include/rom_config.h
    include/cheat_database.h
)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries - platform specific
if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        ${SDL2_LIBRARIES}
        ${OPENGL_LIBRARIES}
        opengl32
    )
else()
    target_link_libraries(${PROJECT_NAME}
        ${SDL2_LIBRARIES}
        ${OPENGL_LIBRARIES}
        dl
        pthread
    )
endif()

# Installation
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
