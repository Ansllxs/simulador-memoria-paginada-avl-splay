CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Iinclude
SRC      := $(wildcard src/*.cpp)
OBJ      := $(SRC:.cpp=.o)
TARGET   := simulador

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

test1: $(TARGET)
	./$(TARGET) input/test1.txt

test2: $(TARGET)
	./$(TARGET) input/test2_errores.txt

test3: $(TARGET)
	./$(TARGET) input/test3_lru.txt

clean:
	rm -f src/*.o $(TARGET) $(TARGET).exe

.PHONY: all run test1 test2 test3 clean
