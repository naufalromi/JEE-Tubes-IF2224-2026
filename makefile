#Configurasi Compiler
CXX = g++

CXXFLAGS = -Wall -Wextra -std=c++17 -g -O0 -fsanitize=address -fsanitize=undefined

TARGET = jeecompiler

SRC_DIR = src
BUILD_DIR = build
CODEGEN_DIR = src/codegen
INTERPRETER_DIR = src/interpreter

CODEGEN_BUILD_DIR = build/codegen
INTERPRETER_BUILD_DIR = build/interpreter

SRCS = $(SRC_DIR)/main.cpp\
	   $(wildcard $(SRC_DIR)/common/*.cpp)\
	   $(wildcard $(SRC_DIR)/lexer/*.cpp)\
	   $(wildcard $(SRC_DIR)/syntax/*.cpp)\
	   $(wildcard $(SRC_DIR)/semantic/*.cpp)\
	   $(wildcard $(CODEGEN_DIR)/*.cpp)\
	   $(wildcard $(INTERPRETER_DIR)/*.cpp)

HDRS = $(shell find $(SRC_DIR) -name '*.hpp')

# Menentuka file Object (.o)
# Mengubah path src/xxx/file.cpp menjadi build/xxx/file.o
OBJS = $(patsubst src/%,build/%,$(SRCS:.cpp=.o))

all: $(TARGET)

run: $(TARGET)
	./$(TARGET)

# Proses Linking (Menyatukan file .o menjadi program aplikasi)
$(TARGET) : $(OBJS)
		echo "Linking file.."
		$(CXX) $(CXXFLAGS) -o $@ $^ 
		@echo "Build sukses! Jalankan dengan ./$(TARGET)"


# Proses Compile merubah .cpp menjadi .o
build/%.o: src/%.cpp $(HDRS)
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Membersihkan File Hasil Compile (Dijalankan dengan 'make clean')
clean: 
	@echo "Membersihkan file build..."
	rm -rf $(BUILD_DIR)/*


# Memberitahu make bahwa 'all', 'run' dan 'clean' bukanlah nama file
.PHONY: all run clean

