CC = gcc
CFLAGS = -Wall -Wextra -fPIC $(shell pkg-config --cflags x11 cairo pangocairo)
LIBS = $(shell pkg-config --libs x11 cairo pangocairo) -lm

CORE_SRC = src/core/parser.c src/core/themes.c src/render/renderer.c src/ui/backend_x11.c
CORE_OBJ = $(CORE_SRC:.c=.o)
MAIN_SRC = src/ui/main.c
MAIN_OBJ = $(MAIN_SRC:.c=.o)

TARGET = slides
TARGET_ADA = slides_ada
TARGET_LIB = libslider.so

RUN_TESTS_MARKUP = run_tests_markup
RUN_TESTS_PARSER = run_tests_parser
RUN_TESTS_INTEGRATION = run_tests_integration

all: $(TARGET) $(TARGET_ADA) $(TARGET_LIB)

$(TARGET): $(CORE_OBJ) $(MAIN_OBJ)
	$(CC) $(CORE_OBJ) $(MAIN_OBJ) -o $(TARGET) $(LIBS)

$(TARGET_LIB): $(CORE_OBJ)
	$(CC) -shared -o $(TARGET_LIB) $(CORE_OBJ) $(LIBS)

$(TARGET_ADA): $(CORE_OBJ)
	gnatmake -aIada/ -D ada/ -o $(TARGET_ADA) ada/slides_main.adb -largs $(CORE_OBJ) $(LIBS)

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
	rm -f $(CORE_OBJ) $(MAIN_OBJ) tests/*.o $(TARGET) $(TARGET_ADA) $(TARGET_LIB) $(RUN_TESTS_MARKUP) $(RUN_TESTS_PARSER) $(RUN_TESTS_INTEGRATION)
	rm -f ada/*.o ada/*.ali

.PHONY: all clean test test_markup test_parser test_integration
