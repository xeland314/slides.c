# Tests

Suite de tests para C-Slides. Tests unitarios, de integración y de exportación escritos
en Python, ejecutados vía `ctypes` sobre la biblioteca compartida (`slider.dll` / `libslider.so`).

## Ejecución rápida

```bash
make test
```

Esto compila la biblioteca compartida (si es necesario) y ejecuta todos los tests:

```bash
python3 ports/python/run_all_tests.py
```

### Requisitos

- Python 3.6+
- La biblioteca compartida compilada (`slider.dll` en Windows, `libslider.so` en Linux)
- Ninguna dependencia externa de Python (solo `unittest` y `ctypes` de la stdlib)

## Estructura

```
ports/python/
  c_slides_bindings.py   Bindings FFI (ctypes) de la API pública de C-Slides
  run_all_tests.py       Descubridor y ejecutor de la suite de tests
  test_parser.py         Tests del parser de líneas Markdown
  test_markup.py         Tests del conversor Markdown → markup Pango (bold/italic/code)
  test_highlighter.py    Tests del resaltador de sintaxis genérico
  test_themes.py         Tests del sistema de temas y color overrides
  test_export.py         Tests de exportación a PNG/JPG/SVG/PDF/GIF
  test_integration.py    Tests de integración: carga de archivos, frontmatter, propiedades
  test_visual.py         Tests visuales: exportación y renderizado con distintos temas
```

## Archivos de test detallados

### test_parser.py — Parser de líneas Markdown (26 tests)

Tests unitarios para `parse_line()`, la función que clasifica una línea individual de Markdown.

| Test | Descripción |
|:---|:---|
| `test_parse_title` | Línea `# Titulo` → `LINE_TITLE` |
| `test_parse_subtitle` | Línea `## Subtitulo` → `LINE_SUBTITLE` |
| `test_parse_blockquote` | Línea `> Texto` → `LINE_BLOCKQUOTE` |
| `test_parse_bullets` | Viñetas nivel 1 (`- item`) y nivel 2 (`  - item`) |
| `test_parse_image` | Formato corto `!archivo.png` y Markdown `![alt](path)` |
| `test_parse_table` | Fila de tabla `| A | B |` y separador `|---|---|` |
| `test_parse_task_list` | `- [ ]`, `- [x]`, `- [X]` → task unchecked/checked |
| `test_parse_empty_line` | Línea vacía → `LINE_EMPTY` |
| `test_parse_whitespace_only` | Espacios/tabs → `LINE_EMPTY` |
| `test_parse_body_text` | Texto plano → `LINE_BODY` |
| `test_parse_body_long_text` | Párrafo largo (500+ chars) |
| `test_parse_num_list_dot` | `1. Elemento` → `LINE_NUM_LIST`, marker=`1.` |
| `test_parse_num_list_letter` | `a) Elemento` → `LINE_NUM_LIST`, marker=`a)` |
| `test_parse_num_list_roman` | `i. Numero` → `LINE_NUM_LIST`, marker=`i.` |
| `test_parse_num_list_uppercase` | `A) Mayuscula` → `LINE_NUM_LIST`, marker=`A)` |
| `test_parse_num_list_multi_digit` | `12. Grande` → marker=`12.` |
| `test_parse_code_fence_start` | `` ```c `` → `LINE_BODY` (parse_line es de una sola línea; los bloques de código se manejan en `slider_load`) |
| `test_parse_code_fence_end` | ````` `` → `LINE_BODY` (idem) |
| `test_parse_table_multi_column` | Tabla de 4 columnas |
| `test_parse_table_with_alignment` | `|:---|---:|` es separador, no fila |
| `test_parse_table_separator_variants` | Variantes de formato de separador |
| `test_parse_bullet_with_special_chars` | Viñeta con markdown inline |
| `test_parse_image_markdown_format` | `![desc](ruta)` extrae solo la ruta |
| `test_parse_blockquote_long` | Blockquote largo |
| `test_parse_title_with_special_chars` | Titulo con `&`, `<`, unicode |
| `test_parse_subtitle_deep` | Subtítulo con texto largo |

### test_markup.py — Conversor Markdown → Pango (7 tests)

Tests para `md_to_markup()`, que convierte markdown inline a tags Pango.

| Test | Descripción |
|:---|:---|
| `test_simple_bold` | `**bold**` → `<b>bold</b>` |
| `test_simple_italic` | `*italic*` → `<i>italic</i>` |
| `test_combined_bold_italic` | `***ambos***` → `<b><i>ambos</i></b>` |
| `test_nested_complex` | Negrita con itálica anidada |
| `test_escaping` | `&`, `<`, `>` → `&amp;`, `&lt;`, `&gt;` |
| `test_inline_code` | `` `code` `` → `<tt>code</tt>` |
| `test_fuzzing_markup` | 10,000 iteraciones con tokens aleatorios, verificando que solo se generan tags válidos |

