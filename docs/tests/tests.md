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
- `libcairo-2.dll` / `libcairo.so` (solo para tests de transiciones e img_config render)

## Estructura

```
ports/python/
  c_slides_bindings.py   Bindings FFI (ctypes) de la API pública de C-Slides
  cairo_helpers.py       Helper ctypes para operaciones Cairo (superficies, contextos)
  run_all_tests.py       Descubridor y ejecutor de la suite de tests
  test_parser.py         Tests del parser de líneas Markdown
  test_markup.py         Tests del conversor Markdown → markup Pango (bold/italic/code)
  test_highlighter.py    Tests del resaltador de sintaxis genérico
   test_lexer_py.py       Tests del lexer de Python (flex lexer)
   test_lexer_go.py       Tests del lexer de Go (flex lexer)
   test_lexer_js.py       Tests del lexer de JavaScript (flex lexer)
   test_lexer_ts.py       Tests del lexer de TypeScript (flex lexer)
   test_lexer_tsx.py      Tests del lexer de TSX con JSX (flex lexer)
   test_themes.py         Tests del sistema de temas y color overrides
  test_export.py         Tests de exportación a PNG/JPG/SVG/PDF/GIF
  test_transitions.py    Tests de transiciones entre slides (render_transition.c)
  test_renderer_gaps.py  Tests de renderer, img_config, notes, parser limits, hot reload
  test_empty_warnings.py Tests de warnings para archivos vacíos/blank
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

### test_lexer_py.py — Lexer de Python (46 tests)

Tests para `py_lexer_run()`, el lexer flex para código Python.

| Test | Descripción |
|:---|:---|
| `test_keyword_def` | `def` resaltado |
| `test_keyword_class` | `class` resaltado |
| `test_keyword_return` | `return` resaltado |
| `test_keyword_if_else` | `if`, `elif`, `else` resaltados |
| `test_keyword_for_while` | `for`, `while` resaltados |
| `test_keyword_import_from` | `import`, `from` resaltados |
| `test_keyword_in_is` | `in`, `is`, `not` resaltados |
| `test_keyword_lambda` | `lambda` resaltado |
| `test_keyword_none_true_false` | `None`, `True`, `False` resaltados |
| `test_keyword_not_and_or` | `not`, `and`, `or` resaltados |
| `test_keyword_pass_break_continue` | `pass`, `break`, `continue` resaltados |
| `test_keyword_raise` | `raise` resaltado |
| `test_keyword_try_except` | `try`, `except` resaltados |
| `test_keyword_with_as` | `with`, `as` resaltados |
| `test_comment_single` | Comentario `# ...` resaltado |
| `test_comment_inline` | Código + comentario en la misma línea |
| `test_string_double_quotes` | `"string"` resaltado |
| `test_string_single_quotes` | `'string'` resaltado |
| `test_string_triple_double` | `"""docstring"""` resaltado |
| `test_string_triple_single` | `'''docstring'''` resaltado |
| `test_string_escaped` | `"with \"escape\""` resaltado |
| `test_number_integer` | Entero `42` resaltado |
| `test_number_float` | Float `3.14` resaltado |
| `test_number_hex` | Hex `0xFF` resaltado |
| `test_number_binary` | Binario `0b1010` resaltado |
| `test_number_octal` | Octal `0o77` resaltado |
| `test_number_scientific` | `6.022e23` resaltado |
| `test_number_complex` | `3j` resaltado |
| `test_number_underscore` | `1_000_000` resaltado |
| `test_operators` | Operadores aritméticos y lógicos |
| `test_comparison_operators` | `==`, `!=`, `<`, `>`, `<=`, `>=` |
| `test_brackets` | Paréntesis, corchetes, llaves |
| `test_identifier` | Identificadores alfanuméricos |
| `test_identifier_underscore_start` | `_private` identificador |
| `test_attribute_access` | `obj.attr` identificador |
| `test_decorator` | `@decorator` resaltado |
| `test_fstring` | `f"format"` resaltado |
| `test_ellipsis` | `...` (Ellipsis) resaltado |
| `test_empty_string` | Input vacío → output vacío |
| `test_negative_number` | `-42` resaltado |
| `test_complex_expression` | Expresión completa con múltiples tokens |
| `test_multiline_simulation` | 5 líneas de Python, una por una |
| `test_power_operator` | `**` operador |
| `test_star_import` | `from module import *` |
| `test_walrus_operator` | `:=` operador |
| `test_via_highlighter_python` | Llamada a `highlight_lang("python")` |

