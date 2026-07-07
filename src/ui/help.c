#include "help.h"
#include <stdio.h>

void print_help(const char *prog) {
    printf("Usage: %s [options] presentation.md\n\n", prog);
    printf("Options:\n");
    printf("  -p, --palette <name>    Choose palette (dark, rose, monokai, nord, light, blue, ambercat,\n");
    printf("                          dracula, gruvbox, catppuccin, tokyo-night)\n");
    printf("  -f, --font-family <str> Font family (e.g. 'Arial', 'JetBrains Mono')\n");
    printf("  -s, --font-scale <num>  Scale font size (e.g. 1.2)\n");
    printf("  -e, --export <type>     Export slides to 'pdf', 'png', 'svg', 'gif' or 'jpg'\n");
    printf("  -er, --export-res <WxH> Export resolution (e.g. 1920x1080, default 1080x1080)\n");
    printf("  -sl, --slide <num>      Select specific slide to export (0-indexed)\n");
    printf("  --bg <hex>              Background color (e.g. '#0f0f23')\n");
    printf("  --title <hex>           Title color (e.g. '#ff6b6b')\n");
    printf("  --text <hex>            Body text color (e.g. '#c0caf5')\n");
    printf("  --accent <hex>          Accent color (e.g. '#7aa2f7')\n");
    printf("  -h, --help              Show this help\n");
    printf("  -v, --version           Show version\n");
}

void print_version(void) {
    printf("c-slides v0.2.2\n");
}
