# Roadmap

## v0.2.x — Mejoras incrementales (PRs pequeños)

| # | Rama | Descripción |
|---|------|-------------|
| 1 | `feat/export-svg` | Exportar slides a SVG con `--export svg` |
| 2 | `feat/export-jpg` | Exportar slides a JPEG con `--export jpg` |
| 3 | `feat/export-gif` | Exportar presentación animada a GIF |
| 4 | `feat/hide-progress` | Permitir desactivar la barra de progreso |
| 5 | `feat/transition-ms` | Duración de transición configurable desde frontmatter/CLI |
| 6 | `feat/user-themes` | Temas personalizables desde frontmatter |

## v0.3.x — Lenzuelaje

## v0.4.x — Modo kiosk

- Avance automático de slides cada N segundos
- Flag `--kiosk` o `--auto-advance`
- Útil para vitrinas, displays públicos o ferias

## v0.5.x — Soporte Wayland

- Backend nativo para Wayland (además de X11 y Win32)
- Sin dependencia de X11 en entornos Wayland-puro

## v0.6.x - TODO

Aquí tienes un análisis completo del estado del proyecto y las acciones posibles, ordenadas por prioridad:
Bugs críticos
1. Double-free en el backend X11 (src/ui/backend_x11.c:260) — slider_free(s) se llama dentro de backend_run, pero main() también llama slider_free(s) con el mismo puntero después.
2. Use-after-free en hot reload Win32 (src/ui/backend_win32.c:219) — Durante hot-reload, slider_free(g_slider) se ejecuta mientras WM_TIMER podría estar usando ese puntero.
3. Memory leak en la caché de imágenes (src/render/render_util.c) — Las superficies Cairo cacheadas nunca se liberan. Cada slider_free acumula más memoria.
4. export_jpg.c:24-26 — No verifica si cairo_image_surface_get_data retorna NULL antes de leer los datos.
Code quality
5. Funciones semipublicas sin declarar en slider.h — parse_line, md_to_markup, theme_default, theme_find se usan en los tests Python pero no están en el header público. Hay que decidir: o se exportan o se remueven de los bindings.
6. main.c:114-115 — Usa acceso directo a s->font_family en vez de slider_set_font_family().
7. help.c — Flag --accent duplicado en la ayuda.
8. Versión hardcodeada — help.c:39 dice v0.2.2 sin macro VERSION.
9. Falta const correctness en funciones internas como render_table, render_code_block.
Ports incompletos
Los ports de Go, Lua, Zig, Dart y Ada solo bindean ~10 de las ~19+ funciones disponibles. Les faltan: slider_export_jpg/svg/pdf/gif, slider_set_color, slider_print_notes, highlighter_highlight, theme_default, theme_find, parse_line, md_to_markup.
Solo el port de Python está completo.
Testing gaps
10. render_transition.c al 0% — Ninguna transición (fade, slide-left/right/up/down) tiene tests.
11. lexer_py.c al 0% — El lexer de Python sin cobertura.
12. renderer.c al 41% — Falta cubrir rendering de bullets nivel 2, task lists, num lists, imágenes con ImageConfig (fit, opacity, rotation, radius, alignment).
13. Sin tests para parse_img_config — fit, width, height, opacity, radius, rotate, align no están testeados.
14. Sin tests para notes parsing — <!-- notes: ... --> multilinea sin tests unitarios.
15. Sin tests para hot reload, kiosk mode, o límites del parser (MAX_SLIDES, MAX_LINE_LEN, MAX_COLS).
Documentación
16. README.md:271 dice "21 tests" cuando hay 108.
17. PLAN.md lista features ya implementadas (export-svg, export-jpg) como pendientes.
18. Ports muestran solo 5 temas en su help text (faltan 6).
19. slides.1 (man page) referenciado en Makefile pero puede no existir.
Performance
20. 7 PangoLayout creados/destruidos por frame (renderer.c:44-50) — Podrían cachearse.
21. render_pango re-parsea markup en cada llamada — 4KB de stack por línea de texto por frame.
22. Caché de imágenes fija a 64 slots sin LRU eviction.
Funcionalidades faltantes
23. Links Markdown [text](url) — No soportados.
24. Highlighter solo C/Python — Falta JS, Rust, Go, Java, Shell, etc.
25. Duración de transición configurable — Mencionada en ideas.md pero TRANSITION_DEFAULT_MS sigue hardcodeada a 300ms.
26. No hay Wayland backend — Listed en PLAN.md como futuro.
27. ### y niveles superiores caen a BODY sin warning.
28. No hay horizontal rule (---, ***, ___) como separador visual dentro de una slide.

