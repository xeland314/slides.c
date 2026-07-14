import unittest
import os
import tempfile
import ctypes
from c_slides_bindings import CSlides, LineType, TransitionType
from cairo_helpers import create_image_surface, create_context, destroy_context, destroy_surface


class TestTransitions(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        lib_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "libslider.so")
        )
        cls.cs = CSlides(lib_path)

    def _example(self, name):
        return os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "examples", name)
        )

    def _tmp_md(self, content):
        f = tempfile.NamedTemporaryFile(mode='w', suffix='.md', delete=False,
                                        encoding='utf-8', dir=tempfile.gettempdir())
        f.write(content)
        f.close()
        return f.name

    def test_transition_types_parsed(self):
        path = self._example("transitions.md")
        s = self.cs.load(path)
        self.assertTrue(bool(s))
        count = self.cs.get_count(s)
        self.assertGreaterEqual(count, 7)
        self.assertEqual(self.cs.test_get_transition(s, 1), TransitionType.FADE)
        self.assertEqual(self.cs.test_get_transition(s, 2), TransitionType.SLIDE_LEFT)
        self.assertEqual(self.cs.test_get_transition(s, 3), TransitionType.SLIDE_RIGHT)
        self.assertEqual(self.cs.test_get_transition(s, 4), TransitionType.SLIDE_UP)
        self.assertEqual(self.cs.test_get_transition(s, 5), TransitionType.SLIDE_DOWN)
        self.assertEqual(self.cs.test_get_transition(s, 6), TransitionType.NONE)
        self.cs.free(s)

    def test_do_transition_fade(self):
        content = "# Slide 1\n\n---\n\n<!-- transition: fade -->\n\n# Slide 2\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            self.assertTrue(bool(s))
            self.assertEqual(self.cs.test_get_transition(s, 1), TransitionType.FADE)
            sfc = create_image_surface(100, 100)
            cr = create_context(sfc)
            self.cs.do_transition(s, 0, 1, cr, 100, 100, 0.5)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_do_transition_slide_left(self):
        content = "# Slide 1\n\n---\n\n<!-- transition: slide-left -->\n\n# Slide 2\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            self.assertTrue(bool(s))
            sfc = create_image_surface(200, 100)
            cr = create_context(sfc)
            self.cs.do_transition(s, 0, 1, cr, 200, 100, 0.5)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_do_transition_slide_right(self):
        content = "# S1\n\n---\n\n<!-- transition: slide-right -->\n\n# S2\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            sfc = create_image_surface(200, 100)
            cr = create_context(sfc)
            self.cs.do_transition(s, 0, 1, cr, 200, 100, 0.5)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_do_transition_slide_up(self):
        content = "# S1\n\n---\n\n<!-- transition: slide-up -->\n\n# S2\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            sfc = create_image_surface(200, 100)
            cr = create_context(sfc)
            self.cs.do_transition(s, 0, 1, cr, 200, 100, 0.5)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_do_transition_slide_down(self):
        content = "# S1\n\n---\n\n<!-- transition: slide-down -->\n\n# S2\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            sfc = create_image_surface(200, 100)
            cr = create_context(sfc)
            self.cs.do_transition(s, 0, 1, cr, 200, 100, 0.5)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_do_transition_progress_zero(self):
        content = "# S1\n\n---\n\n<!-- transition: fade -->\n\n# S2\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            sfc = create_image_surface(100, 100)
            cr = create_context(sfc)
            self.cs.do_transition(s, 0, 1, cr, 100, 100, 0.0)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_do_transition_progress_one(self):
        content = "# S1\n\n---\n\n<!-- transition: fade -->\n\n# S2\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            sfc = create_image_surface(100, 100)
            cr = create_context(sfc)
            self.cs.do_transition(s, 0, 1, cr, 100, 100, 1.0)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_do_transition_same_index_returns(self):
        content = "# S1\n\n---\n\n# S2\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            sfc = create_image_surface(100, 100)
            cr = create_context(sfc)
            self.cs.do_transition(s, 0, 0, cr, 100, 100, 0.5)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_do_transition_invalid_from(self):
        content = "# S1\n\n---\n\n# S2\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            sfc = create_image_surface(100, 100)
            cr = create_context(sfc)
            self.cs.do_transition(s, -1, 1, cr, 100, 100, 0.5)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_do_transition_all_progress_values(self):
        content = "# S1\n\n---\n\n<!-- transition: fade -->\n\n# S2\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            sfc = create_image_surface(100, 100)
            cr = create_context(sfc)
            for p in [0.0, 0.25, 0.5, 0.75, 1.0]:
                self.cs.do_transition(s, 0, 1, cr, 100, 100, p)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_render_slide_with_transition_at_midpoint(self):
        content = "# Slide A\n\n---\n\n<!-- transition: slide-left -->\n\n# Slide B\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            self.cs.test_set_transition_from(s, 0)
            self.cs.test_set_transition_type(s, TransitionType.SLIDE_LEFT)
            sfc = create_image_surface(400, 300)
            cr = create_context(sfc)
            self.cs.render_slide(s, 1, cr, 400, 300, 150.0)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_render_slide_with_transition_expired(self):
        content = "# Slide A\n\n---\n\n<!-- transition: fade -->\n\n# Slide B\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            self.cs.test_set_transition_from(s, 0)
            self.cs.test_set_transition_type(s, TransitionType.FADE)
            sfc = create_image_surface(400, 300)
            cr = create_context(sfc)
            self.cs.render_slide(s, 1, cr, 400, 300, 500.0)
            self.assertEqual(self.cs.test_get_transition_type(s), TransitionType.NONE)
            destroy_context(cr)
            destroy_surface(sfc)
            self.cs.free(s)
        finally:
            os.unlink(path)

    def test_transition_export_png_triggers(self):
        content = "# S1\n\n---\n\n<!-- transition: fade -->\n\n# S2\n"
        path = self._tmp_md(content)
        try:
            s = self.cs.load(path)
            out = os.path.join(tempfile.gettempdir(), "trans_test.png")
            ret = self.cs.export_png(s, 1, out, 100, 100)
            self.assertEqual(ret, 0)
            self.assertTrue(os.path.exists(out))
            os.unlink(out)
            self.cs.free(s)
        finally:
            os.unlink(path)


if __name__ == "__main__":
    unittest.main()