### test_lexer_go.py — Lexer de Go (58 tests)

Tests para `go_lexer_run()`, el lexer flex para código Go, con prefix `go_` para evitar
conflictos de símbolos.

| Test | Descripción |
|:---|:---|
| `test_keyword_func` | `func` resaltado |
| `test_keyword_package` | `package` resaltado |
| `test_keyword_import` | `import` resaltado |
| `test_keyword_var` | `var` resaltado |
| `test_keyword_const` | `const` resaltado |
| `test_keyword_type` | `type` resaltado |
| `test_keyword_struct` | `struct` resaltado |
| `test_keyword_interface` | `interface` resaltado |
| `test_keyword_map` | `map` resaltado |
| `test_keyword_chan` | `chan` resaltado |
| `test_keyword_go` | `go` resaltado |
| `test_keyword_defer` | `defer` resaltado |
| `test_keyword_select` | `select` resaltado |
| `test_keyword_switch` | `switch` resaltado |
| `test_keyword_case` | `case` resaltado |
| `test_keyword_if_else` | `if`, `else` resaltados |
| `test_keyword_for_range` | `for`, `range` resaltados |
| `test_keyword_return` | `return` resaltado |
| `test_keyword_break_continue_fallthrough` | `break`, `continue`, `fallthrough` resaltados |
| `test_keyword_nil_true_false` | `nil`, `true`, `false` resaltados |
| `test_keyword_iota` | `iota` resaltado |
| `test_keyword_make_new_len_cap` | `make`, `new`, `len`, `cap` resaltados |
| `test_keyword_append_copy_delete` | `append`, `copy`, `delete` resaltados |
| `test_keyword_panic_recover` | `panic`, `recover` resaltados |
| `test_keyword_print_println` | `print`, `println` resaltados |
| `test_builtin_types` | 21 tipos: `int`, `int8`..`int64`, `uint*`, `float32/64`, `complex*`, `bool`, `byte`, `rune`, `string`, `error`, `any`, `comparable` |
| `test_comment_single_line` | Comentario `// ...` resaltado |
| `test_comment_inline` | Código + comentario en la misma línea |
| `test_string_double_quotes` | `"string"` resaltado |
| `test_string_escaped` | `"with \"escape\""` resaltado |
| `test_string_rune` | `'a'` resaltado |
| `test_number_integer` | Entero `42` resaltado |
| `test_number_float` | Float `3.14` resaltado |
| `test_number_hex` | Hex `0xFF` resaltado |
| `test_number_binary` | Binario `0b1010` resaltado |
| `test_number_octal` | Octal `0o77` resaltado |
| `test_number_underscore` | `1_000_000` resaltado |
| `test_number_scientific` | `6.022e23` resaltado |
| `test_operator_short_declaration` | `:=` operador |
| `test_operator_receive` | `<-` channel receive |
| `test_operators_arithmetic` | Operadores aritméticos |
| `test_operators_comparison` | `==`, `!=`, `>=`, etc. |
| `test_brackets` | `m[key]` corchetes |
| `test_braces` | `func() {}` llaves |
| `test_identifier` | Identificador no resaltado |
| `test_identifier_underscore_start` | `_private` identificador |
| `test_empty_string` | Input vacío → output vacío |
| `test_multiline_simulation` | 7 líneas de Go, una por una |
| `test_complex_expression` | Expresión con múltiples tokens |
| `test_switch_statement` | `switch` con identificador |
| `test_go_routine` | `go processRequest(req)` |
| `test_channel_operations` | `ch <- data` |
| `test_type_declaration` | `type Server struct {` |
| `test_interface_method` | Método de interfaz con tipos |
| `test_map_literal` | `map[string]int{"a": 1}` |
| `test_defer_close` | `defer f.Close()` |
| `test_error_check_idiom` | `if err != nil {` |
| `test_via_highlight_lang` | Llamada a `highlight_lang("go")` |

