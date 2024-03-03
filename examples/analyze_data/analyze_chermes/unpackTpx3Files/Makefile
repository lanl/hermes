# Compiler and flags
CXX = g++
CFLAGS = -Wall -O2
CXXFLAGS = $(CFLAGS) -Iinc -std=c++17

# Check if the directory exists and CHERMES_DIR is set
ifeq ($(CHERMES_DIR),)
$(error CHERMES_DIR is not set. Please set the environment variable CHERMES_DIR to the appropriate directory.)
else ifneq ($(wildcard $(CHERMES_DIR)/.),)
CXXFLAGS += -I$(CHERMES_DIR)/inc
else
$(error Directory $(CHERMES_DIR) does not exist)
endif


# Directories
WRK_DIR = ./
SRC_DIR = $(CHERMES_DIR)/src
OBJ_DIR = $(CHERMES_DIR)/obj
INC_DIR = $(CHERMES_DIR)/inc

# Files
SRCS = $(filter-out $(SRC_DIR)/histgrams.cpp, $(wildcard $(SRC_DIR)/*.cpp)) $(WRK_DIR)/tpx3Unpacker.cpp
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
OBJS := $(OBJS:$(WRK_DIR)/%.cpp=$(WRK_DIR)/%.o)
DEPS = $(wildcard $(INC_DIR)/*.h)

# Target name
TARGET = tpx3Unpacker

# Rules
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(WRK_DIR)/%.o: $(WRK_DIR)/%.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ_DIR)/*.o $(WRK_DIR)/*.o $(TARGET)

.PHONY: all clean
