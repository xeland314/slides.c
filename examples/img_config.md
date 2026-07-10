# Control de imágenes — `img:`

## fit=contain (default)

Sin directiva — escala uniforme dentro del ancho disponible:

![gato](examples/gato.jpg)

---

## fit=cover

La imagen llena todo el contenedor recortando el sobrante:

<!-- img: fit=cover, opacity=0.85 -->
![preview](examples/preview_dracula.png)

---

## fit=fill

La imagen se estira a las dimensiones exactas del contenedor:

<!-- img: fit=fill, opacity=0.7 -->
![preview](examples/preview_monokai.png)

---

## width + height exactos

Forzar dimensiones en píxeles:

<!-- img: width=200, height=150 -->
![gato](examples/gato.jpg)

---

## width con porcentaje

<!-- img: width=40%, radius=12 -->
![gato](examples/gato.jpg)

---

## align=right + width

<!-- img: width=300, align=right, radius=8 -->
![gato](examples/gato.jpg)

---

## align=left + width

<!-- img: width=250, align=left, opacity=0.9, radius=15 -->
![preview](examples/preview_catppuccin.png)

---

## rotate (-6 grados)

<!-- img: rotate=-6, opacity=0.8 -->
![preview](examples/preview_nord.png)

---

## radius + opacity

<!-- img: radius=30, opacity=0.75 -->
![gato](examples/gato.jpg)

---

## Todas las propiedades combinadas

<!-- img: width=60%, rotate=3, radius=20, opacity=0.85, align=right -->
![preview](examples/preview_gruvbox.png)

---

## height con porcentaje + center

<!-- img: height=50%, align=center, radius=10 -->
![gato](examples/gato.jpg)

---

## fit=cover, radius (se ignora)

Con `fit=cover` el `radius` no aplica (el clip es del contenedor):

<!-- img: fit=cover, radius=30, opacity=0.6 -->
![preview](examples/preview_light.png)

---

## GIF animado con opacidad

<!-- img: opacity=0.7 -->
![tree](examples/tree-in-ball.gif)