### test_lexer_js.py — Lexer de JavaScript (54 tests)

Tests para `js_lexer_run()`, el lexer flex para código JavaScript.

| Test | Descripción |
|:---|:---|
| `test_keyword_var` | `var` resaltado |
| `test_keyword_let` | `let` resaltado |
| `test_keyword_const` | `const` resaltado |
| `test_keyword_function` | `function` resaltado |
| `test_keyword_return` | `return` resaltado |
| `test_keyword_if_else` | `if`, `else` resaltados |
| `test_keyword_for_while_do` | `for`, `while`, `do` resaltados |
| `test_keyword_switch_case_default` | `switch`, `case`, `default` resaltados |
| `test_keyword_break_continue` | `break`, `continue` resaltados |
| `test_keyword_new_this` | `new`, `this` resaltados |
| `test_keyword_class_extends_super` | `class`, `extends`, `super` resaltados |
| `test_keyword_import_export_from` | `import`, `export`, `from` resaltados |
| `test_keyword_try_catch_finally` | `try`, `catch`, `finally` resaltados |
| `test_keyword_throw` | `throw` resaltado |
| `test_keyword_async_await` | `async`, `await` resaltados |
| `test_keyword_yield` | `yield` resaltado |
| `test_keyword_instanceof_typeof_void_delete` | 4 keywords resaltados |
| `test_keyword_true_false_null_undefined` | 4 literals resaltados |
| `test_keyword_nan_infinity` | `NaN`, `Infinity` resaltados |
| `test_keyword_static_get_set` | `static`, `get`, `set` resaltados |
| `test_keyword_with_debugger` | `with`, `debugger` resaltados |
| `test_keyword_of_in` | `of`, `in` resaltados |
| `test_comment_single_line` | Comentario `// ...` resaltado |
| `test_comment_inline` | Código + comentario en la misma línea |
| `test_comment_multi_line` | Comentario multilinea `/* ... */` |
| `test_string_double_quotes` | `"string"` resaltado |
| `test_string_single_quotes` | `'string'` resaltado |
| `test_string_template_literal` | `` `template` `` resaltado |
| `test_string_escaped` | `"with \"escape\""` resaltado |
| `test_number_integer` | Entero `42` resaltado |
| `test_number_float` | Float `3.14` resaltado |
| `test_number_hex` | Hex `0xFF` resaltado |
| `test_number_binary` | Binario `0b1010` resaltado |
| `test_number_octal` | Octal `0o77` resaltado |
| `test_number_underscore` | `1_000_000` resaltado |
| `test_number_scientific` | `6.022e23` resaltado |
| `test_operators` | Operadores aritméticos |
| `test_operators_comparison` | `==`, `!=`, `>=`, etc. |
| `test_operator_ternary` | Operador ternario `? :` |
| `test_identifier` | Identificador no resaltado |
| `test_identifier_dollar` | `$element` identificador |
| `test_identifier_underscore_start` | `_private` identificador |
| `test_empty_string` | Input vacío → output vacío |
| `test_multiline_simulation` | 5 líneas de JS, una por una |
| `test_complex_expression` | Expresión con `map`/`filter` |
| `test_destructuring` | Desestructuración `const { a } = obj` |
| `test_arrow_function` | Arrow function `=>` |
| `test_optional_chaining` | `?.` operador |
| `test_nullish_coalescing` | `??` operador |
| `test_spread_operator` | `...` spread |
| `test_async_await_full` | async/await completo |
| `test_class_with_constructor` | class + constructor + this |
| `test_promise_chaining` | `.then().catch()` |
| `test_via_highlight_lang` | Llamada a `highlight_lang("javascript")` |

### test_lexer_ts.py — Lexer de TypeScript (43 tests)

Tests para `ts_lexer_run()`, el lexer flex para código TypeScript puro.

