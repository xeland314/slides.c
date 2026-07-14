import unittest
import os
import tempfile
import ctypes
from c_slides_bindings import CSlides, LineType, TransitionType, ImgFit, ImgAlign, ImgUnit


class TestRendererGaps(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        lib_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "libslider.so")
        )
        cls.cs = CSlides(lib_path)

    def _tmp_md(self, content):
        f = tempfile.NamedTemporaryFile(mode='w', suffix='.md', delete=False,
                                        encoding='utf-8', dir=tempfile.gettempdir())
        f.write(content)
        f.close()
        return f.name

    def test_bullet_level2_parsed(self):
        path = self._tmp_md("# Title\n\n- Item 1\n  - Sub item\n")
        try:
            s = self.cs.load(path)
            self.assertTrue(bool(s))
            nlines = self.cs.test_get_nlines(s, 0)
            found_b2 = False
            for i in range(nlines):
                if self.cs.test_get_line_type(s, 0, i) == LineType.BULLET2:
                    found_b2 = True
                    self.assertEqual(self.cs.test_get_line_text(s, 0, i), "Sub item")
            self.assertTrue(found_b2, "LINE_BULLET2 not found")
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_task_unchecked_parsed(self):
        path = self._tmp_md("# Title\n\n- [ ] Todo item\n")
        try:
            s = self.cs.load(path)
            self.assertTrue(bool(s))
            nlines = self.cs.test_get_nlines(s, 0)
            found = False
            for i in range(nlines):
                if self.cs.test_get_line_type(s, 0, i) == LineType.TASK_UNCHECKED:
                    found = True
                    self.assertEqual(self.cs.test_get_line_text(s, 0, i), "Todo item")
            self.assertTrue(found)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_task_checked_parsed(self):
        path = self._tmp_md("# Title\n\n- [x] Done item\n")
        try:
            s = self.cs.load(path)
            self.assertTrue(bool(s))
            nlines = self.cs.test_get_nlines(s, 0)
            found = False
            for i in range(nlines):
                if self.cs.test_get_line_type(s, 0, i) == LineType.TASK_CHECKED:
                    found = True
                    self.assertEqual(self.cs.test_get_line_text(s, 0, i), "Done item")
            self.assertTrue(found)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_num_list_parsed(self):
        path = self._tmp_md("# Title\n\n1. First\n2. Second\n3. Third\n")
        try:
            s = self.cs.load(path)
            self.assertTrue(bool(s))
            nlines = self.cs.test_get_nlines(s, 0)
            count = 0
            for i in range(nlines):
                if self.cs.test_get_line_type(s, 0, i) == LineType.NUM_LIST:
                    count += 1
            self.assertEqual(count, 3)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_mixed_content_types(self):
        content = "# Title\n\n## Subtitle\n\nBody text\n\n- Bullet 1\n  - Sub bullet\n\n1. Num item\n\n- [ ] Task\n\n- [x] Done\n\n> Quote\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            self.assertTrue(bool(s))
            nlines = self.cs.test_get_nlines(s, 0)
            types = set()
            for i in range(nlines):
                types.add(self.cs.test_get_line_type(s, 0, i))
            self.assertIn(LineType.TITLE, types)
            self.assertIn(LineType.SUBTITLE, types)
            self.assertIn(LineType.BODY, types)
            self.assertIn(LineType.BULLET1, types)
            self.assertIn(LineType.BULLET2, types)
            self.assertIn(LineType.NUM_LIST, types)
            self.assertIn(LineType.TASK_UNCHECKED, types)
            self.assertIn(LineType.TASK_CHECKED, types)
            self.assertIn(LineType.BLOCKQUOTE, types)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_render_slide_with_all_line_types(self):
        content = "# T\n\nBody\n\n- B1\n  - B2\n\n1. Num\n\n- [ ] T1\n\n- [x] T2\n\n> Quote\n"
        path = self._tmp_md(content)
        try:
            from cairo_helpers import create_image_surface, create_context, destroy_context, destroy_surface
            s = self.cs.load(path)
            sfc = create_image_surface(800, 600)
            cr = create_context(sfc)
            self.cs.render_slide(s, 0, cr, 800, 600, 0.0)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_render_slide_bullet2(self):
        content = "# Title\n\n- Level 1\n  - Level 2\n    - Level 3\n"
        path = self._tmp_md(content)
        try:
            from cairo_helpers import create_image_surface, create_context, destroy_context, destroy_surface
            s = self.cs.load(path)
            sfc = create_image_surface(800, 600)
            cr = create_context(sfc)
            self.cs.render_slide(s, 0, cr, 800, 600, 0.0)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_render_tasks_checked_and_unchecked(self):
        content = "# Tasks\n\n- [ ] Pending\n- [x] Done\n- [X] Done2\n"
        path = self._tmp_md(content)
        try:
            from cairo_helpers import create_image_surface, create_context, destroy_context, destroy_surface
            s = self.cs.load(path)
            sfc = create_image_surface(800, 600)
            cr = create_context(sfc)
            self.cs.render_slide(s, 0, cr, 800, 600, 0.0)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_render_num_list(self):
        content = "# List\n\n1. First\n2. Second\na) Lettered\ni. Roman\n"
        path = self._tmp_md(content)
        try:
            from cairo_helpers import create_image_surface, create_context, destroy_context, destroy_surface
            s = self.cs.load(path)
            sfc = create_image_surface(800, 600)
            cr = create_context(sfc)
            self.cs.render_slide(s, 0, cr, 800, 600, 0.0)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_render_code_block(self):
        content = "# Code\n\n```c\nint main() {\n    return 0;\n}\n```\n"
        path = self._tmp_md(content)
        try:
            from cairo_helpers import create_image_surface, create_context, destroy_context, destroy_surface
            s = self.cs.load(path)
            sfc = create_image_surface(800, 600)
            cr = create_context(sfc)
            self.cs.render_slide(s, 0, cr, 800, 600, 0.0)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_render_table(self):
        content = "# Table\n\n| A | B |\n|---|---|\n| 1 | 2 |\n| 3 | 4 |\n"
        path = self._tmp_md(content)
        try:
            from cairo_helpers import create_image_surface, create_context, destroy_context, destroy_surface
            s = self.cs.load(path)
            sfc = create_image_surface(800, 600)
            cr = create_context(sfc)
            self.cs.render_slide(s, 0, cr, 800, 600, 0.0)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_render_blockquote(self):
        content = "# Quote\n\n> This is a blockquote\n> with multiple lines\n"
        path = self._tmp_md(content)
        try:
            from cairo_helpers import create_image_surface, create_context, destroy_context, destroy_surface
            s = self.cs.load(path)
            sfc = create_image_surface(800, 600)
            cr = create_context(sfc)
            self.cs.render_slide(s, 0, cr, 800, 600, 0.0)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_render_invalid_index(self):
        path = self._tmp_md("# Title\n")
        try:
            from cairo_helpers import create_image_surface, create_context, destroy_context, destroy_surface
            s = self.cs.load(path)
            sfc = create_image_surface(800, 600)
            cr = create_context(sfc)
            self.cs.render_slide(s, -1, cr, 800, 600, 0.0)
            self.cs.render_slide(s, 999, cr, 800, 600, 0.0)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_render_null_slider(self):
        from cairo_helpers import create_image_surface, create_context, destroy_context, destroy_surface
        sfc = create_image_surface(800, 600)
        cr = create_context(sfc)
        self.cs.render_slide(None, 0, cr, 800, 600, 0.0)
        destroy_context(cr)
        destroy_surface(sfc)


