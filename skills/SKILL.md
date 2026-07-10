---
name: c-slides
description: >
  Crea presentaciones en formato Markdown para C-Slides (github.com/xeland314/slides.c), un presentador
  minimalista de alto rendimiento escrito en C con X11/Win32, Cairo y Pango. Usa esta skill siempre que
  el usuario quiera crear una presentación, diapositivas, slides, o un archivo .md para C-Slides. También
  úsala cuando quieran convertir contenido existente (notas, documentos, ideas) en una presentación, cuando
  pidan "hacer slides de X", o cuando necesiten frontmatter, transiciones entre slides, notas del
  presentador o exportación a PNG/PDF/JPG/SVG/GIF. Si el usuario menciona C-Slides, alguno de sus 11 temas
  (dark, rose, monokai, nord, light, blue, ambercat, dracula, gruvbox, catppuccin, tokyo-night), o quiere
  exportar slides, esta skill es la indicada.
---

# C-Slides Skill

Genera archivos Markdown compatibles con C-Slides: un presentador minimalista para desarrolladores
que renderiza Markdown directamente con Cairo + Pango, sobre X11 (Linux/BSD) o Win32 (Windows nativo).

## Flujo de trabajo

1. **Entender el contenido**: ¿Tema, audiencia, número de slides aproximado?
2. **Generar el archivo `.md`**: Siguiendo la sintaxis C-Slides al pie de la letra.
3. **Entregar el archivo** listo para usar con `./slides presentacion.md`.

Si el usuario no da suficiente contexto, haz preguntas breves antes de generar.

---

## Sintaxis C-Slides

> ⚠️ C-Slides usa un parser **lineal orientado a slides** (una línea = un elemento).
> NO es CommonMark. Seguir estas reglas es crítico para que el renderizado sea correcto.

### Separador de slides
```
---
```
Cada `---` en una línea sola crea una nueva diapositiva. La primera slide no necesita `---` antes.

### Títulos
```markdown
# Título Principal       ← H1: grande, color primario
## Subtítulo             ← H2: mediano, color secundario
```

### Texto y énfasis
```markdown
Párrafo normal con wrapping automático.
*Itálica*   **Negrita**   ***Ambos***
`código inline`
```

### Listas (máximo 2 niveles)
```markdown
- Item de primer nivel
- Otro item
  - Sub-item (dos espacios + guión)
  - Otro sub-item
```

### Listas numeradas
```markdown
1. Primer paso
2. Segundo paso
a) Alternativa A
b) Alternativa B
i) Sub-punto en numeración romana
```
Soporta `1.`, `a)`, `i)` y variantes similares como marcador inicial de línea.

### Checklists
```markdown
- [x] Tarea completada
- [ ] Tarea pendiente
```

### Imágenes
```markdown
!nombre_archivo.png
```
Solo el nombre del archivo (sin corchetes ni paréntesis). El archivo debe estar junto al `.md`.

### Imágenes con configuración (tamaño, posición, ajuste)
```markdown
!img/foto.jpg
  width: 70%
  height: 60%
  fit: contain
```
Propiedades opcionales en líneas consecutivas indentadas:
- `width`: ancho en px o % (porcentaje del slide)
- `height`: alto en px o % (porcentaje del slide)
- `fit`: modo de ajuste (`contain`, `cover`, `fill`, `none`)
- `x`: posición horizontal en px
- `y`: posición vertical en px

### Tablas
```markdown
| Columna A | Columna B | Columna C |
| :--- | :---: | ---: |
| izquierda | centro | derecha |
| valor | valor | valor |
```
Alineación: `:---` izquierda, `:---:` centro, `---:` derecha.

### Blockquotes
```markdown
> "Texto de la cita aquí."
```

### Bloques de código con resaltado
````markdown
```c
int main() {
    // comentario
    return 0;
}
```
````
Lenguajes soportados: `c`, `python`, `javascript`, `go`, `bash`, y cualquier otro
(el resaltado es general: keywords, strings, símbolos, comentarios).

### Frontmatter (metadatos YAML)
Al inicio del archivo, entre `---`, se puede fijar tema/fuente/escala por defecto:
```markdown
---
theme: dracula
font: JetBrains Mono
scale: 1.2
---

# Primera slide
...
```
Los flags de la CLI (`-p`, `-f`, `-s`, etc.) sobrescriben lo definido en el frontmatter.
⚠️ El primer bloque `---...---` del archivo se interpreta como frontmatter, no como
separador de slide; la primera diapositiva real empieza justo después de cerrarlo.

### Transiciones entre slides
Se configuran por slide con un comentario HTML justo antes del contenido de esa slide:
```markdown
<!-- transition: slide-left -->
# Slide con transición personalizada
```
Tipos válidos: `fade`, `slide-left`, `slide-right`, `slide-up`, `slide-down`, `none`.

### Notas del presentador
No se muestran en pantalla; se imprimen en consola al navegar a esa slide.
```markdown
<!-- notes: Recordar mencionar el benchmark antes de pasar a la demo -->
```
También soporta forma multilínea:
```markdown
<!-- notes:
Punto 1 a recordar.
Punto 2 a recordar.
-->
```

---

## Estructura recomendada de una presentación

```
# Título de la Presentación
## Subtítulo o autor

Introducción breve del tema.

---
# Sección 1
## Primer punto importante

- Concepto clave
- Otro concepto
  - Detalle adicional

---
# Sección 2
## Comparativa

| Opción | Ventaja | Estado |
| :--- | :--- | :---: |
| A | Rápida | ✅ |
| B | Flexible | ⚠️ |

---
# Conclusión
## Resumen

> "Frase memorable de cierre."

- Punto final 1
- Punto final 2
```

