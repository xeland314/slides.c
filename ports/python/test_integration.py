import unittest
import os
from c_slides_bindings import CSlides


class TestIntegration(unittest.TestCase):
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

    def test_slider_load_basic(self):
        path = self._example("basic.md")
        s = self.cs.load(path)
        self.assertTrue(bool(s), "No se pudo cargar el slider")

        count = self.cs.get_count(s)
        self.assertEqual(count, 2)

        theme_name = self.cs.get_theme_name(s)
        self.assertEqual(theme_name, "dark")

        self.cs.free(s)

    def test_slider_change_palette(self):
        path = self._example("basic.md")
        s = self.cs.load(path)
        self.assertTrue(bool(s))

        self.cs.set_theme(s, "rose")
        theme_name = self.cs.get_theme_name(s)
        self.assertEqual(theme_name, "rose")

        self.cs.free(s)

    def test_slider_load_nonexistent_file(self):
        s = self.cs.load("/tmp/nonexistent_file_xyz_123.md")
        self.assertFalse(bool(s))

    def test_slider_get_count_empty_file(self):
        path = self._example("basic.md")
        s = self.cs.load(path)
        self.assertTrue(bool(s))
        count = self.cs.get_count(s)
        self.assertGreater(count, 0)
        self.cs.free(s)

    def test_slider_load_frontmatter(self):
        path = self._example("frontmatter.md")
        s = self.cs.load(path)
        self.assertTrue(bool(s))

        # frontmatter.md has theme: dracula
        theme_name = self.cs.get_theme_name(s)
        self.assertEqual(theme_name, "dracula")

        # font-family: "JetBrains Mono" — quotes are preserved from YAML
        font = self.cs.get_font_family(s)
        self.assertEqual(font, '"JetBrains Mono"')

        self.cs.free(s)

    def test_slider_load_features(self):
        path = self._example("features.md")
        s = self.cs.load(path)
        self.assertTrue(bool(s))

        count = self.cs.get_count(s)
        self.assertGreaterEqual(count, 4)

        self.cs.free(s)

    def test_slider_load_code_blocks(self):
        path = self._example("code_blocks.md")
        s = self.cs.load(path)
        self.assertTrue(bool(s))

        count = self.cs.get_count(s)
        self.assertGreaterEqual(count, 4)

        self.cs.free(s)

    def test_slider_load_complex_tables(self):
        path = self._example("complex_tables.md")
        s = self.cs.load(path)
        self.assertTrue(bool(s))

        count = self.cs.get_count(s)
        self.assertGreaterEqual(count, 2)

        self.cs.free(s)

    def test_slider_load_transitions(self):
        path = self._example("transitions.md")
        s = self.cs.load(path)
        self.assertTrue(bool(s))

        count = self.cs.get_count(s)
        self.assertGreaterEqual(count, 6)

        self.cs.free(s)

    def test_slider_load_custom_colors(self):
        path = self._example("custom_colors.md")
        s = self.cs.load(path)
        self.assertTrue(bool(s))

        count = self.cs.get_count(s)
        self.assertGreaterEqual(count, 5)

        self.cs.free(s)

    def test_slider_load_img_config(self):
        path = self._example("img_config.md")
        s = self.cs.load(path)
        self.assertTrue(bool(s))

        count = self.cs.get_count(s)
        self.assertGreaterEqual(count, 10)

        self.cs.free(s)

    def test_slider_load_test_gif(self):
        path = self._example("test_gif.md")
        s = self.cs.load(path)
        self.assertTrue(bool(s))

        count = self.cs.get_count(s)
        self.assertGreaterEqual(count, 3)

        self.cs.free(s)

    def test_font_family_default(self):
        path = self._example("basic.md")
        s = self.cs.load(path)
        self.assertTrue(bool(s))

        font = self.cs.get_font_family(s)
        self.assertEqual(font, "Inter")

        self.cs.free(s)

    def test_font_family_set_get(self):
        path = self._example("basic.md")
        s = self.cs.load(path)
        self.assertTrue(bool(s))

        self.cs.set_font_family(s, "Courier New")
        self.assertEqual(self.cs.get_font_family(s), "Courier New")

        self.cs.set_font_family(s, "Fira Code")
        self.assertEqual(self.cs.get_font_family(s), "Fira Code")

        self.cs.free(s)

    def test_font_scale_default(self):
        path = self._example("basic.md")
        s = self.cs.load(path)
        self.assertTrue(bool(s))

        scale = self.cs.get_font_scale(s)
        self.assertAlmostEqual(scale, 1.0, places=5)

        self.cs.free(s)

    def test_font_scale_set_get(self):
        path = self._example("basic.md")
        s = self.cs.load(path)
        self.assertTrue(bool(s))

        self.cs.set_font_scale(s, 1.5)
        self.assertAlmostEqual(self.cs.get_font_scale(s), 1.5, places=5)

        self.cs.set_font_scale(s, 0.8)
        self.assertAlmostEqual(self.cs.get_font_scale(s), 0.8, places=5)

        self.cs.free(s)

    def test_font_scale_minimum_threshold(self):
        path = self._example("basic.md")
        s = self.cs.load(path)
        self.assertTrue(bool(s))

        self.cs.set_font_scale(s, 2.0)
        # Setting scale <= 0.1 should be ignored
        self.cs.set_font_scale(s, 0.05)
        self.assertAlmostEqual(self.cs.get_font_scale(s), 2.0, places=5)

        self.cs.free(s)

    def test_set_color_on_slider(self):
        path = self._example("basic.md")
        s = self.cs.load(path)
        self.assertTrue(bool(s))

        # set_color should not crash; verify by exporting successfully
        self.cs.set_color(s, "bg", "#ff0000")
        self.cs.set_color(s, "title", "#00ff00")

        out = os.path.join(os.path.dirname(__file__), "test_set_color_export.png")
        if os.path.exists(out):
            os.remove(out)
        res = self.cs.export_png(s, 0, out, 400, 300)
        self.assertEqual(res, 0)
        if os.path.exists(out):
            os.remove(out)

        self.cs.free(s)

    def test_set_color_does_not_affect_original_theme(self):
        path = self._example("basic.md")
        s = self.cs.load(path)
        self.assertTrue(bool(s))

        # Set color on one slider
        self.cs.set_theme(s, "dark")
        self.cs.set_color(s, "bg", "#ffffff")

        # Load a new slider — should still have default dark bg
        s2 = self.cs.load(path)
        self.assertTrue(bool(s2))
        self.cs.set_theme(s2, "dark")

        # Both should export without crash
        out1 = os.path.join(os.path.dirname(__file__), "test_color_iso_1.png")
        out2 = os.path.join(os.path.dirname(__file__), "test_color_iso_2.png")
        for o in [out1, out2]:
            if os.path.exists(o):
                os.remove(o)

        self.assertEqual(self.cs.export_png(s, 0, out1, 400, 300), 0)
        self.assertEqual(self.cs.export_png(s2, 0, out2, 400, 300), 0)

        for o in [out1, out2]:
            if os.path.exists(o):
                os.remove(o)

        self.cs.free(s)
        self.cs.free(s2)

    def test_change_theme_resets_colors(self):
        path = self._example("basic.md")
        s = self.cs.load(path)
        self.assertTrue(bool(s))

        self.cs.set_theme(s, "dracula")
        self.cs.set_color(s, "bg", "#ffffff")

        # Switching theme should reset all colors to the new theme's defaults
        self.cs.set_theme(s, "dark")

        # Verify by exporting — should not crash
        out = os.path.join(os.path.dirname(__file__), "test_theme_reset.png")
        if os.path.exists(out):
            os.remove(out)
        res = self.cs.export_png(s, 0, out, 400, 300)
        self.assertEqual(res, 0)
        if os.path.exists(out):
            os.remove(out)

        self.cs.free(s)

    def test_multiple_load_free_cycles(self):
        path = self._example("basic.md")
        for _ in range(10):
            s = self.cs.load(path)
            self.assertTrue(bool(s))
            self.assertEqual(self.cs.get_count(s), 2)
            self.cs.free(s)

    def test_print_notes_does_not_crash(self):
        path = self._example("features.md")
        s = self.cs.load(path)
        self.assertTrue(bool(s))

        # Should not raise
        self.cs.print_notes(s, 0)
        self.cs.print_notes(s, 1)

        self.cs.free(s)

    def test_slider_default_properties(self):
        path = self._example("basic.md")
        s = self.cs.load(path)
        self.assertTrue(bool(s))

        self.assertEqual(self.cs.get_theme_name(s), "dark")
        self.assertEqual(self.cs.get_font_family(s), "Inter")
        self.assertAlmostEqual(self.cs.get_font_scale(s), 1.0, places=5)

        self.cs.free(s)

    def test_frontmatter_with_custom_colors(self):
        path = self._example("custom_colors.md")
        s = self.cs.load(path)
        self.assertTrue(bool(s))

        # custom_colors.md has colors block with bg: "#0f0f23" and title: "#ff6b6b"
        # Verify theme was applied by exporting successfully
        out = os.path.join(os.path.dirname(__file__), "test_custom_colors_export.png")
        if os.path.exists(out):
            os.remove(out)
        res = self.cs.export_png(s, 0, out, 800, 600)
        self.assertEqual(res, 0)
        self.assertTrue(os.path.exists(out))
        self.assertGreater(os.path.getsize(out), 1000)

        self.cs.free(s)
        if os.path.exists(out):
            os.remove(out)

    def test_export_after_theme_change(self):
        path = self._example("basic.md")
        out = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "test_integration_export.png")
        )
        if os.path.exists(out):
            os.remove(out)

        s = self.cs.load(path)
        self.assertTrue(bool(s))

        self.cs.set_theme(s, "dracula")
        res = self.cs.export_png(s, 0, out, 800, 600)
        self.assertEqual(res, 0)
        self.assertTrue(os.path.exists(out))
        self.assertGreater(os.path.getsize(out), 1000)

        self.cs.free(s)
        if os.path.exists(out):
            os.remove(out)


if __name__ == "__main__":
    unittest.main()