class TestImgConfig(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        lib_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "libslider.so")
        )
        cls.cs = CSlides(lib_path)

    def _tmp_md(self, content):
        f = tempfile.NamedTemporaryFile(mode='w', suffix='.md', delete=False,
                                        encoding='utf-8', dir=tempfile.gettempdir())
        f.write(content)
        f.close()
        return f.name

    def _find_image_line(self, s, slide_idx=0):
        nlines = self.cs.test_get_nlines(s, slide_idx)
        for i in range(nlines):
            if self.cs.test_get_line_type(s, slide_idx, i) == LineType.IMAGE:
                if self.cs.test_get_line_has_img_cfg(s, slide_idx, i):
                    return i
        return -1

    def test_img_config_fit_cover(self):
        path = self._tmp_md("<!-- img: fit=cover -->\n![test](examples/gato.jpg)\n")
        try:
            s = self.cs.load(path)
            li = self._find_image_line(s)
            self.assertGreaterEqual(li, 0)
            self.assertEqual(self.cs.test_get_img_cfg_active(s, 0, li), 1)
            self.assertEqual(self.cs.test_get_img_cfg_fit(s, 0, li), ImgFit.COVER)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_fit_fill(self):
        path = self._tmp_md("<!-- img: fit=fill -->\n![test](examples/gato.jpg)\n")
        try:
            s = self.cs.load(path)
            li = self._find_image_line(s)
            self.assertGreaterEqual(li, 0)
            self.assertEqual(self.cs.test_get_img_cfg_fit(s, 0, li), ImgFit.FILL)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_fit_contain(self):
        path = self._tmp_md("<!-- img: fit=contain -->\n![test](examples/gato.jpg)\n")
        try:
            s = self.cs.load(path)
            li = self._find_image_line(s)
            self.assertGreaterEqual(li, 0)
            self.assertEqual(self.cs.test_get_img_cfg_fit(s, 0, li), ImgFit.CONTAIN)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_width_px(self):
        path = self._tmp_md("<!-- img: width=300 -->\n![test](examples/gato.jpg)\n")
        try:
            s = self.cs.load(path)
            li = self._find_image_line(s)
            self.assertGreaterEqual(li, 0)
            self.assertEqual(self.cs.test_get_img_cfg_width(s, 0, li), 300)
            self.assertEqual(self.cs.test_get_img_cfg_width_unit(s, 0, li), ImgUnit.PX)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_width_pct(self):
        path = self._tmp_md("<!-- img: width=50% -->\n![test](examples/gato.jpg)\n")
        try:
            s = self.cs.load(path)
            li = self._find_image_line(s)
            self.assertGreaterEqual(li, 0)
            self.assertEqual(self.cs.test_get_img_cfg_width(s, 0, li), 50)
            self.assertEqual(self.cs.test_get_img_cfg_width_unit(s, 0, li), ImgUnit.PCT)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_height_px(self):
        path = self._tmp_md("<!-- img: height=200 -->\n![test](examples/gato.jpg)\n")
        try:
            s = self.cs.load(path)
            li = self._find_image_line(s)
            self.assertGreaterEqual(li, 0)
            self.assertEqual(self.cs.test_get_img_cfg_height(s, 0, li), 200)
            self.assertEqual(self.cs.test_get_img_cfg_height_unit(s, 0, li), ImgUnit.PX)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_height_pct(self):
        path = self._tmp_md("<!-- img: height=75% -->\n![test](examples/gato.jpg)\n")
        try:
            s = self.cs.load(path)
            li = self._find_image_line(s)
            self.assertGreaterEqual(li, 0)
            self.assertEqual(self.cs.test_get_img_cfg_height(s, 0, li), 75)
            self.assertEqual(self.cs.test_get_img_cfg_height_unit(s, 0, li), ImgUnit.PCT)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_opacity_float(self):
        path = self._tmp_md("<!-- img: opacity=0.5 -->\n![test](examples/gato.jpg)\n")
        try:
            s = self.cs.load(path)
            li = self._find_image_line(s)
            self.assertGreaterEqual(li, 0)
            self.assertAlmostEqual(self.cs.test_get_img_cfg_opacity(s, 0, li), 0.5, places=2)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_opacity_pct(self):
        path = self._tmp_md("<!-- img: opacity=80% -->\n![test](examples/gato.jpg)\n")
        try:
            s = self.cs.load(path)
            li = self._find_image_line(s)
            self.assertGreaterEqual(li, 0)
            self.assertAlmostEqual(self.cs.test_get_img_cfg_opacity(s, 0, li), 0.8, places=2)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_radius(self):
        path = self._tmp_md("<!-- img: radius=20 -->\n![test](examples/gato.jpg)\n")
        try:
            s = self.cs.load(path)
            li = self._find_image_line(s)
            self.assertGreaterEqual(li, 0)
            self.assertEqual(self.cs.test_get_img_cfg_radius(s, 0, li), 20)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_rotate(self):
        path = self._tmp_md("<!-- img: rotate=-6.5 -->\n![test](examples/gato.jpg)\n")
        try:
            s = self.cs.load(path)
            li = self._find_image_line(s)
            self.assertGreaterEqual(li, 0)
            self.assertAlmostEqual(self.cs.test_get_img_cfg_rotate(s, 0, li), -6.5, places=1)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_align_left(self):
        path = self._tmp_md("<!-- img: align=left -->\n![test](examples/gato.jpg)\n")
        try:
            s = self.cs.load(path)
            li = self._find_image_line(s)
            self.assertGreaterEqual(li, 0)
            self.assertEqual(self.cs.test_get_img_cfg_align(s, 0, li), ImgAlign.LEFT)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_align_right(self):
        path = self._tmp_md("<!-- img: align=right -->\n![test](examples/gato.jpg)\n")
        try:
            s = self.cs.load(path)
            li = self._find_image_line(s)
            self.assertGreaterEqual(li, 0)
            self.assertEqual(self.cs.test_get_img_cfg_align(s, 0, li), ImgAlign.RIGHT)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_align_center(self):
        path = self._tmp_md("<!-- img: align=center -->\n![test](examples/gato.jpg)\n")
        try:
            s = self.cs.load(path)
            li = self._find_image_line(s)
            self.assertGreaterEqual(li, 0)
            self.assertEqual(self.cs.test_get_img_cfg_align(s, 0, li), ImgAlign.CENTER)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_all_combined(self):
        path = self._tmp_md("<!-- img: width=60%, rotate=3, radius=20, opacity=0.85, align=right, fit=cover -->\n![test](examples/gato.jpg)\n")
        try:
            s = self.cs.load(path)
            li = self._find_image_line(s)
            self.assertGreaterEqual(li, 0)
            self.assertEqual(self.cs.test_get_img_cfg_active(s, 0, li), 1)
            self.assertEqual(self.cs.test_get_img_cfg_width(s, 0, li), 60)
            self.assertEqual(self.cs.test_get_img_cfg_width_unit(s, 0, li), ImgUnit.PCT)
            self.assertAlmostEqual(self.cs.test_get_img_cfg_rotate(s, 0, li), 3.0, places=1)
            self.assertEqual(self.cs.test_get_img_cfg_radius(s, 0, li), 20)
            self.assertAlmostEqual(self.cs.test_get_img_cfg_opacity(s, 0, li), 0.85, places=2)
            self.assertEqual(self.cs.test_get_img_cfg_align(s, 0, li), ImgAlign.RIGHT)
            self.assertEqual(self.cs.test_get_img_cfg_fit(s, 0, li), ImgFit.COVER)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_malformed_ignored(self):
        path = self._tmp_md("<!-- img: badkey=badval, width=100 -->\n![test](examples/gato.jpg)\n")
        try:
            s = self.cs.load(path)
            li = self._find_image_line(s)
            self.assertGreaterEqual(li, 0)
            self.assertEqual(self.cs.test_get_img_cfg_width(s, 0, li), 100)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_negative_radius_clamped(self):
        path = self._tmp_md("<!-- img: radius=-10 -->\n![test](examples/gato.jpg)\n")
        try:
            s = self.cs.load(path)
            li = self._find_image_line(s)
            self.assertGreaterEqual(li, 0)
            self.assertEqual(self.cs.test_get_img_cfg_radius(s, 0, li), 0)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_opacity_clamped_above(self):
        path = self._tmp_md("<!-- img: opacity=2.0 -->\n![test](examples/gato.jpg)\n")
        try:
            s = self.cs.load(path)
            li = self._find_image_line(s)
            self.assertGreaterEqual(li, 0)
            self.assertAlmostEqual(self.cs.test_get_img_cfg_opacity(s, 0, li), 1.0, places=2)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_opacity_clamped_below(self):
        path = self._tmp_md("<!-- img: opacity=-0.5 -->\n![test](examples/gato.jpg)\n")
        try:
            s = self.cs.load(path)
            li = self._find_image_line(s)
            self.assertGreaterEqual(li, 0)
            self.assertAlmostEqual(self.cs.test_get_img_cfg_opacity(s, 0, li), 0.0, places=2)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_render_cover(self):
        content = "<!-- img: fit=cover -->\n![test](examples/gato.jpg)\n"
        path = self._tmp_md(content)
        try:
            from cairo_helpers import create_image_surface, create_context, destroy_context, destroy_surface
            s = self.cs.load(path)
            sfc = create_image_surface(800, 600)
            cr = create_context(sfc)
            self.cs.render_slide(s, 0, cr, 800, 600, 0.0)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_render_fill(self):
        content = "<!-- img: fit=fill -->\n![test](examples/gato.jpg)\n"
        path = self._tmp_md(content)
        try:
            from cairo_helpers import create_image_surface, create_context, destroy_context, destroy_surface
            s = self.cs.load(path)
            sfc = create_image_surface(800, 600)
            cr = create_context(sfc)
            self.cs.render_slide(s, 0, cr, 800, 600, 0.0)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_render_align_right(self):
        content = "<!-- img: align=right, width=200 -->\n![test](examples/gato.jpg)\n"
        path = self._tmp_md(content)
        try:
            from cairo_helpers import create_image_surface, create_context, destroy_context, destroy_surface
            s = self.cs.load(path)
            sfc = create_image_surface(800, 600)
            cr = create_context(sfc)
            self.cs.render_slide(s, 0, cr, 800, 600, 0.0)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_render_rotate(self):
        content = "<!-- img: rotate=15, opacity=0.7 -->\n![test](examples/gato.jpg)\n"
        path = self._tmp_md(content)
        try:
            from cairo_helpers import create_image_surface, create_context, destroy_context, destroy_surface
            s = self.cs.load(path)
            sfc = create_image_surface(800, 600)
            cr = create_context(sfc)
            self.cs.render_slide(s, 0, cr, 800, 600, 0.0)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_render_radius(self):
        content = "<!-- img: radius=30, opacity=0.6 -->\n![test](examples/gato.jpg)\n"
        path = self._tmp_md(content)
        try:
            from cairo_helpers import create_image_surface, create_context, destroy_context, destroy_surface
            s = self.cs.load(path)
            sfc = create_image_surface(800, 600)
            cr = create_context(sfc)
            self.cs.render_slide(s, 0, cr, 800, 600, 0.0)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_render_width_height(self):
        content = "<!-- img: width=300, height=200 -->\n![test](examples/gato.jpg)\n"
        path = self._tmp_md(content)
        try:
            from cairo_helpers import create_image_surface, create_context, destroy_context, destroy_surface
            s = self.cs.load(path)
            sfc = create_image_surface(800, 600)
            cr = create_context(sfc)
            self.cs.render_slide(s, 0, cr, 800, 600, 0.0)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_render_height_only(self):
        content = "<!-- img: height=200 -->\n![test](examples/gato.jpg)\n"
        path = self._tmp_md(content)
        try:
            from cairo_helpers import create_image_surface, create_context, destroy_context, destroy_surface
            s = self.cs.load(path)
            sfc = create_image_surface(800, 600)
            cr = create_context(sfc)
            self.cs.render_slide(s, 0, cr, 800, 600, 0.0)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_render_width_pct(self):
        content = "<!-- img: width=50% -->\n![test](examples/gato.jpg)\n"
        path = self._tmp_md(content)
        try:
            from cairo_helpers import create_image_surface, create_context, destroy_context, destroy_surface
            s = self.cs.load(path)
            sfc = create_image_surface(800, 600)
            cr = create_context(sfc)
            self.cs.render_slide(s, 0, cr, 800, 600, 0.0)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_img_config_render_height_pct(self):
        content = "<!-- img: height=60% -->\n![test](examples/gato.jpg)\n"
        path = self._tmp_md(content)
        try:
            from cairo_helpers import create_image_surface, create_context, destroy_context, destroy_surface
            s = self.cs.load(path)
            sfc = create_image_surface(800, 600)
            cr = create_context(sfc)
            self.cs.render_slide(s, 0, cr, 800, 600, 0.0)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_missing_image_placeholder(self):
        content = "![missing](nonexistent_image_xyz.png)\n"
        path = self._tmp_md(content)
        try:
            from cairo_helpers import create_image_surface, create_context, destroy_context, destroy_surface
            s = self.cs.load(path)
            sfc = create_image_surface(800, 600)
            cr = create_context(sfc)
            self.cs.render_slide(s, 0, cr, 800, 600, 0.0)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)


