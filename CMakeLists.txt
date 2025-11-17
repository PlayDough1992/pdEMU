cmake_minimum_required(VERSION 3.10)
project(pdEMU VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages
find_package(SDL2 REQUIRED)

# Try to find SDL2_image
if(WIN32)
    # On Windows with MSYS2, we need to find SDL2_image manually
    find_library(SDL2_IMAGE_LIBRARY NAMES SDL2_image)
    if(SDL2_IMAGE_LIBRARY)
        set(SDL2_IMAGE_LIBRARIES ${SDL2_IMAGE_LIBRARY})
    else()
        # If not found, we'll try pkg-config
        find_package(PkgConfig)
        if(PKG_CONFIG_FOUND)
            pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
        endif()
    endif()
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
endif()

# Find OpenGL
find_package(OpenGL REQUIRED)

# ImGui source files
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

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
    src/controller_profile.cpp
    ${IMGUI_SOURCES}
)

# Windows resource file (icon)
if(WIN32)
    set(RESOURCE_FILE ${CMAKE_SOURCE_DIR}/pdemu.rc)
    list(APPEND SOURCES ${RESOURCE_FILE})
endif()

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
    include/controller_profile.h
)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/json
    ${SDL2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries - platform specific
if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        ${SDL2_LIBRARIES}
        ${SDL2_IMAGE_LIBRARIES}
        ${OPENGL_LIBRARIES}
        opengl32
        SDL2_image
    )
else()
    target_link_libraries(${PROJECT_NAME}
        ${SDL2_LIBRARIES}
        ${SDL2_IMAGE_LIBRARIES}
        ${OPENGL_LIBRARIES}
        dl
        pthread
    )
endif()

# Installation
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
