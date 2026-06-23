# C-Slides

Un presentador de diapositivas minimalista y de alto rendimiento escrito en **C** utilizando **Cairo** y **Pango**. Renderiza archivos Markdown directamente en pantalla con transiciones suaves, temas visuales y exportación a PNG/PDF.

## Previsualización

| Dark (Default) | Monokai |
| :---: | :---: |
| ![Dark](examples/preview_dark.png) | ![Monokai](examples/preview_monokai.png) |

| Nord | Rose |
| :---: | :---: |
| ![Nord](examples/preview_nord.png) | ![Rose](examples/preview_rose.png) |

| Light | Ambercat |
| :---: | :---: |
| ![Light](examples/preview_light.png) | ![Ambercat](examples/preview_ambercat.png) |

| Blue | Catppuccin |
| :---: | :---: |
| ![Blue](examples/preview_blue.png) | ![Catppuccin](examples/preview_catppuccin.png) |

| Dracula | Gruvbox |
| :---: | :---: |
| ![Dracula](examples/preview_dracula.png) | ![Gruvbox](examples/preview_gruvbox.png) |

| Tokyo Night | |
| :---: | :---: |
| ![Tokyo Night](examples/preview_tokyo-night.png) | |

## Características

### Contenido Markdown
- Títulos (`#`, `##`) con barra inferior decorativa
- Párrafos con word-wrapping automático
- Listas con viñetas (anidadas nivel 1 y 2)
- Listas numeradas (`1.`, `a)`, `i)`, etc.)
- Listas de tareas (`- [ ]` / `- [x]`)
- Blockquotes con barra lateral
- Imágenes PNG/JPG/GIF/BMP (auto-escalado, caché de 64 slots)
- Tablas con ancho proporcional, wrapping, filas alternadas y cabecera
- Bloques de código (fenced) con resaltado de sintaxis
- Códulo inline (`código`)
- Énfasis: **bold**, *italic*, ***combinado***

### Transiciones entre diapositivas
- `fade` — Fundido cruzado
- `slide-left` — Entrada desde la derecha
- `slide-right` — Entrada desde la izquierda
- `slide-up` — Entrada desde abajo
- `slide-down` — Entrada desde arriba
- `none` — Corte instantáneo
- Duración: 300ms con easing smoothstep
- Configurables por slide via `<!-- transition: tipo -->`

### Temas visuales (11 integrados)
| Tema | Descripción |
| :--- | :--- |
| `dark` | Fondo oscuro, texto blanco, acentos neutros |
| `rose` | Vino oscuro, acentos rosados |
| `monokai` | Vibrante, títulos rosados, viñetas verdes |
| `nord` | Azul-gris ártico, azules y verdes suaves |
| `light` | Fondo blanco, texto oscuro, acentos azules |
| `blue` | Pizarra azul profunda, acentos cielo |
| `ambercat` | Ámbar/marrón técnico |
| `dracula` | Púrpura/rosa/cian vibrante |
| `gruvbox` | Tono tierra retro |
| `catppuccin` | Pastel suave (mocha) |
| `tokyo-night` | Ciberpunk azul/rosa/cian |

### Frontmatter (metadatos YAML)
Configuración incrustada al inicio del archivo:
```markdown
---
theme: dracula
font: JetBrains Mono
scale: 1.2
---
```

### Notas del presentador
- `<!-- notes: texto -->` en una línea
- `<!-- notes:` ... `-->` multilinea
- Se imprimen automáticamente al navegar

### Hot reload
- Detecta cambios en el archivo `.md` cada 500ms
- Recarga automática preservando tema, fuente y escala
- Notificación en consola

### Exportación
- `--export png` — Exporta diapositivas a PNG
- `--export pdf` — Exporta todas a PDF
- `--export-res WxH` — Resolución personalizada (defecto: 1080x1080)
- `--slide N` — Seleccionar diapositiva específica

