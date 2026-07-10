#include "../../slider.h"
#include "../core/internal.h"
#include "render_util.h"
#include "render_table.h"
#include "render_code.h"
#include "render_transition.h"
#include <pango/pangocairo.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void slider_render(Slider *s, int index, cairo_t *cr, int win_w, int win_h, double time_ms) {
    if (!s || index < 0 || index >= s->n_slides) return;

    TransitionType trans_type = s->transition_type;
    if (trans_type == TRANS_NONE) trans_type = s->slides[index].transition;
    double trans_ms = TRANSITION_DEFAULT_MS;

    if (trans_type != TRANS_NONE && time_ms < trans_ms && s->transition_from != index) {
        double progress = time_ms / trans_ms;
        progress = progress * progress * (3.0 - 2.0 * progress);
        do_transition(s, s->transition_from, index, cr, win_w, win_h, progress);
        return;
    }

    if (s->transition_type != TRANS_NONE && time_ms >= trans_ms) {
        s->transition_type = TRANS_NONE;
    }

    Slide *slide = &s->slides[index];
    double content_w = win_w - MARGIN_X * 2.0;

    slide->has_anim = false;

    char f_title[128], f_subtitle[128], f_body[128], f_bullet[128], f_num[128], f_code[128];
    snprintf(f_title,    sizeof(f_title),    "%s Bold %d", s->font_family, (int)(44 * s->font_scale));
    snprintf(f_subtitle, sizeof(f_subtitle), "%s %d",      s->font_family, (int)(26 * s->font_scale));
    snprintf(f_body,     sizeof(f_body),     "%s %d",      s->font_family, (int)(20 * s->font_scale));
    snprintf(f_bullet,   sizeof(f_bullet),   "%s %d",      s->font_family, (int)(18 * s->font_scale));
    snprintf(f_num,      sizeof(f_num),      "%s %d",      s->font_family, (int)(13 * s->font_scale));
    snprintf(f_code,     sizeof(f_code),     "Monospace %d",             (int)(16 * s->font_scale));

    PangoLayout *lay_title    = make_layout(cr, f_title,    content_w);
    PangoLayout *lay_subtitle = make_layout(cr, f_subtitle, content_w);
    PangoLayout *lay_body     = make_layout(cr, f_body,     content_w);
    PangoLayout *lay_bullet   = make_layout(cr, f_body,     content_w - 30.0 * s->font_scale);
    PangoLayout *lay_bullet2  = make_layout(cr, f_bullet,   content_w - 60.0 * s->font_scale);
    PangoLayout *lay_num      = make_layout(cr, f_num,      200.0 * s->font_scale);
    PangoLayout *lay_code     = make_layout(cr, f_code,     content_w - 30.0 * s->font_scale);

    double y = MARGIN_Y;
    int i = 0;
    while (i < slide->nlines) {
        const SlideLine *sl = &slide->lines[i];
        switch (sl->type) {
        case LINE_EMPTY: y += 12.0 * s->font_scale; i++; break;
        case LINE_TITLE:
            SET_COLOR(cr, s->theme->title);
            y += render_pango(cr, lay_title, sl->text, MARGIN_X, y);
            SET_COLOR(cr, s->theme->accent);
            cairo_set_line_width(cr, 2.5 * s->font_scale);
            cairo_move_to(cr, MARGIN_X, y + 8.0 * s->font_scale);
            cairo_line_to(cr, MARGIN_X + content_w, y + 8.0 * s->font_scale);
            cairo_stroke(cr);
            y += 22.0 * s->font_scale; i++; break;
        case LINE_SUBTITLE:
            SET_COLOR(cr, s->theme->sub);
            y += render_pango(cr, lay_subtitle, sl->text, MARGIN_X, y);
            y += 14.0 * s->font_scale; i++; break;
        case LINE_BODY:
            SET_COLOR(cr, s->theme->body);
            y += render_pango(cr, lay_body, sl->text, MARGIN_X, y);
            y += 8.0 * s->font_scale; i++; break;
        case LINE_BLOCKQUOTE: {
            double b_x = MARGIN_X + 10.0 * s->font_scale;
            double t_x = b_x + 25.0 * s->font_scale;
            SET_COLOR(cr, s->theme->accent);
            cairo_set_line_width(cr, 4.0 * s->font_scale);
            char markup[MAX_LINE_LEN * 4];
            md_to_markup(sl->text, markup, sizeof(markup));
            pango_layout_set_markup(lay_body, markup, -1);
            int tw, th;
            pango_layout_get_pixel_size(lay_body, &tw, &th);
            cairo_move_to(cr, b_x, y);
            cairo_line_to(cr, b_x, y + (double)th);
            cairo_stroke(cr);
            SET_COLOR(cr, s->theme->sub);
            cairo_move_to(cr, t_x, y);
            pango_cairo_show_layout(cr, lay_body);
            y += (double)th + 12.0 * s->font_scale;
            i++; break;
        }
        case LINE_BULLET1: {
            char markup[MAX_LINE_LEN * 4];
            md_to_markup(sl->text, markup, sizeof(markup));
            pango_layout_set_markup(lay_bullet, markup, -1);

            PangoLayoutIter *iter = pango_layout_get_iter(lay_bullet);
            PangoRectangle logical_rect;
            pango_layout_iter_get_line_extents(iter, NULL, &logical_rect);
            pango_layout_iter_free(iter);
            double line_h = (double)logical_rect.height / PANGO_SCALE;
            double bullet_y = y + line_h / 2.0;

            SET_COLOR(cr, s->theme->bullet);
            cairo_arc(cr, MARGIN_X + 8.0 * s->font_scale, bullet_y, 4.0 * s->font_scale, 0, 2 * M_PI);
            cairo_fill(cr);

            SET_COLOR(cr, s->theme->body);
            int tw, th;
            pango_layout_get_pixel_size(lay_bullet, &tw, &th);
            cairo_move_to(cr, MARGIN_X + 22.0 * s->font_scale, y);
            pango_cairo_show_layout(cr, lay_bullet);
            y += (double)th + 6.0 * s->font_scale;
            i++; break;
        }
        case LINE_NUM_LIST:
            SET_COLOR(cr, s->theme->bullet);
            render_pango(cr, lay_bullet, sl->marker, MARGIN_X + 2.0 * s->font_scale, y);
            SET_COLOR(cr, s->theme->body);
            y += render_pango(cr, lay_bullet, sl->text, MARGIN_X + 40.0 * s->font_scale, y) + 6.0 * s->font_scale;
            i++; break;
        case LINE_TASK_UNCHECKED:
        case LINE_TASK_CHECKED: {
            char markup[MAX_LINE_LEN * 4];
            md_to_markup(sl->text, markup, sizeof(markup));
            pango_layout_set_markup(lay_bullet, markup, -1);

            PangoLayoutIter *iter = pango_layout_get_iter(lay_bullet);
            PangoRectangle logical_rect;
            pango_layout_iter_get_line_extents(iter, NULL, &logical_rect);
            pango_layout_iter_free(iter);
            double line_h = (double)logical_rect.height / PANGO_SCALE;
            double center_y = y + line_h / 2.0;

            double sz = 18.0 * s->font_scale;
            double bx = MARGIN_X + 2.0 * s->font_scale;
            double by = center_y - sz / 2.0;

            SET_COLOR(cr, s->theme->accent);
            cairo_set_line_width(cr, 1.5 * s->font_scale);
            cairo_rectangle(cr, bx, by, sz, sz);
            cairo_stroke(cr);
            if (sl->type == LINE_TASK_CHECKED) {
                cairo_move_to(cr, bx + 4.0 * s->font_scale, by + 9.0 * s->font_scale);
                cairo_line_to(cr, bx + 8.0 * s->font_scale, by + 13.0 * s->font_scale);
                cairo_line_to(cr, bx + 14.0 * s->font_scale, by + 5.0 * s->font_scale);
                cairo_stroke(cr);
            }

            SET_COLOR(cr, s->theme->body);
            int tw, th;
            pango_layout_get_pixel_size(lay_bullet, &tw, &th);
            cairo_move_to(cr, MARGIN_X + 30.0 * s->font_scale, y);
            pango_cairo_show_layout(cr, lay_bullet);
            y += (double)th + 6.0 * s->font_scale;
            i++; break;
        }
        case LINE_BULLET2: {
            char markup[MAX_LINE_LEN * 4];
            md_to_markup(sl->text, markup, sizeof(markup));
            pango_layout_set_markup(lay_bullet2, markup, -1);

            PangoLayoutIter *iter = pango_layout_get_iter(lay_bullet2);
            PangoRectangle logical_rect;
            pango_layout_iter_get_line_extents(iter, NULL, &logical_rect);
            pango_layout_iter_free(iter);
            double line_h = (double)logical_rect.height / PANGO_SCALE;
            double bullet_y = y + line_h / 2.0;

            double bx = MARGIN_X + 38.0 * s->font_scale;
            SET_COLOR(cr, s->theme->accent);
            cairo_move_to(cr, bx, bullet_y - 4.0 * s->font_scale);
            cairo_line_to(cr, bx + 4.0 * s->font_scale, bullet_y);
            cairo_line_to(cr, bx, bullet_y + 4.0 * s->font_scale);
            cairo_line_to(cr, bx - 4.0 * s->font_scale, bullet_y);
            cairo_close_path(cr); cairo_fill(cr);

            SET_COLOR(cr, s->theme->body);
            int tw, th;
            pango_layout_get_pixel_size(lay_bullet2, &tw, &th);
            cairo_move_to(cr, MARGIN_X + 52.0 * s->font_scale, y);
            pango_cairo_show_layout(cr, lay_bullet2);
            y += (double)th + 4.0 * s->font_scale;
            i++; break;
        }
        case LINE_IMAGE: {
            ImgCache *imgc = get_image_cache(sl->text);
            if (imgc && imgc->surfaces) {
                if (imgc->n_frames > 1) slide->has_anim = true;

                cairo_surface_t *img_sfc = NULL;
                if (imgc->n_frames == 1) {
                    img_sfc = imgc->surfaces[0];
                } else if (imgc->n_frames > 1 && imgc->total_duration > 0) {
                    int t = (int)time_ms % imgc->total_duration;
                    int acc = 0;
                    for (int k = 0; k < imgc->n_frames; k++) {
                        acc += imgc->delays[k];
                        if (acc > t) {
                            img_sfc = imgc->surfaces[k];
                            break;
                        }
                    }
                    if (!img_sfc) img_sfc = imgc->surfaces[0];
                }

                if (img_sfc) {
                    int iw = cairo_image_surface_get_width(img_sfc);
                    int ih = cairo_image_surface_get_height(img_sfc);
                    double avail_h = win_h - y - MARGIN_Y - 40.0 * s->font_scale;
                    const ImageConfig *cfg = sl->img_cfg;
                    double dw = (double)iw, dh = (double)ih, ox;

                    if (cfg && cfg->active && cfg->fit == IMG_FIT_COVER) {
                        double sx = content_w / (double)iw;
                        double sy = avail_h / (double)ih;
                        double scale = (sx > sy) ? sx : sy;
                        dw = content_w; dh = avail_h;
                        cairo_save(cr);
                        cairo_translate(cr, MARGIN_X, y);
                        cairo_rectangle(cr, 0, 0, dw, dh);
                        cairo_clip(cr);
                        cairo_translate(cr, (content_w - iw*scale)/2.0, (avail_h - ih*scale)/2.0);
                        cairo_scale(cr, scale, scale);
                        cairo_set_source_surface(cr, img_sfc, 0, 0);
                        cairo_paint_with_alpha(cr, cfg->opacity);
                        cairo_restore(cr);
                    } else if (cfg && cfg->active && cfg->fit == IMG_FIT_FILL) {
                        dw = content_w; dh = avail_h;
                        cairo_save(cr);
                        cairo_translate(cr, MARGIN_X, y);
                        cairo_scale(cr, dw/iw, dh/ih);
                        cairo_set_source_surface(cr, img_sfc, 0, 0);
                        cairo_paint_with_alpha(cr, cfg->opacity);
                        cairo_restore(cr);
                    } else {
                        double scale = 1.0;
                        double cont_w = content_w;
                        if (cfg && cfg->active && cfg->width > 0)
                            cont_w = cfg->width_unit == UNIT_PCT ? content_w * cfg->width / 100.0 : (double)cfg->width;
                        if (cfg && cfg->active && cfg->height > 0) {
                            double th = cfg->height_unit == UNIT_PCT ? avail_h * cfg->height / 100.0 : (double)cfg->height;
                            if (cfg->width > 0) { dw = cont_w; dh = th; }
                            else { scale = th / ih; dw = iw * scale; dh = th; }
                        } else if (cfg && cfg->active && cfg->width > 0) {
                            scale = cont_w / iw; dw = cont_w; dh = ih * scale;
                        } else {
                            if (iw > cont_w) scale = cont_w / iw;
                            if (ih * scale > avail_h) scale = avail_h / ih;
                            dw = iw * scale; dh = ih * scale;
                        }
                        ox = MARGIN_X;
                        if (cfg && cfg->active && cfg->align == IMG_ALIGN_RIGHT) ox = MARGIN_X + content_w - dw;
                        else if (!cfg || !cfg->active || cfg->align == IMG_ALIGN_CENTER) ox = MARGIN_X + (content_w - dw) / 2.0;

                        cairo_save(cr);
                        cairo_translate(cr, ox + dw/2.0, y + dh/2.0);
                        if (cfg && cfg->active && cfg->rotate != 0.0) cairo_rotate(cr, cfg->rotate * G_PI / 180.0);
                        cairo_translate(cr, -dw/2.0, -dh/2.0);
                        if (cfg && cfg->active && cfg->radius > 0) {
                            double r = cfg->radius;
                            cairo_new_sub_path(cr);
                            cairo_arc(cr, dw - r, r, r, -G_PI_2, 0);
                            cairo_arc(cr, dw - r, dh - r, r, 0, G_PI_2);
                            cairo_arc(cr, r, dh - r, r, G_PI_2, G_PI);
                            cairo_arc(cr, r, r, r, G_PI, 3 * G_PI_2);
                            cairo_close_path(cr);
                            cairo_clip(cr);
                        }
                        cairo_scale(cr, scale, scale);
                        cairo_set_source_surface(cr, img_sfc, 0, 0);
                        if (cfg && cfg->active && cfg->opacity < 1.0)
                            cairo_paint_with_alpha(cr, cfg->opacity);
                        else
                            cairo_paint(cr);
                        cairo_restore(cr);
                    }
                    y += dh + 14.0 * s->font_scale;
                }
            } else {
                set_color(cr, 0.15, 0.18, 0.38); cairo_rectangle(cr, MARGIN_X, y, content_w, 80.0 * s->font_scale); cairo_fill(cr);
                set_color(cr, COL_LABEL_R, COL_LABEL_G, COL_LABEL_B);
                char msg[MAX_LINE_LEN + 32]; snprintf(msg, sizeof(msg), "\u26a0 No se encontr\u00f3: %s", sl->text);
                render_pango(cr, lay_body, msg, MARGIN_X + 10.0 * s->font_scale, y + 28.0 * s->font_scale);
                y += 94.0 * s->font_scale;
            }
            i++; break;
        }
        case LINE_TABLE_ROW:
        case LINE_TABLE_SEP: {
            int j = i;
            while (j < slide->nlines && (slide->lines[j].type == LINE_TABLE_ROW || slide->lines[j].type == LINE_TABLE_SEP)) j++;
            y += render_table(cr, lay_body, s, (SlideLine *)slide->lines, i, j - i, MARGIN_X, y, content_w) + 14.0 * s->font_scale;
            i = j; break;
        }
        case LINE_CODE_START: {
            int j = i;
            while (j < slide->nlines && slide->lines[j].type != LINE_CODE_END) j++;
            if (j < slide->nlines) j++;
            y += render_code_block(cr, lay_code, s, (SlideLine *)slide->lines, i, j - i, MARGIN_X, y, content_w) + 14.0 * s->font_scale;
            i = j; break;
        }
        default: i++; break;
        }
    }
    if (!s->hide_num) {
        char num_buf[32]; snprintf(num_buf, sizeof(num_buf), "%d / %d", index + 1, s->n_slides);
        SET_COLOR(cr, s->theme->num);
        pango_layout_set_width(lay_num, (int)(200.0 * s->font_scale * PANGO_SCALE)); pango_layout_set_alignment(lay_num, PANGO_ALIGN_RIGHT);
        render_pango(cr, lay_num, num_buf, win_w - MARGIN_X - 200.0 * s->font_scale, win_h - 32.0 * s->font_scale);
        double prog = (s->n_slides > 1) ? (double)index / (s->n_slides - 1) : 1.0;
        set_color(cr, COLOR_R(s->theme->bg)*1.5, COLOR_G(s->theme->bg)*1.5, COLOR_B(s->theme->bg)*1.5); cairo_rectangle(cr, 0, win_h - 4, win_w, 4); cairo_fill(cr);
        SET_COLOR(cr, s->theme->accent); cairo_rectangle(cr, 0, win_h - 4, win_w * prog, 4); cairo_fill(cr);
    }
    g_object_unref(lay_title); g_object_unref(lay_subtitle); g_object_unref(lay_body);
    g_object_unref(lay_bullet); g_object_unref(lay_bullet2); g_object_unref(lay_num);
    g_object_unref(lay_code);
}
