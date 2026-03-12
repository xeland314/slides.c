# C-Slides 🛝

Un presentador de diapositivas minimalista y de alto rendimiento escrito en **C** utilizando **X11**, **Cairo** y **Pango**. Diseñado para renderizar archivos Markdown directamente en pantalla con una estética moderna y profesional.

## Arquitectura del Proyecto

El proyecto ha sido refactorizado para separar las responsabilidades y permitir la portabilidad de sus componentes:

- `slider.h`: API pública (Firmas de funciones para facilitar el porting a Python, Ada o Dart).
- `src/core/`: Lógica del Parser y estructuras de datos internas.
- `src/render/`: Motor de dibujo basado en Cairo/Pango (soporta Markup y Anti-aliasing).
- `src/ui/`: Gestión de ventanas X11 y loop de eventos.

## Comparativa de Implementación (Markdown Spec)

La implementación actual sigue una filosofía orientada a **slides** (una línea = un elemento), lo que difiere del estándar CommonMark que es orientado a documentos de flujo continuo.

| Característica | Soporte | Detalle de Implementación |
| :--- | :---: | :--- |
| **Headers (#, ##)** | ✅ | Soporta niveles 1 y 2 con estilos diferenciados. |
| **Énfasis (Bold/Italic)** | ✅ | Implementado mediante Pango Markup (`**`, `__`, `*`, `_`). |
| **Listas (Bullets)** | ✅ | Soporta dos niveles (`-` y `  - `). |
| **Imágenes** | ✅ | Carga de PNGs con auto-escalado y cache. |
| **Tablas (GFM)** | ✅ | Renderizado completo con headers y filas alternas. |
| **Párrafos** | ✅ | Texto normal con soporte de wrapping automático. |
| **Blockquotes** | ✅ | Implementado con barra lateral acentuada y texto en color secundario. |
| **Código (Blocks/Inline)** | ❌ | Pendiente de implementación (requiere fuente monoespaciada). |
| **Enlaces [text](url)** | ❌ | No implementado (X11 no gestiona clicks en texto por defecto). |
| **Anidamiento Complejo** | ⚠️ | Limitado por el parser lineal; no soporta listas dentro de tablas. |

## Especificaciones Técnicas

- **Parser:** Lineal basado en prefijos (rápido, O(n)).
- **Rendering:** Sub-pixel precision con Cairo.
- **Tipografía:** Renderizado de fuentes del sistema vía Pango (default: Inter).
- **Performance:** Doble buffer para transiciones sin parpadeo (flicker-free).

## Uso

```bash
make
./slides [-p palette] presentacion.md
```

**Paletas disponibles:**
- `dark` (Por defecto): El tema azul oscuro original.
- `rose`: Un tema elegante basado en tonos vino y rosa.
- `monokai`: Estilo retro-vibrante con fondo oscuro y colores neón.
- `nord`: Estética nórdica, limpia y profesional basada en tonos fríos.
- `light`: Tema claro de alto contraste, ideal para lectura en papel.

**Controles:**
- `->` / `Enter`: Siguiente diapositiva.
- `<-` / `Backspace`: Diapositiva anterior.
- `F`: Pantalla completa (Toggle).
- `Q` / `ESC`: Salir.