| Test | Descripción |
|:---|:---|
| `test_keyword_const` | `const` resaltado (heredado JS) |
| `test_keyword_function` | `function` resaltado |
| `test_keyword_class` | `class` resaltado |
| `test_keyword_async_await` | `async`, `await` resaltados |
| `test_keyword_import_export` | `import`, `from`, `export` resaltados |
| `test_keyword_true_false_null_undefined` | 4 literals resaltados |
| `test_keyword_type` | `type` resaltado (TS) |
| `test_keyword_interface` | `interface` resaltado (TS) |
| `test_keyword_enum` | `enum` resaltado (TS) |
| `test_keyword_namespace` | `namespace` resaltado (TS) |
| `test_keyword_module` | `module` resaltado (TS) |
| `test_keyword_declare` | `declare` resaltado (TS) |
| `test_keyword_abstract` | `abstract` resaltado (TS) |
| `test_keyword_implements` | `implements` resaltado (TS) |
| `test_keyword_readonly` | `readonly` resaltado (TS) |
| `test_keyword_access_modifiers` | `private`, `protected`, `public` resaltados |
| `test_keyword_keyof` | `keyof` resaltado (TS) |
| `test_keyword_infer` | `infer` resaltado (TS) |
| `test_keyword_is` | `is` resaltado (TS) |
| `test_keyword_asserts` | `asserts` resaltado (TS) |
| `test_keyword_satisfies` | `satisfies` resaltado (TS) |
| `test_keyword_override` | `override` resaltado (TS) |
| `test_comment_single_line` | Comentario `// ...` |
| `test_comment_multi_line` | Comentario multilinea `/* ... */` |
| `test_string_double_quotes` | `"string"` resaltado |
| `test_string_single_quotes` | `'string'` resaltado |
| `test_string_template_literal` | `` `template` `` resaltado |
| `test_number_integer` | Entero `42` resaltado |
| `test_number_hex` | Hex `0xFF` resaltado |
| `test_number_float` | Float `3.14` resaltado |
| `test_type_annotation` | Anotación de tipo `string`, `void` |
| `test_generic_type` | Genérico `<T>` |
| `test_union_type` | Unión `string \| number` |
| `test_interface_with_methods` | Interface con métodos |
| `test_enum_with_values` | Enum con valores |
| `test_class_with_access_modifiers` | class + private/protected/public |
| `test_abstract_class` | class abstracta |
| `test_readonly_property` | Propiedad readonly |
| `test_type_assertion` | `as Type` |
| `test_satisfies_example` | `satisfies Record` |
| `test_empty_string` | Input vacío → output vacío |
| `test_multiline_simulation` | 11 líneas de TS |
| `test_complex_expression` | Expresión con tipos |
| `test_mapped_type` | Tipo mapeado `{ [K in keyof T] }` |
| `test_conditional_type` | Tipo condicional `T extends U ? X : Y` |
| `test_via_highlight_lang` | Llamada a `highlight_lang("typescript")` |

### test_lexer_tsx.py — Lexer de TSX con JSX (37 tests)

Tests para `tsx_lexer_run()`, el lexer flex para TSX con soporte JSX completo.
Usa estados exclusivos `JSX_TAG` y `JSX_CHILD` para manejar etiquetas web.

