CXX = g++
CXXFLAGS = -g -O0 -DX11 -Wall -Wextra -std=c++17 -I src

LDFLAGS = -L./libs -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi -lFastNoise
SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = $(BIN_DIR)/obj
SHADER_DIR = res/shaders
SHADERS = $(SHADER_DIR)/shader.vert \
		  $(SHADER_DIR)/shader.tesc \
		  $(SHADER_DIR)/shader.tese \
		  $(SHADER_DIR)/shader.frag

SPVS	= $(SHADER_DIR)/vert.spv \
		  $(SHADER_DIR)/tesc.spv \
		  $(SHADER_DIR)/tese.spv \
		  $(SHADER_DIR)/frag.spv

TARGET = $(BIN_DIR)/app

# Find all .cpp files recursively
SRCS = $(shell find $(SRC_DIR) -name '*.cpp')

# Map src/... → bin/obj/...
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

all: $(TARGET)

$(SPVS): $(SHADERS)
	@mkdir -p $(BIN_DIR)/shaders
	$(SHADER_DIR)/compile.sh

# Link
$(TARGET): $(OBJS) $(SPVS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(OBJS) $(LDFLAGS) -o $(TARGET)

# Compile
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(BIN_DIR)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean