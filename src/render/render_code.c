#include "render_code.h"
#include "render_util.h"
#include "../core/highlighter.h"
#include <stdlib.h>
#include <string.h>

double render_code_block(cairo_t *cr, PangoLayout *lay_code,
                         Slider *s, SlideLine *lines, int start, int count,
                         double x, double y, double max_w) {
    if (count <= 0) return 0.0;

    double lines_h_sum = 0.0;
    double *line_heights = malloc(count * sizeof(double));
    if (!line_heights) return 0.0;

    const char *lang = lines[start].text;

    for (int i = 0; i < count; i++) {
        line_heights[i] = 0.0;
        SlideLine *sl = &lines[start + i];
        if (sl->type == LINE_CODE_START || sl->type == LINE_CODE_END) continue;

        static char markup[MAX_LINE_LEN * 8];
        highlighter_highlight(sl->text, lang, s->theme, markup, sizeof(markup));
        pango_layout_set_markup(lay_code, markup, -1);

        int tw, th;
        pango_layout_get_pixel_size(lay_code, &tw, &th);

        double min_h = 24.0 * s->font_scale;
        double actual_h = (double)th;
        if (actual_h < min_h) {
            actual_h = min_h;
        }
        line_heights[i] = actual_h;
        lines_h_sum += actual_h;
    }

    double total_h = lines_h_sum + 20.0 * s->font_scale;
    if (total_h < 20.0 * s->font_scale) total_h = 20.0 * s->font_scale;

    SET_COLOR(cr, s->theme->code_bg);
    cairo_rectangle(cr, x, y, max_w, total_h);
    cairo_fill(cr);

    SET_COLOR(cr, s->theme->code_txt);

    double cur_y = y + 10.0 * s->font_scale;
    for (int i = 0; i < count; i++) {
        SlideLine *sl = &lines[start + i];
        if (sl->type == LINE_CODE_START || sl->type == LINE_CODE_END) continue;

        static char markup[MAX_LINE_LEN * 8];
        highlighter_highlight(sl->text, lang, s->theme, markup, sizeof(markup));
        pango_layout_set_markup(lay_code, markup, -1);

        cairo_move_to(cr, x + 15.0 * s->font_scale, cur_y);
        pango_cairo_show_layout(cr, lay_code);
        cur_y += line_heights[i];
    }

    free(line_heights);
    return total_h;
}