## v0.7.x - Layouts disenados: cover y end (mini motor de plantillas)

Objetivo: replicar la identidad visual de Cover Creator (herramienta web del portfolio
xeland314.github.io) sin clonar sus 21 templates. Solo 2-3 layouts especiales; todo lo
demas sigue siendo markdown puro. Aprovechar el mecanismo existente de directivas en
comentarios HTML (<!-- transition -->, <!-- notes -->, <!-- img: -->).

### Directiva propuesta

    <!-- slide-type: cover -->
    # Titulo grande de la portada
    ## Subtitulo
    categoria: DEEP DIVE
    icono: (emoji o ruta a PNG)

    <!-- slide-type: end -->
    # Thanks for / Reading!
    descripcion: Si te ha gustado este contenido, sigueme para mas.
    handle: @xeland314

Reglas:
- La directiva debe ser la PRIMERA linea de la slide (el parser ya divide por ---).
- Keys desconocidas se ignoran con warning en stderr (no fallar).
- Sin directiva => layout markdown normal (comportamiento actual intacto).
- Opcional fase 2: <!-- slide-type: quote --> con autor.

### Parser (src/core/parser.c)

1. Detectar <!-- slide-type: X --> al inicio de slide; guardar slide->layout (enum
   LAYOUT_MARKDOWN | LAYOUT_COVER | LAYOUT_END) en struct Slide (internal.h).
2. Parsear las keys siguientes como pares clave:valor de linea simple:
   - cover: titulo (de # ), subtitulo (## ), categoria (categoria:), icono (icono:)
   - end: dos lineas del #  separadas por / o campos first/second, descripcion,
     handle.
3. Tests: slide con directiva produce layout correcto; keys invalidas no crashean;
   slides normales siguen igual (los 668 tests actuales deben pasar).

### Renderer (nuevo src/render/render_layout.c + hook en renderer.c)

Composicion Cairo/Pango del layout COVER sobre el canvas estandar (1080x1350 base,
respetar --export-res):
- Fondo: color theme.bg + 3 orbes radiales translucidos del color accent (cairo
  radial gradient, blur simulado con varios circulos concentricos alpha decreciente).
- Grid sutil: lineas verticales/horizontales cada 48px, stroke rgba blanco 7%.
- Badge de categoria arriba-centro: rounded rect con borde accent, texto uppercase
  tracking amplio (Pango letter-spacing via markup span letter_spacing).
- Titulo centrado, font ~72px * font_scale, bold; subtitulo debajo ~32px en theme.sub.
- Circulo con icono (emoji via Pango markup o imagen cacheada si icono: es ruta).
- Footer: handle en mono + logo.png (ya existe en raiz) abajo.
Composicion END similar: dos lineas gigantes, descripcion, fila de 3 botones sociales
(rects redondeados con textos Like/Comment/Save), footer.

### Temas compartidos con Cover Creator

Cover Creator define los mismos temas en TypeScript
(xeland314.github.io/src/components/tools/covers/types.ts -> THEME_STYLES +
THEME_BG_COLORS: dark, rose, blue, ambercat, catppuccin, obsidian, midnight...).
Plan de sincronizacion:
1. Extraer los hex a un JSON unico: tools/themes.json (bg, text, sub, footer, badge,
   accent, code-bg...) por tema, versionado en el portfolio.
2. Escribir scripts/gen_themes.py que lea themes.json y EMITA src/core/themes_data.c
   (array C de structs Theme). Anadir target make themes al Makefile.
3. Mapear los nombres de paleta ya existentes en slides.c a los nuevos valores para
   no romper los tests actuales; los temas nuevos (obsidian, midnight) se agregan.
4. Los ports (Python ctypes) heredan gratis porque usan slider.h.

### Criterios de aceptacion

- ./slides --palette rose demo-cover.md renderiza portada con badge/titulo/icono/footer.
- Export png/pdf/svg/gif respetan los layouts nuevos (misma ruta de render).
- Slides markdown puras quedan pixel-identicas a v0.6.x (regression).
- make test verde (668 tests + los nuevos de parser/layout).
- make themes regenera themes_data.c desde themes.json sin diffs manuales.
