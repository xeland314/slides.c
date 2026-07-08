---
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

# Paleta Full
## Las 17 claves de color personalizadas

Todas las claves del `colors:` block ahora son funcionales.

```
colors:
  bg:         "#0f0f23"   # fondo
  title:      "#ff6b6b"   # títulos
  sub:        "#a9b1d6"   # subtítulos
  body:       "#c0caf5"   # texto normal
  bullet:     "#7dcfff"   # viñetas
  accent:     "#7aa2f7"   # acentos + barra de progreso
  num:        "#565f89"   # número de slide
```

---

# Tablas con colores propios

| Clave | Color | Se usa en |
| :--- | :--- | :--- |
| `table-hdr` | `#1a1b26` | Encabezado |
| `table-row` | `#24283b` | Filas pares |
| `table-alt` | `#1f2335` | Filas impares |
| `table-bdr` | `#3b4261` | Bordes |

> Presiona **`n`** para ocultar/mostrar el número de slide y la barra de progreso.

---

# Código con syntax highlighting

```python
def hello():
    # comentario en code-com
    name = "mundo"      # code-str
    if name:            # code-kw
        print(f"Hola {name}!")
    return [1, 2, 3]    # code-sym
```

| Clave | Color |
| :--- | :--- |
| `code-bg` | `#1a1b26` |
| `code-txt` | `#c0caf5` |
| `code-kw` | `#bb9af7` |
| `code-com` | `#565f89` |
| `code-str` | `#9ece6a` |
| `code-sym` | `#f7768e` |

---

# Listas y tareas

- **bullet** controla el color de las viñetas
  - Y también las sub-viñetas
- [x] Tarea completada
- [ ] Tarea pendiente

Las viñetas y checkboxes usan `bullet` o `accent` si no se define `bullet`.

---

# Desde la CLI

Cada color tiene su propio flag:

```bash
slides --bg "#0f0f23" --title "#ff6b6b" --sub "#a9b1d6" \
       --text "#c0caf5" --bullet "#7dcfff" --accent "#7aa2f7" \
       --num "#565f89" --table-hdr "#1a1b26" \
       --table-row "#24283b" --table-alt "#1f2335" \
       --table-bdr "#3b4261" --code-bg "#1a1b26" \
       --code-txt "#c0caf5" --code-kw "#bb9af7" \
       --code-com "#565f89" --code-str "#9ece6a" \
       --code-sym "#f7768e" presentacion.md
```

Los flags CLI tienen prioridad sobre el frontmatter.
