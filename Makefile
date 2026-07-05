CC = gcc
CFLAGS = -Wall -Wextra -fPIC
AR = ar
RM = rm -f
TREE_SITTER_CFLAGS = -I$(CURDIR)/tree-sitter/zig-out/include
TREE_SITTER_LIBS = $(CURDIR)/tree-sitter/zig-out/lib/libtree-sitter.a -lubsan -ldl

# --- Gramática tree-sitter-c ---
# libtree-sitter.a de arriba es solo el runtime (ts_parser_*, ts_tree_*, etc.).
# La función tree_sitter_c() vive en el parser.c generado por el repo
# tree-sitter-c, que hay que compilar y enlazar aparte.
# Sobreescribible: make TREE_SITTER_C_DIR=/otra/ruta
TREE_SITTER_C_DIR ?= $(CURDIR)/tree-sitter-c
TREE_SITTER_C_SRC := $(wildcard $(TREE_SITTER_C_DIR)/src/parser.c $(TREE_SITTER_C_DIR)/src/scanner.c)
TREE_SITTER_C_OBJ := $(TREE_SITTER_C_SRC:.c=.o)
TREE_SITTER_C_CFLAGS = -I$(TREE_SITTER_C_DIR)/src

ifeq ($(TREE_SITTER_C_SRC),)
    $(warning No se encontro tree-sitter-c en $(TREE_SITTER_C_DIR)/src (falta parser.c). Clona https://github.com/tree-sitter/tree-sitter-c ahi, o pasa TREE_SITTER_C_DIR=/ruta explicito. Sin esto el link fallara con: undefined reference to `tree_sitter_c'.)
endif

# Detección de Sistema Operativo
ifeq ($(OS),Windows_NT)
    PLATFORM_LIBS = -lgdi32 -luser32
    PKGS = cairo pangocairo
    BACKEND_SRC = src/ui/backend_win32.c
    TARGET_EXE = slides.exe
    TARGET_DLL = slider.dll
    TARGET_ADA = slides_ada.exe

    # Auto-detectar directorio MSYS2 MinGW64
    ifeq ($(shell test -d C:/msys64/mingw64 && echo 1),1)
        MINGW_DIR := C:/msys64/mingw64
    else ifeq ($(shell test -d C:/msys2/mingw64 && echo 1),1)
        MINGW_DIR := C:/msys2/mingw64
    else ifeq ($(shell test -d C:/tools/msys64/mingw64 && echo 1),1)
        MINGW_DIR := C:/tools/msys64/mingw64
    endif

    ifdef MINGW_DIR
        export PKG_CONFIG_PATH := $(MINGW_DIR)/lib/pkgconfig
    endif

    # Verificar que NO sea Cygwin GCC
    GCC_TARGET := $(shell $(CC) -dumpmachine 2>/dev/null)
    ifneq (,$(findstring cygwin,$(GCC_TARGET)))
        $(warning *******************************************************)
        $(warning Se detectó Cygwin GCC: $(GCC_TARGET))
        $(warning Asegúrate de que MinGW64 esté primero en PATH :)
        $(warning   PATH=C:\msys64\mingw64\bin;$$$$PATH)
        $(warning O usa: make CC=C:/msys64/mingw64/bin/gcc)
        $(warning *******************************************************)
    endif
else
    PLATFORM_LIBS = -lm
    PKGS = x11 cairo pangocairo
    BACKEND_SRC = src/ui/backend_x11.c
    TARGET_EXE = slides
    TARGET_DLL = libslider.so
    TARGET_ADA = slides_ada
endif

# Flags de pkg-config (con fallback informativo)
PKG_CFLAGS := $(shell pkg-config --cflags $(PKGS) 2>/dev/null)
PKG_LIBS   := $(shell pkg-config --libs   $(PKGS) 2>/dev/null)

ifeq ($(PKG_CFLAGS),)
    $(error pkg-config falló para: $(PKGS). Verifica que las dependencias estén instaladas y PKG_CONFIG_PATH apunte al directorio correcto (eg. C:\msys64\mingw64\lib\pkgconfig))
endif

CFLAGS += $(PKG_CFLAGS) $(TREE_SITTER_CFLAGS)
LIBS = $(PKG_LIBS) $(PLATFORM_LIBS) $(TREE_SITTER_LIBS)

# Fuentes y Objetos
CORE_COMMON_SRC = src/core/parser.c src/core/themes.c src/core/highlighter.c src/render/renderer.c
CORE_SRC = $(CORE_COMMON_SRC) $(BACKEND_SRC)
CORE_OBJ = $(CORE_SRC:.c=.o) $(TREE_SITTER_C_OBJ)

MAIN_SRC = src/ui/main.c
MAIN_OBJ = $(MAIN_SRC:.c=.o)

all: $(TARGET_EXE) $(TARGET_ADA) $(TARGET_DLL)

$(TARGET_EXE): $(CORE_OBJ) $(MAIN_OBJ)
	$(CC) $^ -o $@ $(LIBS)

$(TARGET_DLL): $(CORE_OBJ)
	$(CC) -shared -o $@ $(CORE_OBJ) $(LIBS)

$(TARGET_ADA): $(CORE_OBJ)
	gnatmake -aIada/ -D ada/ -o $@ ada/slides_main.adb -largs $(CORE_OBJ) $(LIBS)

# --- Tests (Python) ---
test: $(TARGET_DLL)
	python3 python/run_all_tests.py

# --- Regla para la gramática vendorizada (código generado, sin -Wall/-Wextra) ---
$(TREE_SITTER_C_OBJ): %.o: %.c
	$(CC) -fPIC -O2 -w $(TREE_SITTER_C_CFLAGS) -c $< -o $@

# --- Regla genérica de compilación ---
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(CORE_OBJ) $(MAIN_OBJ) $(TARGET_EXE) $(TARGET_ADA) $(TARGET_DLL)
	$(RM) $(TREE_SITTER_C_OBJ)
	$(RM) slides libslider.so slides_ada.exe
	$(RM) ada/*.o ada/*.ali

.PHONY: all clean test