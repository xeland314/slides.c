CC = gcc
CFLAGS = -Wall -Wextra $(shell pkg-config --cflags x11 cairo pangocairo)
LIBS = $(shell pkg-config --libs x11 cairo pangocairo) -lm

SRC = src/core/parser.c src/core/themes.c src/render/renderer.c src/ui/main.c
OBJ = $(SRC:.c=.o)
TARGET = slides
RUN_TESTS_MARKUP = run_tests_markup
RUN_TESTS_PARSER = run_tests_parser
RUN_TESTS_INTEGRATION = run_tests_integration

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LIBS)

# Reglas de tests
test: test_markup test_parser test_integration

test_markup: tests/test_markup.o src/render/renderer.o
	$(CC) tests/test_markup.o src/render/renderer.o -o $(RUN_TESTS_MARKUP) $(LIBS)
	./$(RUN_TESTS_MARKUP)

test_parser: tests/test_parser.o src/core/parser.o src/core/themes.o
	$(CC) tests/test_parser.o src/core/parser.o src/core/themes.o -o $(RUN_TESTS_PARSER) $(LIBS)
	./$(RUN_TESTS_PARSER)

test_integration: tests/test_integration.o src/core/parser.o src/core/themes.o src/render/renderer.o
	$(CC) tests/test_integration.o src/core/parser.o src/core/themes.o src/render/renderer.o -o $(RUN_TESTS_INTEGRATION) $(LIBS)
	./$(RUN_TESTS_INTEGRATION)

tests/test_markup.o: tests/test_markup.c
	$(CC) $(CFLAGS) -c $< -o $@

tests/test_parser.o: tests/test_parser.c
	$(CC) $(CFLAGS) -c $< -o $@

tests/test_integration.o: tests/test_integration.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) tests/*.o $(TARGET) $(RUN_TESTS_MARKUP) $(RUN_TESTS_PARSER) $(RUN_TESTS_INTEGRATION)

.PHONY: all clean test test_markup test_parser test_integration

