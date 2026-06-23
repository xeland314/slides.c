---
colors:
  bg: "#0f0f23"
  title: "#ff6b6b"
  body: "#c0caf5"
  accent: "#7aa2f7"
---

# Colores Personalizados
## Tema inspirado en Tokyo Night

Este slide usa colores definidos en el `frontmatter` del archivo.

| Clave | Color |
| :--- | :--- |
| `bg` | `#0f0f23` |
| `title` | `#ff6b6b` |
| `body` | `#c0caf5` |
| `accent` | `#7aa2f7` |

---

<!-- transition: slide-right -->

# Probar desde CLI

```bash
./slides --bg "#1b1b2f" --title "#e43d5a" --text "#e0e0e0" --accent "#43aa8b" examples/custom_colors.md
```

Los argumentos CLI **sobrescriben** los colores del frontmatter.

---

<!-- transition: slide-right -->

# Cómo funciona

- `bg` — color de fondo
- `title` — color de títulos (##)
- `text` / `body` — color del texto normal
- `accent` — acentos, viñetas y barras decorativas

Los demás colores (subtítulos, tablas, código) heredan del tema base.

---

<!-- transition: slide-right -->

# Combinar con temas

```bash
# Tema Dracula con acento dorado
./slides --palette dracula --accent "#f0c000" presentacion.md
```

Las paletas personalizadas complementan cualquier tema integrado.