| Test | Descripción |
|:---|:---|
| `test_keyword_const` | `const` resaltado (heredado TS) |
| `test_keyword_function` | `function` resaltado |
| `test_keyword_type` | `type` resaltado (TS) |
| `test_keyword_interface` | `interface` resaltado (TS) |
| `test_keyword_class` | `class` resaltado |
| `test_keyword_return` | `return` resaltado |
| `test_keyword_async_await` | `async`, `await` resaltados |
| `test_jsx_opening_tag` | `<div className="box">` — tag + attr + string |
| `test_jsx_opening_tag_single_attr` | `<span>text</span>` — tag name resaltado |
| `test_jsx_component_tag` | `<Button onClick={handler}>` — componente |
| `test_jsx_nested_components` | `<App><Header /><Footer /></App>` |
| `test_jsx_self_closing` | `<img src={url} />` — self-closing |
| `test_jsx_self_closing_no_space` | `<br/>` — self-closing sin espacio |
| `test_jsx_self_closing_with_props` | `<Input type="text" ... />` — múltiples props |
| `test_jsx_closing_tag` | `</div>` — closing tag |
| `test_jsx_closing_component` | `</Button>` — closing component |
| `test_jsx_text_content` | `<p>Hello World</p>` — texto entre tags |
| `test_jsx_expression_in_content` | `<span>{name}</span>` — expresión en contenido |
| `test_jsx_mixed_content` | `<div>Hi {user.name}!</div>` — mixto |
| `test_jsx_string_attribute` | `<div id="main">` — attr string |
| `test_jsx_single_quote_attribute` | `<div className='active'>` |
| `test_jsx_expression_attribute` | `<div onClick={handleClick}>` — attr expresión |
| `test_jsx_full_component` | Componente completo con JSX |
| `test_jsx_conditional_rendering` | `{cond ? <A /> : <B />}` |
| `test_jsx_list_rendering` | `{items.map(item => <li>...)}` |
| `test_jsx_fragment` | `<><Header /><Content /></>` |
| `test_jsx_deep_nesting` | 4 niveles de anidación |
| `test_jsx_html_entity` | `&lt; &gt; &amp;` en contenido |
| `test_jsx_multiple_attributes` | 5 atributos en una etiqueta |
| `test_jsx_closing_tag_in_expression` | `<div>{show && <Modal />}</div>` |
| `test_comment_outside_jsx` | `// comment` fuera de JSX |
| `test_string_outside_jsx` | `"React"` fuera de JSX |
| `test_template_literal_outside_jsx` | `` `template` `` fuera de JSX |
| `test_empty_string` | Input vacío → output vacío |
| `test_multiline_simulation` | 12 líneas de TSX |
| `test_complex_component` | Componente completo con useState |
| `test_via_highlight_lang` | Llamada a `highlight_lang("tsx")` |

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

### test_transitions.py — Transiciones entre slides (14 tests)

Tests de `do_transition()`, `slider_render()` con transiciones, y parsing de transiciones.

| Test | Descripción |
|:---|:---|
| `test_transition_types_parsed` | transitions.md: fade, slide-left/right/up/down, none |
| `test_do_transition_fade` | Transición fade al 50% sin crash |
| `test_do_transition_slide_left` | Transición slide-left al 50% |
| `test_do_transition_slide_right` | Transición slide-right al 50% |
| `test_do_transition_slide_up` | Transición slide-up al 50% |
| `test_do_transition_slide_down` | Transición slide-down al 50% |
| `test_do_transition_progress_zero` | Progress 0.0 sin crash |
| `test_do_transition_progress_one` | Progress 1.0 sin crash |
| `test_do_transition_same_index_returns` | from==to retorna sin hacer nada |
| `test_do_transition_invalid_from` | from=-1 no crashea |
| `test_do_transition_all_progress_values` | 5 valores de progress (0.0–1.0) |
| `test_render_slide_with_transition_at_midpoint` | `slider_render()` con transición a 150ms |
| `test_render_slide_with_transition_expired` | Transición expirada (>300ms) resetea a NONE |
| `test_transition_export_png_triggers` | `export_png` en slide con transición no crashea |

### test_renderer_gaps.py — Renderer, img_config, notes, limits, hot reload (65 tests)

Tests que cubren las brechas del renderer: bullet2, tasks, num_list, img_config parsing,
notas del presentador, límites del parser, y hot reload.

#### TestRendererGaps (14 tests)

| Test | Descripción |
|:---|:---|
| `test_bullet_level2_parsed` | `  - Sub item` → `LINE_BULLET2` con texto correcto |
| `test_task_unchecked_parsed` | `- [ ] Todo item` → `LINE_TASK_UNCHECKED` |
| `test_task_checked_parsed` | `- [x] Done item` → `LINE_TASK_CHECKED` |
| `test_num_list_parsed` | 3 items numerados → 3 × `LINE_NUM_LIST` |
| `test_mixed_content_types` | Slide con title, subtitle, body, bullets, tasks, quote |
| `test_render_slide_with_all_line_types` | Render slide con todos los tipos de línea |
| `test_render_slide_bullet2` | Render slide con 3 niveles de viñetas |
| `test_render_tasks_checked_and_unchecked` | Render slide con tasks pendientes y completadas |
| `test_render_num_list` | Render slide con listas numeradas |
| `test_render_code_block` | Render slide con bloque de código C |
| `test_render_table` | Render slide con tabla |
| `test_render_blockquote` | Render slide con blockquote multilinea |
| `test_render_invalid_index` | `render_slide(-1)` y `render_slide(999)` no crashean |
| `test_render_null_slider` | `render_slide(NULL)` no crashea |