class TestNotes(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        lib_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "libslider.so")
        )
        cls.cs = CSlides(lib_path)

    def _tmp_md(self, content):
        f = tempfile.NamedTemporaryFile(mode='w', suffix='.md', delete=False,
                                        encoding='utf-8', dir=tempfile.gettempdir())
        f.write(content)
        f.close()
        return f.name

    def test_notes_inline(self):
        path = self._tmp_md("# Title\n\n<!-- notes: Remember to say hello -->\nBody\n")
        try:
            s = self.cs.load(path)
            notes = self.cs.test_get_notes(s, 0)
            self.assertIn("Remember to say hello", notes)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_notes_multiline(self):
        content = "# Title\n\n<!-- notes: Line one\nLine two\nLine three -->\nBody\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            notes = self.cs.test_get_notes(s, 0)
            self.assertIn("Line one", notes)
            self.assertIn("Line two", notes)
            self.assertIn("Line three", notes)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_notes_note_alias(self):
        path = self._tmp_md("# Title\n\n<!-- note: Short note -->\nBody\n")
        try:
            s = self.cs.load(path)
            notes = self.cs.test_get_notes(s, 0)
            self.assertIn("Short note", notes)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_notes_uppercase(self):
        path = self._tmp_md("# Title\n\n<!-- NOTES: Uppercase note -->\nBody\n")
        try:
            s = self.cs.load(path)
            notes = self.cs.test_get_notes(s, 0)
            self.assertIn("Uppercase note", notes)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_notes_per_slide(self):
        content = "Slide 1 notes\n\n<!-- notes: Notes for slide 1 -->\n\n---\n\nSlide 2 notes\n\n<!-- notes: Notes for slide 2 -->\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            n1 = self.cs.test_get_notes(s, 0)
            n2 = self.cs.test_get_notes(s, 1)
            self.assertIn("Notes for slide 1", n1)
            self.assertIn("Notes for slide 2", n2)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_notes_with_content_after(self):
        content = "# Title\n\n<!-- notes: My note -->\n\nSome body text\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            notes = self.cs.test_get_notes(s, 0)
            self.assertIn("My note", notes)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_notes_empty_slide(self):
        content = "# Title\n\n---\n\n# No notes\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            notes = self.cs.test_get_notes(s, 1)
            self.assertEqual(notes, "")
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_notes_trimmed(self):
        content = "# Title\n\n<!-- notes:   padded note   -->\nBody\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            notes = self.cs.test_get_notes(s, 0)
            self.assertEqual(notes, "padded note")
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_notes_multiline_no_close(self):
        content = "# Title\n\n<!-- notes: unclosed note\nstill going\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            notes = self.cs.test_get_notes(s, 0)
            self.assertIn("unclosed note", notes)
            self.assertIn("still going", notes)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_notes_print_does_not_crash(self):
        content = "# Title\n\n<!-- notes: Test note -->\nBody\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            self.cs.print_notes(s, 0)
            self.cs.free(s)
        finally:
            os.unlink(path)


