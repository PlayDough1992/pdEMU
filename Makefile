# Simple Makefile for building without CMake
CXX = g++
SDL2_CFLAGS = $(shell pkg-config --cflags sdl2)
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude -Iexternal/imgui -Iexternal/imgui/backends $(SDL2_CFLAGS)
LDFLAGS = -lSDL2 -lSDL2_image -lGL -ldl -lpthread

TARGET = mGBA_Frontend
SRC_DIR = src
OBJ_DIR = build/obj
BIN_DIR = build
IMGUI_DIR = external/imgui

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
IMGUI_SOURCES = $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp \
                $(IMGUI_DIR)/imgui_widgets.cpp $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp \
                $(IMGUI_DIR)/backends/imgui_impl_sdlrenderer2.cpp

OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
IMGUI_OBJECTS = $(OBJ_DIR)/imgui.o $(OBJ_DIR)/imgui_draw.o $(OBJ_DIR)/imgui_tables.o \
                $(OBJ_DIR)/imgui_widgets.o $(OBJ_DIR)/imgui_impl_sdl2.o $(OBJ_DIR)/imgui_impl_sdlrenderer2.o

all: $(BIN_DIR)/$(TARGET)

$(BIN_DIR)/$(TARGET): $(OBJECTS) $(IMGUI_OBJECTS) | $(BIN_DIR)
	$(CXX) $(OBJECTS) $(IMGUI_OBJECTS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/imgui.o: $(IMGUI_DIR)/imgui.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/imgui_draw.o: $(IMGUI_DIR)/imgui_draw.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/imgui_tables.o: $(IMGUI_DIR)/imgui_tables.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/imgui_widgets.o: $(IMGUI_DIR)/imgui_widgets.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/imgui_impl_sdl2.o: $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/imgui_impl_sdlrenderer2.o: $(IMGUI_DIR)/backends/imgui_impl_sdlrenderer2.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

run: $(BIN_DIR)/$(TARGET)
	./$(BIN_DIR)/$(TARGET)

.PHONY: all clean run
