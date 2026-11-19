CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -Werror

SRC := src
BUILD := build
EXES := security-analyst dataset-gen

BOOLTRDIRS := $(addprefix $(BOOLTRAINING)/,$(BOOLTRSUBDIR))

.PHONY: all clean directories
all: directories $(EXES)

directories: $(BOOLTRDIRS) 

# main.cpp
security-analyst: $(BUILD)/main.o
	$(CXX) $(CXXFLAGS) $^ -o $@ 

# Dataset generator
dataset-gen: $(BUILD)/dataset-generator.o
	$(CXX) $(CXXFLAGS) $^ -o $@

# Make a build directory for all the ugly .o files
$(BUILD):
	mkdir -p $@

$(BUILD)/%.o: $(SRC)/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(BUILD)/*.o $(EXES)
