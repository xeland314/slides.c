CC = gcc
CFLAGS = -Wall -Wextra -fPIC
AR = ar
RM = rm -f

# Detección de Sistema Operativo
ifeq ($(OS),Windows_NT)
    PLATFORM_LIBS = -lgdi32 -luser32
    PKGS = cairo pangocairo
    BACKEND_SRC = src/ui/backend_win32.c
    TARGET_EXE = slides.exe
    TARGET_DLL = slider.dll
    TARGET_ADA = slides_ada.exe

    # Auto-detectar directorio MSYS2 MinGW64
    ifneq (,$(wildcard C:/msys64/mingw64/bin/gcc.exe))
        MINGW_DIR := C:/msys64/mingw64
    else ifneq (,$(wildcard C:/msys2/mingw64/bin/gcc.exe))
        MINGW_DIR := C:/msys2/mingw64
    else ifneq (,$(wildcard C:/tools/msys64/mingw64/bin/gcc.exe))
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

CFLAGS += $(PKG_CFLAGS)
LIBS = $(PKG_LIBS) $(PLATFORM_LIBS)

# Fuentes y Objetos
CORE_COMMON_SRC = src/core/parser.c src/core/themes.c src/core/highlighter.c src/render/renderer.c src/core/lexer_c.c src/core/lexer_py.c
CORE_SRC = $(CORE_COMMON_SRC) $(BACKEND_SRC)
CORE_OBJ = $(CORE_SRC:.c=.o)

MAIN_SRC = src/ui/main.c src/ui/help.c
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

# Regla para compilar archivos .l usando Flex automáticamente
src/core/lexer_%.c: src/core/lexer_%.l
	flex -o $@ $<

# --- Regla genérica de compilación ---
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(CORE_OBJ) $(MAIN_OBJ) $(TARGET_EXE) $(TARGET_ADA) $(TARGET_DLL)
	$(RM) slides libslider.so slides_ada.exe
	$(RM) ada/*.o ada/*.ali
	$(RM) src/core/lexer_c.c src/core/lexer_py.c

install: $(TARGET_EXE)
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(TARGET_EXE) $(DESTDIR)$(PREFIX)/bin/
	install -d $(DESTDIR)$(PREFIX)/share/man/man1
	install -m 644 slides.1 $(DESTDIR)$(PREFIX)/share/man/man1/

.PHONY: all clean test install
