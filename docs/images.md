# Directiva `img:` — Control de imágenes

Controla el renderizado de imágenes desde comentarios HTML en el Markdown.

## Gramática

```
<!-- img: clave=valor, clave=valor, ... -->
![alt](ruta.png)
```

Las propiedades se separan por coma y se aplican a la imagen de la línea
siguiente. Si la línea siguiente no es una imagen, la directiva se ignora
silenciosamente. Líneas vacías entre la directiva y la imagen se toleran.

## Propiedades

| Clave | Valores | Ejemplo | Descripción |
|-------|---------|---------|-------------|
| `fit` | `cover`, `contain`, `fill` | `fit=cover` | Prioridad máxima. Ignora `width` y `height`. |
| `width` | Entero (px) o Porcentaje | `width=50%` | Ancho objetivo. Solo: calcula alto con aspect ratio. |
| `height` | Entero (px) o Porcentaje | `height=80%` | Alto objetivo. Solo: calcula ancho con aspect ratio. |
| `opacity` | Flotante `0.0`–`1.0` o `0%`–`100%` | `opacity=0.4` | Transparencia. Mapea a `cairo_paint_with_alpha`. |
| `radius` | Entero (px) | `radius=20` | Esquinas redondeadas. Aplica clip con arcos. |
| `rotate` | Flotante (grados, `-180`–`180`) | `rotate=-5` | Rotación sobre el centro geométrico. |
| `align` | `center`, `left`, `right` | `align=left` | Alineación horizontal en la slide. |

## Semántica

### Resolución de dimensiones

- **fit=cover**: escala la imagen para llenar el contenedor (la más grande de
  las escalas), recortando el sobrante con clip.
- **fit=contain**: escala la imagen para que quepa completa (la más chica de
  las escalas). Es el comportamiento por defecto.
- **fit=fill**: estira la imagen a las dimensiones exactas del contenedor,
  ignorando el aspect ratio.
- **Solo width**: el alto se calcula manteniendo el aspect ratio original.
- **Solo height**: el ancho se calcula manteniendo el aspect ratio original.
- **Ambos**: la imagen se forza a esas dimensiones exactas (equivalente a
  fit=fill, pero con valores arbitrarios).

### Ciclo de vida

1. El parser encuentra `<!-- img: ... -->` y almacena la configuración en un
   estado temporal.
2. Si la siguiente línea (ignorando vacíos) es `![...](ruta)` o `!ruta`, la
   configuración se aplica a esa imagen.
3. Si la siguiente línea no es una imagen, la configuración se descarta.
4. No hay fuga de estado entre slides.

### Valores por defecto

| Propiedad | Default |
|-----------|---------|
| `fit` | `contain` |
| `width` | no seteado |
| `height` | no seteado |
| `opacity` | `1.0` |
| `radius` | `0` |
| `rotate` | `0.0` |
| `align` | `center` |

## Ejemplos

```markdown
<!-- img: fit=cover, opacity=0.5 -->
![fondo](textura.png)

<!-- img: width=400, radius=15, align=right -->
![decorativa](screenshot.png)

<!-- img: rotate=-3, opacity=0.85 -->
![inclinada](foto.jpg)

<!-- img: height=60%, width=60%, align=left, radius=10 -->
![logo](icon.png)
```