### test_highlighter.py — Resaltador de sintaxis (21 tests)

Tests para `highlighter_highlight()`, el resaltador genérico multi-lenguaje.

| Test | Descripción |
|:---|:---|
| `test_highlighter_basic` | Keywords, comentarios, símbolos con `<span foreground=...>` |
| `test_highlighter_escaping` | `<`, `>`, `→ `&lt;`, `&gt;`, `&amp;` |
| `test_highlighter_string` | Cadenas entre comillas dobles |
| `test_highlighter_keywords_c` | 18 keywords C: `if`, `else`, `for`, `while`, `return`, `int`, etc. |
| `test_highlighter_keywords_python` | 26 keywords Python: `def`, `class`, `True`, `False`, `lambda`, etc. |
| `test_highlighter_single_line_comment` | Comentario `// ...` |
| `test_highlighter_python_comment` | Comentario `# ...` |
| `test_highlighter_strings_double` | `"string"` resaltado |
| `test_highlighter_strings_single` | `'string'` resaltado |
| `test_highlighter_numbers_integer` | Entero `42` |
| `test_highlighter_numbers_float` | Float `3.14159` |
| `test_highlighter_numbers_hex` | Hexadecimal `0xFF` |
| `test_highlighter_numbers_binary` | Binario `0b1010` |
| `test_highlighter_numbers_scientific` | Notación científica `6.022e23` |
| `test_highlighter_operators` | 24 operadores: `+`, `-`, `==`, `&&`, `>>=`, etc. |
| `test_highlighter_brackets` | Paréntesis, corchetes, llaves |
| `test_highlighter_empty_string` | Input vacío → output vacío |
| `test_highlighter_pure_text_no_highlight` | Texto sin tokens resaltados |
| `test_highlighter_mixed_content` | Línea con keywords + números + strings |
| `test_highlighter_multiline_simulation` | 5 líneas de código C, una por una |
| `test_highlighter_different_themes` | Mismo código con 4 temas distintos |

### test_themes.py — Sistema de temas y color overrides (16 tests)

Tests para `theme_default()`, `theme_find()`, `slider_set_theme()` y `slider_set_color()`.

| Test | Descripción |
|:---|:---|
| `test_theme_default_exists` | `theme_default()` devuelve "dark" |
| `test_theme_default_colors_are_nonzero` | Colores del tema default son distintos de 0 |
| `test_all_themes_found_by_name` | Los 11 temas se encuentran por nombre |
| `test_unknown_theme_falls_back_to_dark` | Nombre inválido → fallback a "dark" |
| `test_null_theme_name_falls_back_to_dark` | `NULL` → fallback a "dark" |
| `test_theme_colors_are_distinct_across_themes` | Los 11 temas tienen bg distintos |
| `test_set_all_themes_on_slider` | Se puede aplicar cada tema a un slider |
| `test_set_theme_invalid_name_keeps_previous` | Tema inválido reemplaza con "dark" |
| `test_theme_struct_fields_are_uint32` | Los 17 campos de color son `uint32` |
| `test_set_color_bg` | `set_color("bg", "#ff0000")` no crashea, export funciona |
| `test_set_color_all_keys` | Las 17 claves de color se pueden establecer |
| `test_set_color_alias_subtitle` | `"subtitle"` es alias de `"sub"` |
| `test_set_color_alias_text` | `"text"` es alias de `"body"` |
| `test_set_color_alias_number` | `"number"` es alias de `"num"` |
| `test_set_color_accent_also_sets_bullet` | `"accent"` también modifica `"bullet"` |
| `test_set_color_invalid_hex_ignored` | Hex inválido no modifica el color |

### test_export.py — Exportación multi-formato (11 tests)

Tests de `slider_export_png()`, `slider_export_jpg()`, `slider_export_svg()`,
`slider_export_pdf()` y `slider_export_gif()`.

| Test | Descripción |
|:---|:---|
| `test_export_png_basic` | PNG exporta, archivo > 1KB |
| `test_export_png_different_resolutions` | 1920×1080, 640×480, 3840×2160 |
| `test_export_jpg_basic` | JPG exporta con calidad 90 |
| `test_export_jpg_low_quality` | JPG calidad 10 < calidad 100 en tamaño |
| `test_export_svg_basic` | SVG exporta, contiene tag `<svg>` |
| `test_export_pdf_basic` | PDF exporta, cabecera `%PDF-` |
| `test_export_gif_basic` | GIF exporta, cabecera `GIF` |
| `test_export_png_second_slide` | Exporta slide índice 1 (no solo la primera) |
| `test_export_with_different_themes` | Exporta PNG con 3 temas distintos |
| `test_export_svg_all_slides` | Exporta todas las slides de basic.md a SVG |
| `test_export_formats_with_all_themes` | Matriz 5 temas × 3 formatos (PNG/JPG/SVG) |