#### TestImgConfig (29 tests)

| Test | Descripción |
|:---|:---|
| `test_img_config_fit_cover` | `fit=cover` parseado correctamente |
| `test_img_config_fit_fill` | `fit=fill` parseado correctamente |
| `test_img_config_fit_contain` | `fit=contain` parseado correctamente |
| `test_img_config_width_px` | `width=300` → 300px |
| `test_img_config_width_pct` | `width=50%` → 50 con unit=PCT |
| `test_img_config_height_px` | `height=200` → 200px |
| `test_img_config_height_pct` | `height=75%` → 75 con unit=PCT |
| `test_img_config_opacity_float` | `opacity=0.5` → 0.5 |
| `test_img_config_opacity_pct` | `opacity=80%` → 0.8 |
| `test_img_config_radius` | `radius=20` → 20 |
| `test_img_config_rotate` | `rotate=-6.5` → -6.5 (valores negativos) |
| `test_img_config_align_left` | `align=left` |
| `test_img_config_align_right` | `align=right` |
| `test_img_config_align_center` | `align=center` |
| `test_img_config_all_combined` | Todos los parámetros juntos |
| `test_img_config_malformed_ignored` | Clave inválida ignorada, válida procesada |
| `test_img_config_negative_radius_clamped` | `radius=-10` → 0 (clamped) |
| `test_img_config_opacity_clamped_above` | `opacity=2.0` → 1.0 (clamped) |
| `test_img_config_opacity_clamped_below` | `opacity=-0.5` → 0.0 (clamped) |
| `test_img_config_render_cover` | Render con `fit=cover` |
| `test_img_config_render_fill` | Render con `fit=fill` |
| `test_img_config_render_align_right` | Render con `align=right, width=200` |
| `test_img_config_render_rotate` | Render con `rotate=15, opacity=0.7` |
| `test_img_config_render_radius` | Render con `radius=30, opacity=0.6` |
| `test_img_config_render_width_height` | Render con `width=300, height=200` |
| `test_img_config_render_height_only` | Render con solo `height=200` |
| `test_img_config_render_width_pct` | Render con `width=50%` |
| `test_img_config_render_height_pct` | Render con `height=60%` |
| `test_missing_image_placeholder` | Imagen inexistente → placeholder |

#### TestNotes (10 tests)

| Test | Descripción |
|:---|:---|
| `test_notes_inline` | `<!-- notes: Hello -->` → notas presentes |
| `test_notes_multiline` | Notas multilinea con 3 líneas |
| `test_notes_note_alias` | `<!-- note: -->` alias funcional |
| `test_notes_uppercase` | `<!-- NOTES: -->` mayúsculas funcional |
| `test_notes_per_slide` | Cada slide tiene sus propias notas |
| `test_notes_with_content_after` | Notas seguidas de contenido body |
| `test_notes_empty_slide` | Slide sin notas → cadena vacía |
| `test_notes_trimmed` | Notas con espacios extra → trimming |
| `test_notes_multiline_no_close` | Nota sin `-->` de cierre |
| `test_notes_print_does_not_crash` | `slider_print_notes()` no crashea |

#### TestParserLimits (9 tests)

