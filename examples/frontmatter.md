---
theme: dracula
font-family: "JetBrains Mono"
font-scale: 1.0
---

# Frontmatter en Acción
## Configuración desde el archivo

Este slide usa los valores definidos en el frontmatter de arriba.

```
---
theme: dracula
font-family: "JetBrains Mono"
font-scale: 1.0
---
```

- **theme:** `dracula` — paleta de colores vibrante
- **font-family:** `JetBrains Mono` — tipografía monoespaciada
- **font-scale:** `1.0` — tamaño normal

> ⚠️ Los argumentos de CLI (`-p`, `-f`, `-s`) tienen PRIORIDAD sobre el frontmatter.

---

# Temas desde Frontmatter
## Cambia la paleta sin flags de CLI

| Tema | Clave |
| :--- | :--- |
| Dark (default) | `dark` |
| Rose | `rose` |
| Monokai | `monokai` |
| Nord | `nord` |
| Light | `light` |
| Blue | `blue` |
| Ambercat | `ambercat` |
| Dracula | `dracula` |
| Gruvbox | `gruvbox` |
| Catppuccin | `catppuccin` |
| Tokyo Night | `tokyo-night` |

Ejemplo:
```
---
theme: dracula
---
```

---

# Fonts y Escala
## Personalización tipográfica

Puedes definir cualquier fuente instalada en el sistema:

```
---
font-family: "Inter"
font-scale: 1.2
---
```

- **font-family:** Cualquier fuente Pango-compatible
- **font-scale:** Factor de escala (ej. `1.0`, `1.5`, `0.8`)

---

# Combinado
## Todas las opciones juntas

```
---
theme: monokai
font-family: "Fira Code"
font-scale: 1.1
---
```

| Opción | Valor |
| :--- | :--- |
| Tema | Monokai |
| Fuente | Fira Code |
| Escala | 1.1 × |

> También puedes usar `palette` en lugar de `theme`, `font` en lugar de `font-family`, y `scale` en lugar de `font-scale` como alias.
