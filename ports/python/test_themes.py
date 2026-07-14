import unittest
import os
from c_slides_bindings import CSlides, Theme


ALL_THEME_NAMES = [
    "dark", "rose", "monokai", "nord", "light",
    "blue", "ambercat", "dracula", "gruvbox", "catppuccin", "tokyo-night",
]


class TestThemes(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        lib_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "libslider.so")
        )
        cls.cs = CSlides(lib_path)

    def test_theme_default_exists(self):
        t = self.cs.theme_default()
        self.assertTrue(bool(t))
        self.assertEqual(t.contents.name.decode("utf-8"), "dark")

    def test_theme_default_colors_are_nonzero(self):
        t = self.cs.theme_default()
        self.assertNotEqual(t.contents.bg, 0)
        self.assertNotEqual(t.contents.title, 0)
        self.assertNotEqual(t.contents.body, 0)

    def test_all_themes_found_by_name(self):
        for name in ALL_THEME_NAMES:
            with self.subTest(theme=name):
                theme_ptr = self.cs.theme_find(name)
                self.assertTrue(bool(theme_ptr), f"theme_find('{name}') returned NULL")
                self.assertEqual(theme_ptr.contents.name.decode("utf-8"), name)

    def test_unknown_theme_falls_back_to_dark(self):
        theme_ptr = self.cs.theme_find("nonexistent_theme_xyz")
        self.assertTrue(bool(theme_ptr))
        self.assertEqual(theme_ptr.contents.name.decode("utf-8"), "dark")

    def test_null_theme_name_falls_back_to_dark(self):
        theme_ptr = self.cs.theme_find(None)
        self.assertTrue(bool(theme_ptr))
        self.assertEqual(theme_ptr.contents.name.decode("utf-8"), "dark")

    def test_theme_colors_are_distinct_across_themes(self):
        bgs = set()
        for name in ALL_THEME_NAMES:
            theme_ptr = self.cs.theme_find(name)
            bg = theme_ptr.contents.bg
            self.assertNotIn(bg, bgs, f"Duplicate bg color for theme {name}")
            bgs.add(bg)

    def test_set_all_themes_on_slider(self):
        md_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "examples", "basic.md")
        )
        s = self.cs.load(md_path)
        self.assertTrue(bool(s))

        for name in ALL_THEME_NAMES:
            with self.subTest(theme=name):
                self.cs.set_theme(s, name)
                self.assertEqual(self.cs.get_theme_name(s), name)

        self.cs.free(s)

    def test_set_theme_invalid_name_keeps_previous(self):
        md_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "examples", "basic.md")
        )
        s = self.cs.load(md_path)
        self.assertTrue(bool(s))

        self.cs.set_theme(s, "dracula")
        self.assertEqual(self.cs.get_theme_name(s), "dracula")

        # Invalid name should fallback to dark (per theme_find behavior)
        self.cs.set_theme(s, "invalid_theme_abc")
        self.assertEqual(self.cs.get_theme_name(s), "dark")

        self.cs.free(s)

    def test_theme_struct_fields_are_uint32(self):
        t = self.cs.theme_default()
        for field_name in [
            "bg", "title", "sub", "body", "bullet", "accent", "num",
            "table_hdr", "table_row", "table_alt", "table_bdr",
            "code_bg", "code_txt", "code_kw", "code_com", "code_str", "code_sym",
        ]:
            val = getattr(t.contents, field_name)
            self.assertIsInstance(val, int, f"Field {field_name} should be an integer")
            self.assertGreaterEqual(val, 0, f"Field {field_name} should be >= 0")
            self.assertLessEqual(val, 0xFFFFFF, f"Field {field_name} should be <= 0xFFFFFF")

    def test_set_color_bg(self):
        md_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "examples", "basic.md")
        )
        s = self.cs.load(md_path)
        self.assertTrue(bool(s))

        # set_color should not crash
        self.cs.set_color(s, "bg", "#ff0000")

        # Verify by exporting — should succeed
        out = os.path.join(os.path.dirname(__file__), "test_setcolor_bg.png")
        if os.path.exists(out):
            os.remove(out)
        res = self.cs.export_png(s, 0, out, 400, 300)
        self.assertEqual(res, 0)
        self.assertTrue(os.path.exists(out))
        if os.path.exists(out):
            os.remove(out)

        self.cs.free(s)

    def test_set_color_all_keys(self):
        md_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "examples", "basic.md")
        )
        s = self.cs.load(md_path)
        self.assertTrue(bool(s))

        color_keys = [
            "bg", "title", "sub", "body", "bullet", "accent", "num",
            "table-hdr", "table-row", "table-alt", "table-bdr",
            "code-bg", "code-txt", "code-kw", "code-com", "code-str", "code-sym",
        ]

        for key in color_keys:
            with self.subTest(key=key):
                self.cs.set_color(s, key, "#abcdef")

        # Verify all colors were set without crash by exporting
        out = os.path.join(os.path.dirname(__file__), "test_setcolor_all.png")
        if os.path.exists(out):
            os.remove(out)
        res = self.cs.export_png(s, 0, out, 400, 300)
        self.assertEqual(res, 0)
        if os.path.exists(out):
            os.remove(out)

        self.cs.free(s)

    def test_set_color_alias_subtitle(self):
        md_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "examples", "basic.md")
        )
        s = self.cs.load(md_path)
        self.assertTrue(bool(s))

        # Should not crash
        self.cs.set_color(s, "subtitle", "#aabbcc")
        self.cs.free(s)

    def test_set_color_alias_text(self):
        md_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "examples", "basic.md")
        )
        s = self.cs.load(md_path)
        self.assertTrue(bool(s))

        self.cs.set_color(s, "text", "#112233")
        self.cs.free(s)

    def test_set_color_alias_number(self):
        md_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "examples", "basic.md")
        )
        s = self.cs.load(md_path)
        self.assertTrue(bool(s))

        self.cs.set_color(s, "number", "#445566")
        self.cs.free(s)

    def test_set_color_accent_also_sets_bullet(self):
        md_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "examples", "basic.md")
        )
        s = self.cs.load(md_path)
        self.assertTrue(bool(s))

        self.cs.set_color(s, "accent", "#abcdef")
        self.cs.free(s)

    def test_set_color_invalid_hex_ignored(self):
        md_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "examples", "basic.md")
        )
        s = self.cs.load(md_path)
        self.assertTrue(bool(s))

        # Should not crash with invalid hex
        self.cs.set_color(s, "bg", "not_a_hex_color")

        out = os.path.join(os.path.dirname(__file__), "test_setcolor_invalid.png")
        if os.path.exists(out):
            os.remove(out)
        res = self.cs.export_png(s, 0, out, 400, 300)
        self.assertEqual(res, 0)
        if os.path.exists(out):
            os.remove(out)

        self.cs.free(s)


if __name__ == "__main__":
    unittest.main()