### test_integration.py — Tests de integración (25 tests)

Tests que verifican el comportamiento completo de carga → configuración → exportación.

| Test | Descripción |
|:---|:---|
| `test_slider_load_basic` | Carga basic.md: 2 slides, tema "dark" |
| `test_slider_change_palette` | Cambia tema a "rose" |
| `test_slider_load_nonexistent_file` | Archivo inexistente → NULL |
| `test_slider_get_count_empty_file` | basic.md tiene > 0 slides |
| `test_slider_load_frontmatter` | frontmatter.md: tema "dracula", font "JetBrains Mono" |
| `test_slider_load_features` | features.md tiene ≥ 4 slides |
| `test_slider_load_code_blocks` | code_blocks.md tiene ≥ 4 slides |
| `test_slider_load_complex_tables` | complex_tables.md tiene ≥ 2 slides |
| `test_slider_load_transitions` | transitions.md tiene ≥ 6 slides |
| `test_slider_load_custom_colors` | custom_colors.md tiene ≥ 5 slides |
| `test_slider_load_img_config` | img_config.md tiene ≥ 10 slides |
| `test_slider_load_test_gif` | test_gif.md tiene ≥ 3 slides |
| `test_font_family_default` | Font por defecto es "Inter" |
| `test_font_family_set_get` | set/get font family roundtrip |
| `test_font_scale_default` | Scale por defecto es 1.0 |
| `test_font_scale_set_get` | set/get font scale roundtrip |
| `test_font_scale_minimum_threshold` | Scale ≤ 0.1 es ignorado |
| `test_set_color_on_slider` | set_color + export sin crash |
| `test_set_color_does_not_affect_original_theme` | Dos sliders independientes |
| `test_change_theme_resets_colors` | Cambiar tema resetea colores previos |
| `test_multiple_load_free_cycles` | 10 ciclos load/free sin memory leak |
| `test_print_notes_does_not_crash` | `slider_print_notes` no crashea |
| `test_slider_default_properties` | Props default: dark, Inter, 1.0 |
| `test_frontmatter_with_custom_colors` | custom_colors.md: colors block aplicado |
| `test_export_after_theme_change` | Export funciona tras cambiar tema |

### test_visual.py — Tests visuales (2 tests)

Tests de exportación visual (pre-existente).

| Test | Descripción |
|:---|:---|
| `test_export_png` | Exporta basic.md a PNG, verifica tamaño |
| `test_export_all_themes` | Exporta con los 11 temas (stress test) |

## Resumen

| Archivo | Tests | Función C cubierta |
|:---|:---:|:---|
| `test_parser.py` | 26 | `parse_line()` |
| `test_markup.py` | 7 | `md_to_markup()` |
| `test_highlighter.py` | 21 | `highlighter_highlight()` |
| `test_themes.py` | 16 | `theme_default()`, `theme_find()`, `slider_set_theme()`, `slider_set_color()` |
| `test_export.py` | 11 | `slider_export_png/jpg/svg/pdf/gif()` |
| `test_integration.py` | 25 | `slider_load()`, `slider_free()`, `slider_get_count()`, getters/setters, frontmatter |
| `test_visual.py` | 2 | `slider_export_png()`, `slider_set_theme()` |
| **Total** | **108** | |

## Cobertura de código (Coverage)

Para generar un reporte de cobertura de código C:

```bash
make coverage
```

Esto compila con `--coverage`, ejecuta los tests, y genera un reporte en `coverage/`.
Ver [coverage.md](coverage.md) para más detalles.

## Notas técnicas

### Port Python (ctypes)

Los tests usan `ports/python/c_slides_bindings.py` para cargar la biblioteca compartida
y llamar a las funciones C vía `ctypes`. Esto permite testear la lógica de negocio
sin necesidad de compilar un ejecutable separado para tests.

### Opaque struct

El tipo `Slider` es una estructura opaca en C. Los tests no pueden acceder directamente
a sus campos internos (como `theme_storage`). Para verificar que `slider_set_color()`
funciona correctamente, se valida a través de exportación: si la exportación produce
un archivo válido, los colores se aplicaron correctamente.

### Linux vs Windows

Los paths de la biblioteca se resuelven automáticamente:
- Linux: `libslider.so`
- Windows: `slider.dll`

El binding detecta la plataforma y ajusta la extensión y prefijo del archivo de biblioteca.
