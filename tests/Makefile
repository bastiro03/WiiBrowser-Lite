CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -I../src -I../src/utils -I../src/core
TARGET := test_runner
SRC := unit/test_url_helper.cpp ../src/utils/url_helper.cpp
OBJ := $(SRC:.cpp=.o)

all: $(TARGET)
	@echo "Running host unit tests..."
	./$(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) unit/*.o

.PHONY: all clean
