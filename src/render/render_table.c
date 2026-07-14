#include "render_table.h"
#include "render_util.h"
#include <string.h>

double render_table(cairo_t *cr, PangoLayout *lay_body,
                    Slider *s, SlideLine *lines, int start, int count,
                    double x, double y, double max_w) {
    if (count == 0) return 0.0;
    int max_cols = 0, header_row = -1;
    for (int i = 0; i < count; i++) {
        if (lines[start + i].type == LINE_TABLE_ROW && lines[start + i].ncols > max_cols)
            max_cols = lines[start + i].ncols;
        if (lines[start + i].type == LINE_TABLE_SEP) header_row = i;
    }
    if (max_cols == 0) return 0.0;
    if (max_cols > MAX_COLS) max_cols = MAX_COLS;

    double col_w[MAX_COLS];
    double desired_w[MAX_COLS] = {0};
    double total_desired_w = 0.0;

    for (int i = 0; i < count; i++) {
        SlideLine *sl = &lines[start + i];
        if (sl->type != LINE_TABLE_ROW) continue;
        for (int c = 0; c < sl->ncols && c < max_cols; c++) {
            static char markup[MAX_LINE_LEN * 4];
            md_to_markup(sl->cols[c], markup, sizeof(markup));
            pango_layout_set_markup(lay_body, markup, -1);
            pango_layout_set_width(lay_body, -1);
            int tw, th;
            pango_layout_get_pixel_size(lay_body, &tw, &th);
            double w = (double)tw + 24.0 * s->font_scale;
            if (w > desired_w[c]) desired_w[c] = w;
        }
    }

    for (int c = 0; c < max_cols; c++) {
        if (desired_w[c] < 40.0 * s->font_scale) desired_w[c] = 40.0 * s->font_scale;
        total_desired_w += desired_w[c];
    }

    for (int c = 0; c < max_cols; c++) {
        col_w[c] = (desired_w[c] / total_desired_w) * max_w;
    }

    double cur_y = y;
    int data_row = 0;

    for (int i = 0; i < count; i++) {
        SlideLine *sl = &lines[start + i];
        if (sl->type == LINE_TABLE_SEP) continue;
        if (sl->type != LINE_TABLE_ROW) continue;
        int is_header = (header_row >= 0 && i == 0);

        double row_h = 40.0 * s->font_scale;
        for (int c = 0; c < sl->ncols && c < max_cols; c++) {
            pango_layout_set_width(lay_body, (int)((col_w[c] - 12.0 * s->font_scale) * PANGO_SCALE));
            static char markup[MAX_LINE_LEN * 4];
            md_to_markup(sl->cols[c], markup, sizeof(markup));
            pango_layout_set_markup(lay_body, markup, -1);
            int tw, th;
            pango_layout_get_pixel_size(lay_body, &tw, &th);
            if ((double)th + 16.0 * s->font_scale > row_h) row_h = (double)th + 16.0 * s->font_scale;
        }

        if (is_header) SET_COLOR(cr, s->theme->table_hdr);
        else if (data_row % 2 == 0) SET_COLOR(cr, s->theme->table_row);
        else SET_COLOR(cr, s->theme->table_alt);

        cairo_rectangle(cr, x, cur_y, max_w, row_h);
        cairo_fill(cr);

        SET_COLOR(cr, s->theme->table_bdr);
        cairo_set_line_width(cr, 0.5 * s->font_scale);

        double cur_v_x = x;
        for (int c = 0; c <= max_cols; c++) {
            cairo_move_to(cr, cur_v_x, cur_y);
            cairo_line_to(cr, cur_v_x, cur_y + row_h);
            cairo_stroke(cr);
            if (c < max_cols) cur_v_x += col_w[c];
        }
        cairo_move_to(cr, x, cur_y + row_h);
        cairo_line_to(cr, x + max_w, cur_y + row_h);
        cairo_stroke(cr);

        if (is_header) SET_COLOR(cr, s->theme->bullet);
        else SET_COLOR(cr, s->theme->body);

        double cur_text_x = x;
        for (int c = 0; c < sl->ncols && c < max_cols; c++) {
            pango_layout_set_width(lay_body, (int)((col_w[c] - 12.0 * s->font_scale) * PANGO_SCALE));
            static char markup[MAX_LINE_LEN * 4];
            md_to_markup(sl->cols[c], markup, sizeof(markup));
            pango_layout_set_markup(lay_body, markup, -1);
            int tw, th;
            pango_layout_get_pixel_size(lay_body, &tw, &th);
            cairo_move_to(cr, cur_text_x + 6.0 * s->font_scale, cur_y + (row_h - (double)th) / 2.0);
            pango_cairo_show_layout(cr, lay_body);
            cur_text_x += col_w[c];
        }
        cur_y += row_h;
        if (!is_header) data_row++;
    }
    SET_COLOR(cr, s->theme->table_bdr);
    cairo_set_line_width(cr, 1.0 * s->font_scale);
    cairo_rectangle(cr, x, y, max_w, cur_y - y);
    cairo_stroke(cr);
    pango_layout_set_width(lay_body, (int)((max_w) * PANGO_SCALE));
    return cur_y - y;
}
