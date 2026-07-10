# Personalización de colores

Se pueden personalizar 17 claves de color del tema activo desde el
**frontmatter** del archivo Markdown o via **CLI**. Los flags CLI tienen
prioridad sobre el frontmatter.

## Via frontmatter (YAML)

```yaml
---
theme: dracula
colors:
  bg: "#0f0f23"
  title: "#ff6b6b"
  sub: "#a9b1d6"
  body: "#c0caf5"
  bullet: "#7dcfff"
  accent: "#7aa2f7"
  num: "#565f89"
  table-hdr: "#1a1b26"
  table-row: "#24283b"
  table-alt: "#1f2335"
  table-bdr: "#3b4261"
  code-bg: "#1a1b26"
  code-txt: "#c0caf5"
  code-kw: "#bb9af7"
  code-com: "#565f89"
  code-str: "#9ece6a"
  code-sym: "#f7768e"
---
```

## Via CLI

Cada clave tiene su flag `--<clave> <hex>`:

| Flag | Clave |
|------|-------|
| `--bg` | Fondo general |
| `--title` | Títulos (`#`) |
| `--sub` / `--subtitle` | Subtítulos (`##`) |
| `--text` / `--body` | Texto de cuerpo |
| `--bullet` | Viñetas |
| `--accent` | Acento (también define bullet si no se usa `--bullet`) |
| `--num` / `--number` | Número de slide |
| `--table-hdr` | Encabezado de tabla |
| `--table-row` | Fila de tabla |
| `--table-alt` | Fila alterna de tabla |
| `--table-bdr` | Bordes de tabla |
| `--code-bg` | Fondo de bloque de código |
| `--code-txt` | Texto de código |
| `--code-kw` | Palabras clave de código |
| `--code-com` | Comentarios de código |
| `--code-str` | Strings de código |
| `--code-sym` | Símbolos de código |

Ejemplo:

```bash
./slides --bg "#0f0f23" --title "#ff6b6b" --text "#c0caf5" presentacion.md
```

## Comportamiento

- Los valores CLI sobrescriben los del frontmatter.
- Si no se define `--bullet`, hereda el valor de `--accent`.
- Valores no especificados mantienen los del tema activo.
