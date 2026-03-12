CC = gcc
CFLAGS = -Wall -Wextra $(shell pkg-config --cflags x11 cairo pangocairo)
LIBS = $(shell pkg-config --libs x11 cairo pangocairo) -lm

SRC = src/core/parser.c src/render/renderer.c src/ui/main.c
OBJ = $(SRC:.c=.o)
TARGET = slides

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
