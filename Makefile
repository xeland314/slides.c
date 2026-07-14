CC = gcc
CFLAGS = -Wall -Wextra -fPIC
AR = ar
RM = rm -f

PREFIX ?= /usr/local
DESTDIR ?=

# Versión unificada (fuente única: src/version.h)
VERSION := $(shell grep '^#define VERSION ' src/version.h | sed 's/.*"\(.*\)"/\1/')

# Detección de Sistema Operativo
ifeq ($(OS),Windows_NT)
    PLATFORM_LIBS = -lgdi32 -luser32
    PKGS = cairo pangocairo
    BACKEND_SRC = src/ui/backend_win32.c
    TARGET_EXE = slides.exe
    TARGET_DLL = slider.dll

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
CORE_COMMON_SRC = src/core/parser.c src/core/themes.c src/core/highlighter.c src/render/renderer.c src/render/render_util.c src/render/render_table.c src/render/render_code.c src/render/render_transition.c src/render/export_png.c src/render/export_jpg.c src/render/export_pdf.c src/render/export_gif.c src/render/export_svg.c src/core/lexer_c.c src/core/lexer_py.c src/core/lexer_go.c src/core/lexer_js.c src/core/lexer_ts.c src/core/lexer_tsx.c src/core/lexer_html.c src/core/lexer_css.c src/core/lexer_sh.c
CORE_SRC = $(CORE_COMMON_SRC) $(BACKEND_SRC)
CORE_OBJ = $(CORE_SRC:.c=.o)

MAIN_SRC = src/ui/main.c src/ui/help.c
MAIN_OBJ = $(MAIN_SRC:.c=.o)

all: $(TARGET_EXE) $(TARGET_DLL)

$(TARGET_EXE): $(CORE_OBJ) $(MAIN_OBJ)
	$(CC) $^ -o $@ $(LIBS)

$(TARGET_DLL): $(CORE_OBJ)
	$(CC) -shared -o $@ $(CORE_OBJ) $(LIBS)

# --- Man page ---
slides.1: slides.1.in
	sed 's/@VERSION@/$(VERSION)/g' $< > $@

# --- Tests (Python) ---
test: $(TARGET_DLL)
	python3 ports/python/run_all_tests.py

# --- Coverage ---
ifeq ($(OS),Windows_NT)
    COV_DLL = slider_coverage.dll
else
    COV_DLL = libslider_coverage.so
endif
COV_CFLAGS = $(CFLAGS) -fprofile-arcs -ftest-coverage

coverage: clean_coverage src/core/lexer_c.c src/core/lexer_py.c src/core/lexer_go.c src/core/lexer_js.c src/core/lexer_ts.c src/core/lexer_tsx.c src/core/lexer_html.c src/core/lexer_css.c src/core/lexer_sh.c
	$(CC) -shared -o $(COV_DLL) $(CORE_SRC) $(LIBS) $(COV_CFLAGS)
	cp $(COV_DLL) $(TARGET_DLL)
	python3 ports/python/run_all_tests.py
	@echo ""
	@echo "=== Coverage Report ==="
	@for src in parser themes highlighter renderer render_util render_table \
	            render_code render_transition export_png export_jpg export_pdf \
	            export_gif export_svg; do \
		gcov $(COV_DLL)-$$src.gcno 2>/dev/null | grep -E "File|Lines executed" | head -2; \
	done
	@echo ""
	@echo "=== Per-file .gcov generated. Use 'gcov <file>.gcov' for line-by-line details ==="

clean_coverage:
	$(RM) *.gcda *.gcno *.gcov
	$(RM) slider_coverage.dll libslider_coverage.so
	$(RM) coverage -rf

# Regla para compilar archivos .l usando Flex automáticamente
src/core/lexer_%.c: src/core/lexer_%.l
	flex -o $@ $<

# --- Regla genérica de compilación ---
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(CORE_OBJ) $(MAIN_OBJ) $(TARGET_EXE) $(TARGET_DLL)
	$(RM) slides libslider.so
	$(RM) slides.1
	$(RM) src/core/lexer_c.c src/core/lexer_py.c src/core/lexer_go.c src/core/lexer_js.c src/core/lexer_ts.c src/core/lexer_tsx.c src/core/lexer_html.c src/core/lexer_css.c src/core/lexer_sh.c
	$(RM) *.gcda *.gcno *.gcov
	$(RM) slider_coverage.dll libslider_coverage.so
	$(RM) coverage -rf

install: $(TARGET_EXE) slides.1
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(TARGET_EXE) $(DESTDIR)$(PREFIX)/bin/
	install -d $(DESTDIR)$(PREFIX)/share/man/man1
	install -m 644 slides.1 $(DESTDIR)$(PREFIX)/share/man/man1/

.PHONY: all clean test install coverage clean_coverage