---

## Reglas de diseño para buenos slides

- **Una idea por slide**: No sobrecargar con texto. Si hay mucho, dividir en más slides.
- **Títulos descriptivos**: El H1 es el tema, el H2 es el ángulo o subtema.
- **Listas cortas**: Máximo 5-6 items por lista. Más = nueva slide.
- **Código enfocado**: Mostrar solo el fragmento relevante, no archivos completos.
- **Tablas concisas**: Máximo 4-5 columnas. Preferir 3.
- **Citas para impacto**: Una cita por slide, al final como remate.
- **Imágenes con propósito**: Cada imagen debe aportar información, no decorar.
- **Checklists para progreso**: Usar `- [x]` / `- [ ]` para mostrar avance.

---

## Paletas disponibles

C-Slides trae 11 temas integrados. Informar al usuario sobre las opciones para ejecutar
su presentación:

| Paleta | Comando | Estilo |
| :--- | :--- | :--- |
| `dark` (default) | `./slides archivo.md` | Fondo oscuro, texto blanco, acentos neutros |
| `rose` | `./slides -p rose archivo.md` | Vino oscuro, acentos rosados |
| `monokai` | `./slides -p monokai archivo.md` | Vibrante, títulos rosados, viñetas verdes |
| `nord` | `./slides -p nord archivo.md` | Azul-gris ártico, azules y verdes suaves |
| `light` | `./slides -p light archivo.md` | Fondo blanco, texto oscuro, acentos azules |
| `blue` | `./slides -p blue archivo.md` | Pizarra azul profunda, acentos cielo |
| `ambercat` | `./slides -p ambercat archivo.md` | Ámbar/marrón técnico |
| `dracula` | `./slides -p dracula archivo.md` | Púrpura/rosa/cian vibrante |
| `gruvbox` | `./slides -p gruvbox archivo.md` | Tono tierra retro |
| `catppuccin` | `./slides -p catppuccin archivo.md` | Pastel suave (mocha) |
| `tokyo-night` | `./slides -p tokyo-night archivo.md` | Ciberpunk azul/rosa/cian |

También se pueden fijar colores individuales sin usar un tema completo:
```bash
./slides --bg 1a1a2e --title ff6b9d --text eaeaea --accent 4ecdc4 archivo.md
```

### Exportación

```bash
./slides --export png archivo.md                       # una imagen por slide
./slides --export pdf archivo.md                        # PDF completo
./slides --export jpg archivo.md                        # JPG (más comprimido)
./slides --export svg archivo.md                        # SVG vectorial
./slides --export gif archivo.md                        # GIF animado con transiciones
./slides --export png --export-res 1920x1080 archivo.md # resolución custom (default 1080x1080)
./slides --export png --slide 2 archivo.md               # solo una slide (0-index)
./slides --palette dracula --export pdf archivo.md       # combinar tema + exportación
```

### Otras opciones útiles

| Flag | Descripción |
| :--- | :--- |
| `-p, --palette <nombre>` | Elegir tema |
| `-f, --font-family <str>` | Tipografía (ej. `'JetBrains Mono'`) |
| `-s, --font-scale <num>` | Factor de escala (ej. `1.2`) |
| `-e, --export <tipo>` | Exportar a `pdf`, `png`, `jpg`, `svg` o `gif` |
| `-er, --export-res <WxH>` | Resolución de exportación |
| `-sl, --slide <num>` | Slide específico a exportar (0-index) |
| `--bg / --title / --sub / --text / --accent <hex>` | Colores individuales |
| `--bullet / --num / --table-hdr / --table-row / --table-alt / --table-bdr <hex>` | Colores de tabla |
| `--code-bg / --code-txt / --code-kw / --code-com / --code-str / --code-sym <hex>` | Colores de código |
| `--kiosk <sec>` / `--auto-advance <sec>` | Avance automático cada N segundos (loop infinito) |
| `-v, --version` / `-h, --help` | Versión / ayuda |

Los flags de CLI sobrescriben lo definido en el frontmatter del archivo.

---

## Colores configurables individualmente

Cada color del tema se puede sobrescribir por separado. Los flags aceptan hex sin `#`:
- `--bg` / `--title` / `--sub` / `--text` / `--bullet` / `--accent` / `--num`
- `--table-hdr` / `--table-row` / `--table-alt` / `--table-bdr`
- `--code-bg` / `--code-txt` / `--code-kw` / `--code-com` / `--code-str` / `--code-sym`

El flag `--accent` también ajusta `--bullet` automáticamente.

Cuando un usuario no especifica un color, el tema base provee el valor por defecto.

---

## Qué NO soporta C-Slides

- ❌ **Enlaces** `[texto](url)` — no implementado
- ❌ **Listas dentro de tablas** — parser lineal no lo soporta  
- ❌ **HTML inline** — ignorado
- ❌ **Más de 2 niveles de listas** — solo `-` y `  -`

Si el usuario pide algo de esta lista, sugerir la alternativa más cercana soportada.

---

## Entrega

Siempre:
1. Crear el archivo `.md` y entregarlo con `present_files`.
2. Incluir el comando de ejecución sugerido (con paleta recomendada según el tema).
3. Mencionar brevemente cuántas slides tiene la presentación.
4. Si el usuario ya sabe qué tema/fuente/escala quiere usar siempre, ofrecer fijarlo vía
   frontmatter en el propio archivo en lugar de repetir flags de CLI cada vez.

Ver `references/ejemplos.md` para ejemplos completos de distintos tipos de presentaciones.