### Backends de ventana
- **Win32** (Windows nativo) — GDI double-buffer, timer 60fps
- **X11** (Linux/BSD) — Doble buffer, framerate adaptativo
- Teclado, ratón, pantalla completa en ambos

## Requisitos

### Linux
```bash
sudo apt install libcairo2-dev libpango1.0-dev libx11-dev
```

### Windows (MSYS2 MinGW64)
Instala las dependencias desde la terminal MSYS2 (no PowerShell):
```
pacman -S mingw-w64-x86_64-cairo mingw-w64-x86_64-pango \
          mingw-w64-x86_64-gcc make
```

## Compilar

```bash
make
```

Esto produce:
- `slides` / `slides.exe` — Presentador interactivo
- `libslider.so` / `slider.dll` — Librería compartida (para ports FFI)
- `slides_ada` / `slides_ada.exe` — Versión Ada

En Windows, el Makefile auto-detecta MSYS2 (`C:/msys64/mingw64`, `C:/msys2/mingw64`).
Si usas Cygwin GCC, el Makefile muestra una advertencia.

## Uso

```bash
./slides [opciones] presentacion.md
```

### Opciones

| Flag | Descripción |
| :--- | :--- |
| `-p, --palette <nombre>` | Elegir tema (dark, rose, monokai, nord, light, ...) |
| `-f, --font-family <str>` | Tipografía (ej. 'Arial', 'JetBrains Mono') |
| `-s, --font-scale <num>` | Factor de escala (ej. 1.2) |
| `-e, --export <tipo>` | Exportar a 'pdf' o 'png' |
| `-er, --export-res <WxH>` | Resolución de exportación (ej. 1920x1080) |
| `-sl, --slide <num>` | Slide específico para exportar (0-index) |
| `-v, --version` | Versión |
| `-h, --help` | Ayuda |

Los argumentos CLI sobrescriben la configuración de frontmatter.

### Controles

| Tecla | Acción |
| :--- | :--- |
| `→` `Enter` `Space` `PageDown` | Siguiente diapositiva |
| `←` `Backspace` `PageUp` | Diapositiva anterior |
| `Home` | Primera diapositiva |
| `End` | Última diapositiva |
| `F` `F11` | Pantalla completa (toggle) |
| `Tab` | Vista general (mini-mapa de todas las slides) |
| `→` `←` `↑` `↓` `Enter` | Navegar y seleccionar en vista general |
| `Esc` | Salir / Cerrar vista general |
| `Q` | Salir |

Ratón: clic izquierdo/rueda-arriba = anterior, clic derecho/rueda-abajo = siguiente.

## Ejemplos

```bash
./slides examples/features.md
./slides examples/transitions.md
./slides --palette dracula examples/frontmatter.md
./slides --export pdf --export-res 1920x1080 presentacion.md
```

## Arquitectura

```
slider.h              API pública
src/
  core/
    parser.c          Parser Markdown + frontmatter + transiciones
    themes.c          11 temas visuales
    highlighter.c     Resaltado de sintaxis genérico
    internal.h        Estructuras internas (Slide, Slider, Theme)
  render/
    renderer.c        Motor Cairo/Pango + transiciones + GIF
  ui/
    backend_win32.c   Ventana Win32 (nativa)
    backend_x11.c     Ventana X11
    main.c            CLI + punto de entrada
```

## Tests

```bash
make test
```

Ejecuta 21 tests unitarios y de integración (Python + ctypes sobre `slider.dll`).

## Ports

El proyecto incluye bindings FFI funcionales en varios lenguajes:

- **Ada** (`ada/`) — System.Address FFI, CLI completo
- **Dart** (`dart/`) — dart:ffi + package:ffi
- **Python** (`python/`) — ctypes + tests
- **Zig** (`zig/`) — @cImport, Zig build system
- **Lua** (`lua/`) — LuaJIT FFI
- **Go** (`go/`) — cgo + flag package
