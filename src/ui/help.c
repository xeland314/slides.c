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
    printf("  --sub <hex>             Subtitle color (e.g. '#a9b1d6')\n");
    printf("  --text <hex>            Body text color (e.g. '#c0caf5')\n");
    printf("  --accent <hex>          Accent color (e.g. '#7aa2f7')\n");
    printf("  --kiosk <sec>           Auto-advance every N seconds (infinite loop)\n");
    printf("  --auto-advance <sec>    Same as --kiosk\n");
    printf("  --bullet <hex>          Bullet point color (e.g. '#7dcfff')\n");
    printf("  --accent <hex>          Accent color (also sets bullet) (e.g. '#7aa2f7')\n");
    printf("  --num <hex>             Slide number color (e.g. '#565f89')\n");
    printf("  --table-hdr <hex>       Table header background (e.g. '#1a1b26')\n");
    printf("  --table-row <hex>       Table row background (e.g. '#24283b')\n");
    printf("  --table-alt <hex>       Table alternate row background (e.g. '#1f2335')\n");
    printf("  --table-bdr <hex>       Table border color (e.g. '#3b4261')\n");
    printf("  --code-bg <hex>         Code block background (e.g. '#1a1b26')\n");
    printf("  --code-txt <hex>        Code text color (e.g. '#c0caf5')\n");
    printf("  --code-kw <hex>         Code keyword color (e.g. '#bb9af7')\n");
    printf("  --code-com <hex>        Code comment color (e.g. '#565f89')\n");
    printf("  --code-str <hex>        Code string color (e.g. '#9ece6a')\n");
    printf("  --code-sym <hex>        Code symbol color (e.g. '#f7768e')\n");
    printf("  -h, --help              Show this help\n");
    printf("  -v, --version           Show version\n");
}

void print_version(void) {
    printf("c-slides v0.2.2\n");
}
