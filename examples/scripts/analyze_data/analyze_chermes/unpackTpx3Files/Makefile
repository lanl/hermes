# Compiler and flags
CXX = g++
CFLAGS = -Wall -O2
CXXFLAGS = $(CFLAGS) -Iinc -std=c++17

# Check if the directory exists and cHERMES_DIR is set
ifeq ($(cHERMES_DIR),)
$(error cHERMES_DIR is not set. Please set the environment variable cHERMES_DIR to the appropriate directory.)
else ifneq ($(wildcard $(cHERMES_DIR)/.),)
CXXFLAGS += -I$(cHERMES_DIR)/inc
else
$(error Directory $(cHERMES_DIR) does not exist)
endif

# Directories
WRK_DIR = ./
SRC_DIR = $(cHERMES_DIR)/src
OBJ_DIR = $(cHERMES_DIR)/obj
INC_DIR = $(cHERMES_DIR)/inc

# Files
SRCS = $(filter-out $(SRC_DIR)/histgrams.cpp, $(wildcard $(SRC_DIR)/*.cpp)) $(WRK_DIR)/tpx3Unpacker.cpp
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
OBJS := $(OBJS:$(WRK_DIR)/%.cpp=$(WRK_DIR)/%.o)
DEPS = $(wildcard $(INC_DIR)/*.h)

# Target name
TARGET = tpx3Unpacker

# Prerequisite checks
#check-prerequisites: check-cpp17 # Target to perform all prerequisite checks

#check-cpp17: # Check for C++17 support
#	@echo "Checking for C++17 support..."
#	@echo 'int main(){}' > temp.cpp
#	@$(CXX) -x c++ -std=c++17 -o /dev/null temp.cpp 2>/dev/null || (echo "ERROR: C++17 support is required. Please update your C++ compiler." && rm temp.cpp && false)
#	@rm -f temp.cpp

# Rules
all: $(TARGET) # Main target depends on prerequisite checks and the final binary

# Rule to link the object files into the final binary
$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

# Ensure OBJ_DIR exists before compiling object files
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

 # Compile additional source files in the working directory
$(WRK_DIR)/%.o: $(WRK_DIR)/%.cpp $(DEPS) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ_DIR)/*.o $(WRK_DIR)/*.o $(TARGET)

.PHONY: all clean