| Test | Descripción |
|:---|:---|
| `test_many_slides` | 50 slides cargadas correctamente |
| `test_max_approaching_slides` | 255 slides (cerca de MAX_SLIDES=256) |
| `test_long_line` | Línea de 900 chars (cerca de MAX_LINE_LEN=1024) |
| `test_many_columns_table` | Tabla de 15 columnas (cerca de MAX_COLS=16) |
| `test_empty_file` | Archivo vacío → NULL |
| `test_single_slide_no_separator` | Slide único sin separador `---` |
| `test_code_block_unclosed` | Bloque de código sin ` ``` ` de cierre |
| `test_transition_on_first_slide` | Transición en slide 0 parseada |
| `test_kiosk_interval` | `kiosk_interval_ms` legible |

#### TestHotReload (3 tests)

| Test | Descripción |
|:---|:---|
| `test_filepath_stored` | `filepath` contiene la ruta del archivo |
| `test_last_mtime_nonzero` | `last_mtime > 0` tras carga |
| `test_reload_preserves_theme` | Recarga preserva theme configurado |

### test_empty_warnings.py — Warnings para archivos vacíos (7 tests)

Tests que verifican que `slider_load()` emite `[LINTER WARNING]` en stderr cuando
el archivo markdown está vacío o no contiene contenido significativo.

| Test | Descripción |
|:---|:---|
| `test_empty_file_warning` | Archivo vacío (0 bytes) → NULL + warning "Archivo vacio" |
| `test_spaces_only_warning` | Solo espacios → warning "sin contenido" |
| `test_tabs_only_warning` | Solo tabs → warning "sin contenido" |
| `test_newlines_only_warning` | Solo saltos de línea → warning "sin contenido" |
| `test_only_separators_warning` | Solo separadores `---` → warning "sin contenido" |
| `test_content_no_warning` | Archivo con contenido real → sin warning |
| `test_single_line_no_warning` | Una línea de texto → sin warning |

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
| `test_lexer_py.py` | 46 | `py_lexer_run()`, `highlighter_highlight("python")` |
| `test_lexer_go.py` | 58 | `go_lexer_run()`, `highlighter_highlight("go")` |
| `test_lexer_js.py` | 54 | `js_lexer_run()`, `highlighter_highlight("javascript")` |
| `test_lexer_ts.py` | 43 | `ts_lexer_run()`, `highlighter_highlight("typescript")` |
| `test_lexer_tsx.py` | 37 | `tsx_lexer_run()`, `highlighter_highlight("tsx")` con JSX_TAG/JSX_CHILD |
| `test_themes.py` | 16 | `theme_default()`, `theme_find()`, `slider_set_theme()`, `slider_set_color()` |
| `test_export.py` | 11 | `slider_export_png/jpg/svg/pdf/gif()` |
| `test_transitions.py` | 14 | `do_transition()`, `slider_render()` con transiciones |
| `test_renderer_gaps.py` | 65 | `slider_render()` (bullet2/tasks/numlist/table/code), `ImageConfig`, notas, límites, hot reload |
| `test_empty_warnings.py` | 7 | `slider_load()` warnings para archivos vacíos |
| `test_integration.py` | 25 | `slider_load()`, `slider_free()`, `slider_get_count()`, getters/setters, frontmatter |
| `test_visual.py` | 2 | `slider_export_png()`, `slider_set_theme()` |
| **Total** | **435** | |

## Cobertura de código (Coverage)

Para generar un reporte de cobertura de código C:

```bash
make coverage
```

Esto compila con `-fprofile-arcs -ftest-coverage`, ejecuta los tests, y genera un reporte
con `gcov`. Ver [coverage.md](coverage.md) para más detalles.

## Notas técnicas

### Port Python (ctypes)

Los tests usan `ports/python/c_slides_bindings.py` para cargar la biblioteca compartida
y llamar a las funciones C vía `ctypes`. Esto permite testear la lógica de negocio
sin necesidad de compilar un ejecutable separado para tests.

### Acceso a estado interno

Los tests que necesitan leer el estado interno del `Slider` (transiciones, notas,
img_config, etc.) usan funciones de acceso (`slider_test_get_*`) definidas en
`parser.c`. Estas funciones exponen campos internos de la struct opaca sin requerir
una definición de struct en Python (que sería frágil y dependiente de la plataforma).

### Cairo para tests de renderizado

Los tests de transiciones e img_config render crean superficies Cairo vía
`cairo_helpers.py`, que carga `libcairo-2.dll` / `libcairo.so` vía `ctypes`.
Esto permite crear superficies y contextos de renderizado sin un backend de ventana real.

### Linux vs Windows

Los paths de la biblioteca se resuelven automáticamente:
- Linux: `libslider.so`
- Windows: `slider.dll`

El binding detecta la plataforma y ajusta la extensión y prefijo del archivo de biblioteca.
