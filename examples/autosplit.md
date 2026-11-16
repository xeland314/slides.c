# Proyecto Ejemplo

Un README típico, sin separadores de diapositiva. C-Slides lo divide automáticamente.

## Instalación

Clona el repositorio y compila con el Makefile incluido.

```bash
git clone https://example.com/proyecto.git
cd proyecto
make -j8
```

Los requisitos principales son un compilador C99, Cairo, Pango y X11.

## Uso

Ejecuta el binario pasando el archivo markdown como argumento.

- Abre la presentación en una ventana
- Navega con las flechas o con la barra espaciadora
- Exporta a PNG, PDF o GIF con `--export`

## Configuración

Las opciones se pueden fijar con un bloque de metadatos al inicio del archivo, o desde la línea de comandos. La línea de comandos tiene prioridad.

Las claves más usadas son el tema, la fuente y la escala del texto.

## Temas visuales

Hay varios temas integrados: dark, monokai, nord, rose y light.

Cada tema define colores para el fondo, los títulos, el cuerpo y el código.

## Arquitectura

El núcleo está escrito en C y separa el parser, el renderizador y el backend de ventana.

- El parser convierte el markdown en diapositivas
- El renderizador dibuja cada diapositiva con Cairo y Pango
- El backend gestiona eventos, transiciones y exportación

## Contribuir

Las contribuciones son bienvenidas. Abre un issue antes de proponer cambios grandes y sigue las pautas de CONTRIBUTING.md.

Antes de enviar un pull request, ejecuta las pruebas.

```bash
make test
```

## Licencia

El proyecto se distribuye bajo licencia MIT.
