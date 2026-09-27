# Ideas para nuevas features

## Paletas personalizadas
- Colores por hex desde CLI (`--bg "#0f0f23" --title "#ff6b6b" --text "#c0caf5" --accent "#7aa2f7"`)
- Colores por hex desde frontmatter (sección `colors:` con bg, title, body, accent, etc.)
- ✅ Implementado: CLI + frontmatter, keys: bg, title, body/text, accent (accent also sets bullet)

## Imagen de fondo
- Imagen de fondo por slide o global (`--bg-image fondo.png`)

## Duración de transición configurable
- `--transition-ms 500` desde CLI
- `<!-- transition-duration: 500 -->` por slide o frontmatter

## Auto-avance (modo kiosko)
- `--autoplay 5` — avanza cada 5 segundos

## Zoom
- `Ctrl++` / `Ctrl+-` para acercar/alejar
- ✅ Implementado: Win32 + X11, Ctrl++/Ctrl+-/Ctrl+0, rango 0.3x–5.0x

## Mini-mapa / vista general
- `TAB` para ver todas las slides en miniatura
- ✅ Implementado: Win32 + X11, teclado (TAB/ESC/flechas/Enter) y ratón

## Resaltado de sintaxis por lenguaje
- Detectar ` ```python ` vs ` ```c ` para colorear específico por lenguaje

## Numeración de diapositivas
- Mostrar "N / M" de forma más prominente

## Gradientes de fondo
- Degradados en vez de color sólido

Algunas ideas basadas en el README y el estado actual del proyecto:
- Enlaces Markdown — Es la única feature de Markdown listada como "no soportado" en el README ([texto](url)). Sería la adición más natural.
- Exportar a SVG — Ya tienes PNG y PDF, SVG es el que falta y cairo lo soporta nativamente.
- GIF animado con transiciones — El renderer ya menciona GIF en su comentario de arquitectura, pero no hay flag de exportación para ello. Sería interesante exportar la presentación completa como GIF animado respetando las transiciones.
- Barra de progreso / número de slide en pantalla — No hay indicador visual de progreso durante la presentación.
- Duración de transición configurable — Hoy está fija en 300ms. Podría ir en frontmatter o como flag CLI.
- Auto-advance mode (--kiosk) — Avance automático cada N segundos, útil para vitrinas o displays públicos.
- Vista de notas del presentador en ventana separada — Ya soportan notas con <!-- notes: -->, pero solo se imprimen en consola. Una segunda ventana (o segundo monitor) con las notas sería un gran paso.
- Temas personalizados por el usuario — Un archivo de tema CSS-like o JSON que el usuario pueda cargar sin recompilar.
- Soporte para Wayland — Hoy solo X11 y Win32.
- Lenzuelaje <!-- fit: --> o similar — Para imágenes que ocupen toda la slide sin márgenes.