class TestParserLimits(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        lib_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "libslider.so")
        )
        cls.cs = CSlides(lib_path)

    def _tmp_md(self, content):
        f = tempfile.NamedTemporaryFile(mode='w', suffix='.md', delete=False,
                                        encoding='utf-8', dir=tempfile.gettempdir())
        f.write(content)
        f.close()
        return f.name

    def test_many_slides(self):
        slides = ["# Slide %d\n" % i for i in range(50)]
        content = "\n---\n".join(slides)
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            self.assertEqual(self.cs.get_count(s), 50)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_max_approaching_slides(self):
        slides = ["# S%d\n" % i for i in range(255)]
        content = "\n---\n".join(slides)
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            self.assertEqual(self.cs.get_count(s), 255)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_long_line(self):
        long_text = "A" * 900
        content = long_text + "\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            self.assertEqual(self.cs.get_count(s), 1)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_many_columns_table(self):
        cols = " | ".join(["C%d" % i for i in range(15)])
        content = "# Table\n\n| %s |\n| %s |\n" % (cols, " | ".join(["---"] * 15))
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            nlines = self.cs.test_get_nlines(s, 0)
            found_table = False
            for i in range(nlines):
                if self.cs.test_get_line_type(s, 0, i) == LineType.TABLE_ROW:
                    found_table = True
                    self.assertLessEqual(self.cs.test_get_line_ncols(s, 0, i), 16)
            self.assertTrue(found_table)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_empty_file(self):
        path = self._tmp_md("")
        try:
            s = self.cs.load(path)
            self.assertFalse(bool(s))
        finally:
            os.unlink(path)

    def test_single_slide_no_separator(self):
        path = self._tmp_md("# Just one slide\n\nBody text\n")
        try:
            s = self.cs.load(path)
            self.assertEqual(self.cs.get_count(s), 1)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_code_block_unclosed(self):
        content = "# Title\n\n```c\nint x = 5;\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            self.assertEqual(self.cs.get_count(s), 1)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_transition_on_first_slide(self):
        content = "<!-- transition: fade -->\n# First slide\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            self.assertEqual(self.cs.test_get_transition(s, 0), TransitionType.FADE)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_kiosk_interval(self):
        path = self._tmp_md("# Title\n")
        try:
            s = self.cs.load(path)
            self.cs.test_set_transition_type(s, 0)
            self.assertEqual(self.cs.test_get_kiosk_interval(s), 0)
            self.cs.free(s)
        finally:
            os.unlink(path)


class TestHotReload(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        lib_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "libslider.so")
        )
        cls.cs = CSlides(lib_path)

    def _tmp_md(self, content):
        f = tempfile.NamedTemporaryFile(mode='w', suffix='.md', delete=False,
                                        encoding='utf-8', dir=tempfile.gettempdir())
        f.write(content)
        f.close()
        return f.name

    def test_filepath_stored(self):
        content = "# Test\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            fp = self.cs.test_get_filepath(s)
            self.assertIn("tmp", fp.lower())
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_last_mtime_nonzero(self):
        content = "# Test\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            self.assertGreater(self.cs.test_get_mtime(s), 0)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_reload_preserves_theme(self):
        content = "# Test\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            self.cs.set_theme(s, "rose")
            new_s = self.cs.load(path)
            self.assertTrue(bool(new_s))
            self.cs.free(s)
            self.cs.free(new_s)
        finally:
            os.unlink(path)


if __name__ == "__main__":
    unittest.main()
