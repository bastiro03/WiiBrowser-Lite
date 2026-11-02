CXX ?= g++
CC ?= gcc
FLEX ?= flex
BISON ?= bison
BUILD := ../build/host-tests
SANITIZE ?= 1
CXXFLAGS := -std=c++17 -O1 -g -Wall -Wextra -Wformat=2 -Werror=format-security -Werror=return-type -Wno-unused-parameter -I../src/utils -I../src/html_parser
ifeq ($(SANITIZE),1)
CXXFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
endif
CFLAGS := -std=c17 -D_DEFAULT_SOURCE -O1 -g -Wall -Wextra -Werror=implicit-function-declaration -Werror=return-type -I../src/html_parser -I$(BUILD)/css
ifeq ($(SANITIZE),1)
CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
endif

.PHONY: all
all: $(BUILD)/unit $(BUILD)/css-test
	$(BUILD)/unit
	$(BUILD)/css-test

$(BUILD)/unit: test_safety.cpp ../src/utils/safety.cpp ../src/utils/file_io.cpp ../src/utils/url_helper.cpp ../src/html_parser/Node.cpp ../src/html_parser/ParserDom.cpp ../src/html_parser/ParserSax.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD)/css/css_syntax.c $(BUILD)/css/css_syntax.h &: ../src/html_parser/css_syntax.y
	@mkdir -p $(BUILD)/css
	$(BISON) --defines=$(BUILD)/css/css_syntax.h -o $(BUILD)/css/css_syntax.c $<

$(BUILD)/css/css_lex.c: ../src/html_parser/css_lex.l $(BUILD)/css/css_syntax.h
	$(FLEX) -o $@ $<

$(BUILD)/css-test: test_css.c ../src/html_parser/parser.c $(BUILD)/css/css_syntax.c $(BUILD)/css/css_lex.c
	$(CC) $(CFLAGS) $^ -o $@
