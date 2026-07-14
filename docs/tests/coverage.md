# Cobertura de código (Test Coverage)

## Ejecutar coverage

```bash
make coverage
```

Este comando funciona tanto en Linux como en Windows (MSYS2 MinGW64). Genera:
- En Windows: `slider_coverage.dll`
- En Linux: `libslider_coverage.so`

El proceso es:
1. Limpia artefactos de coverage anteriores
2. Compila la biblioteca compartida con `-fprofile-arcs -ftest-coverage` (GCC instrumentation)
3. Ejecuta la suite completa de 108 tests (Python + ctypes)
4. Ejecuta `gcov` sobre cada archivo fuente del core
5. Muestra un resumen de cobertura en la terminal

## Herramientas utilizadas

- **gcov** — herramienta de coverage incluida con GCC. Analiza los archivos `.gcda`
  generados por el código instrumentado y produce reportes `.gcov` con conteo de ejecución
  por línea.
- **No se requieren dependencias externas** — `gcov` viene con GCC/MinGW.

## Resultados de coverage (Julio 2026)

### Archivos del proyecto C-Slides

| Archivo | Cobertura | Líneas ejecutadas |
|:---|:---:|:---|
| `src/core/parser.c` | **87.5%** | 391 / 447 |
| `src/core/themes.c` | **100.0%** | 8 / 8 |
| `src/core/highlighter.c` | **66.7%** | 6 / 9 |
| `src/core/lexer_c.c` | **42.4%** | 155 / 366 |
| `src/render/renderer.c` | **41.1%** | 101 / 246 |
| `src/render/render_util.c` | **41.5%** | 54 / 130 |
| `src/render/render_table.c` | **100.0%** | 75 / 75 |
| `src/render/render_code.c` | **97.2%** | 35 / 36 |
| `src/render/render_transition.c` | **0.0%** | 0 / 79 |
| `src/render/export_png.c` | **100.0%** | 14 / 14 |
| `src/render/export_jpg.c` | **100.0%** | 26 / 26 |
| `src/render/export_pdf.c` | **94.4%** | 17 / 18 |
| `src/render/export_gif.c` | **73.0%** | 65 / 89 |
| `src/render/export_svg.c` | **93.8%** | 15 / 16 |
| `src/ui/backend_win32.c` | **0.0%** | 0 / 297 |

### Resumen por módulo

| Módulo | Cobertura |
|:---|:---:|
| **Core** (parser + themes + highlighter) | **84.5%** |
| **Render** (renderer + exports + tables + code) | **54.3%** |
| **Lexer C** | **42.4%** |
| **Lexer Python** | **0.0%** |
| **Backend Win32** | **0.0%** |
| **Total proyecto** (sin stb/msf_gif) | **~48%** |

### Detalles por archivo

#### `src/core/parser.c` — 87.5%

Muy bien cubierto. Las líneas sin ejecutar corresponden mayormente a:
- Ramas defensivas de NULL checks que no se alcanzan en tests
- Parsing de notas del presentador (multilinea)
- Algunas ramas del parsing de frontmatter (colores anidados)

#### `src/core/themes.c` — 100%

Cobertura completa. `theme_default()` y `theme_find()` están testeados con los 11 temas,
nombres inválidos y NULL.

#### `src/core/highlighter.c` — 66.7%

El resaltador genérico tiene cobertura parcial. Las líneas no cubiertas son la detección
de Python (`#` comment) que pasa por un camino diferente.

#### `src/render/render_table.c` — 100%

Cobertura completa vía tests de exportación de slides con tablas.

#### `src/render/render_code.c` — 97.2%

Casi completa. Solo falta una línea de manejo de error.

#### `src/render/export_png.c` — 100%

Cobertura completa.

#### `src/render/export_jpg.c` — 100%

Cobertura completa.

#### `src/render/export_pdf.c` — 94.4%

Casi completa. Solo falta una rama de error.

#### `src/render/export_gif.c` — 73.0%

Cobertura moderada. Las líneas no cubiertas corresponden a la generación de frames
intermedios y manejo de GIFs animados.

#### `src/render/export_svg.c` — 93.8%

Casi completa. Solo falta una rama de error.

#### `src/render/render_transition.c` — 0%

Las transiciones no se testean porque requieren un contexto de renderizado con tiempo
(timeline de animación). Solo se ejecutan en el backend gráfico interactivo.

#### `src/ui/backend_win32.c` — 0%

El backend de ventana no se testea porque requiere una ventana gráfica real (Win32 API).
Solo funciona en modo interactivo.

## Archivos no incluidos en el conteo

Los siguientes archivos son bibliotecas de terceros incluidas en el proyecto:
- `src/stb_image.h` — carga de imágenes (STB)
- `src/stb_image_write.h` — escritura de imágenes (STB)
- `src/msf_gif.h` — codificación GIF (MSF)

Su coverage no es representativo ya que se incluyen como headers completos.

## Funciones NO cubiertas por los tests automatizados

| Función | Razón |
|:---|:---|
| `backend_run()` | Requiere ventana gráfica (X11/Win32). Solo ejecutable en modo interactivo. |
| `slider_render()` directamente | Solo se invoca internamente por exportaciones con contexto Cairo. |
| `main()` / `help.c` | CLI entry point, no testeable vía la biblioteca. |

## Generar reporte HTML (opcional)

Si se instala `lcov`:

```bash
make coverage
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

## Verificar cobertura manualmente

Para inspeccionar una fuente específica:

```bash
# Compilar con coverage
make clean
make coverage

# Ver cobertura de parser.c
gcov slider_coverage.dll-parser.gcno

# Abrir parser.c.gcov y buscar líneas con ##### (no ejecutadas)
```
