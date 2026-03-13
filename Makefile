CC = gcc
CFLAGS = -Wall -Wextra -fPIC

# Detección de Sistema Operativo
ifeq ($(OS),Windows_NT)
    # Configuración para Windows (MinGW)
    PLATFORM_LIBS = -lgdi32 -luser32
    # En Windows usamos cairo-win32 en lugar de x11
    PKGS = cairo pangocairo
    BACKEND_SRC = src/ui/backend_win32.c
    TARGET_EXE = slides.exe
    TARGET_DLL = slider.dll
    RM = rm -f
else
    # Configuración para Linux/Unix
    PLATFORM_LIBS = -lm
    PKGS = x11 cairo pangocairo
    BACKEND_SRC = src/ui/backend_x11.c
    TARGET_EXE = slides
    TARGET_DLL = libslider.so
    RM = rm -f
endif

# Flags de pkg-config
CFLAGS += $(shell pkg-config --cflags $(PKGS))
LIBS = $(shell pkg-config --libs $(PKGS)) $(PLATFORM_LIBS)

# Fuentes y Objetos
CORE_COMMON_SRC = src/core/parser.c src/core/themes.c src/render/renderer.c
CORE_SRC = $(CORE_COMMON_SRC) $(BACKEND_SRC)
CORE_OBJ = $(CORE_SRC:.c=.o)

MAIN_SRC = src/ui/main.c
MAIN_OBJ = $(MAIN_SRC:.c=.o)

TARGET = $(TARGET_EXE)
TARGET_ADA = slides_ada
TARGET_LIB = $(TARGET_DLL)

RUN_TESTS_MARKUP = run_tests_markup
RUN_TESTS_PARSER = run_tests_parser
RUN_TESTS_INTEGRATION = run_tests_integration

all: $(TARGET) $(TARGET_ADA) $(TARGET_LIB)

# Ejecutable principal
$(TARGET): $(CORE_OBJ) $(MAIN_OBJ)
	$(CC) $(CORE_OBJ) $(MAIN_OBJ) -o $(TARGET) $(LIBS)

# Librería dinámica (DLL o SO)
$(TARGET_LIB): $(CORE_OBJ)
	$(CC) -shared -o $(TARGET_LIB) $(CORE_OBJ) $(LIBS)

# Regla para Ada (mantenida igual, usa LIBS dinámicos)
$(TARGET_ADA): $(CORE_OBJ)
	gnatmake -aIada/ -D ada/ -o $(TARGET_ADA) ada/slides_main.adb -largs $(CORE_OBJ) $(LIBS)

# --- Reglas de tests ---
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

# --- Reglas de compilación genéricas ---
tests/%.o: tests/%.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(CORE_OBJ) $(MAIN_OBJ) tests/*.o $(TARGET_EXE) slides $(TARGET_ADA) $(TARGET_DLL) libslider.so $(RUN_TESTS_MARKUP) $(RUN_TESTS_PARSER) $(RUN_TESTS_INTEGRATION)
	$(RM) ada/*.o ada/*.ali

.PHONY: all clean test test_markup test_parser test_integration
