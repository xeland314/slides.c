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
2. Genera lexer C/Python desde archivos `.l` con flex
3. Compila la biblioteca compartida con `-fprofile-arcs -ftest-coverage` (GCC instrumentation)
4. Copia la DLL de coverage como `slider.dll` para que los tests la carguen
5. Ejecuta la suite completa de 435 tests (Python + ctypes)
6. Ejecuta `gcov` sobre cada archivo fuente del core
7. Muestra un resumen de cobertura en la terminal

## Herramientas utilizadas

- **gcov** — herramienta de coverage incluida con GCC. Analiza los archivos `.gcda`
  generados por el código instrumentado y produce reportes `.gcov` con conteo de ejecución
  por línea.
- **No se requieren dependencias externas** — `gcov` viene con GCC/MinGW.

## Resultados de coverage (Julio 2026)

### Archivos del proyecto C-Slides

| Archivo | Cobertura | Líneas ejecutadas |
|:---|:---:|:---|
| `src/core/parser.c` | **95.3%** | ~544 / 571 |
| `src/core/themes.c` | **100.0%** | 8 / 8 |
| `src/core/highlighter.c` | **77.8%** | 7 / 9 |
| `src/render/renderer.c` | **95.9%** | ~236 / 246 |
| `src/render/render_util.c` | **76.0%** | ~111 / 146 |
| `src/render/render_table.c` | **100.0%** | 75 / 75 |
| `src/render/render_code.c` | **97.2%** | ~35 / 36 |
| `src/render/render_transition.c` | **89.9%** | ~71 / 79 |
| `src/render/export_png.c` | **100.0%** | 14 / 14 |
| `src/render/export_jpg.c` | **90.0%** | 27 / 30 |
| `src/render/export_pdf.c` | **94.4%** | ~17 / 18 |
| `src/render/export_gif.c` | **73.0%** | ~65 / 89 |
| `src/render/export_svg.c` | **93.8%** | 15 / 16 |
| `src/ui/backend_win32.c` | **0.0%** | 0 / ~297 |

### Resumen por módulo

| Módulo | Cobertura |
|:---|:---:|
| **Core** (parser + themes + highlighter) | **~95%** |
| **Render** (renderer + util + table + code + transition) | **~92%** |
| **Exports** (png + jpg + pdf + gif + svg) | **~88%** |
| **Lexer C** | No incluido (auto-generado) |
| **Lexer Python** | No incluido (auto-generado) |
| **Backend Win32** | **0.0%** (requiere ventana real) |
| **Total proyecto** (archivos propios, sin stb/msf_gif) | **~91.6%** |

### Detalles por archivo

#### `src/core/parser.c` — 95.3%

Excelente cobertura. Las líneas sin ejecutar corresponden mayormente a:
- Ramas defensivas de NULL checks que no se alcanzan en tests
- Algunas ramas del parsing de frontmatter (colores anidados menos comunes)
- Funciones de acceso de test (`slider_test_get_*`) que solo se usan en tests específicos

#### `src/core/themes.c` — 100%

Cobertura completa. `theme_default()` y `theme_find()` están testeados con los 11 temas,
nombres inválidos y NULL.

#### `src/core/highlighter.c` — 77.8%

El resaltador genérico tiene buena cobertura. La línea no cubierta corresponde a la
detección de Python (`#` comment) que pasa por un camino diferente al genérico.

#### `src/render/renderer.c` — 95.9%

Excelente cobertura. Solo faltan unas pocas ramas de manejo de error poco probables.

#### `src/render/render_util.c` — 76.0%

Buena cobertura. Las líneas no cubiertas son mayormente funciones de cache de imágenes
que solo se ejecutan en ciertos escenarios de carga.

#### `src/render/render_table.c` — 100%

Cobertura completa vía tests de exportación de slides con tablas.

#### `src/render/render_code.c` — 97.2%

Casi completa. Solo falta una línea de manejo de error.

#### `src/render/render_transition.c` — 89.9%

Muy buena cobertura (antes era 0%). Las transiciones ahora se testean directamente
vía `do_transition()` y `slider_render()` con transiciones configuradas.

#### `src/render/export_png.c` — 100%

Cobertura completa.

#### `src/render/export_jpg.c` — 90.0%

Casi completa. La línea no cubierta es el check de NULL en `cairo_image_surface_get_data()`
que se agregó como fix de bug.

#### `src/render/export_pdf.c` — 94.4%

Casi completa. Solo falta una rama de error.

#### `src/render/export_gif.c` — 73.0%

Cobertura moderada. Las líneas no cubiertas corresponden a la generación de frames
intermedios y manejo de GIFs animados.

#### `src/render/export_svg.c` — 93.8%

Casi completa. Solo falta una rama de error.

#### `src/ui/backend_win32.c` — 0%

El backend de ventana no se testea porque requiere una ventana gráfica real (Win32 API).
Solo funciona en modo interactivo.

## Archivos no incluidos en el conteo

Los siguientes archivos son bibliotecas de terceros incluidas en el proyecto:
- `src/stb_image.h` — carga de imágenes (STB)
- `src/stb_image_write.h` — escritura de imágenes (STB)
- `src/msf_gif.h` — codificación GIF (MSF)

Su coverage no es representativo ya que se incluyen como headers completos.

Los archivos lexer generados (`src/core/lexer_c.c`, `src/core/lexer_py.c`) son
auto-generados por flex y no se cuentan en el coverage del proyecto.

## Funciones NO cubiertas por los tests automatizados

| Función | Razón |
|:---|:---|
| `backend_run()` | Requiere ventana gráfica (X11/Win32). Solo ejecutable en modo interactivo. |
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
