CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -I../src -I../src/utils -I../src/core -I../src/html_parser -I../external
TARGETS := test_url_helper test_stringop test_freetype_guard
SRC_URL := unit/test_url_helper.cpp ../src/utils/url_helper.cpp
SRC_STR := unit/test_stringop.cpp
SRC_FTG := unit/test_freetype_guard.cpp

all: $(TARGETS)
	@echo "Running host unit tests..."
	./test_url_helper && ./test_stringop && ./test_freetype_guard && echo "ALL host tests PASS"

test_url_helper: $(SRC_URL:.cpp=.o)
	$(CXX) $(CXXFLAGS) -o $@ $^

test_stringop: $(SRC_STR:.cpp=.o)
	$(CXX) $(CXXFLAGS) -o $@ $^

test_freetype_guard: $(SRC_FTG:.cpp=.o)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGETS) unit/*.o ../src/utils/*.o *.o

.PHONY: all clean
