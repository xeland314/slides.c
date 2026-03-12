CC = gcc
CFLAGS = -Wall -Wextra $(shell pkg-config --cflags x11 cairo pangocairo)
LIBS = $(shell pkg-config --libs x11 cairo pangocairo) -lm

SRC = src/core/parser.c src/render/renderer.c src/ui/main.c
OBJ = $(SRC:.c=.o)
TARGET = slides
TEST_TARGET = run_tests

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LIBS)

# Regla de tests
test: tests/test_markup.o src/render/renderer.o
	$(CC) tests/test_markup.o src/render/renderer.o -o $(TEST_TARGET) $(LIBS)
	./$(TEST_TARGET)

tests/test_markup.o: tests/test_markup.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) tests/*.o $(TARGET) $(TEST_TARGET)

.PHONY: all clean test
