import unittest
import os
from c_slides_bindings import CSlides


class TestLexerCSS(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        lib_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "libslider.so")
        )
        cls.cs = CSlides(lib_path)

    def setUp(self):
        self.theme_ptr = self.cs.theme_default()

    def _lex(self, line):
        return self.cs.highlight_lang(line, "css", self.theme_ptr)

    def _assert_highlighted(self, out):
        self.assertIn("<span foreground=", out)

    # ── Selectors ────────────────────────────────────────────────────────

    def test_class_selector(self):
        out = self._lex(".container { color: red; }")
        self.assertIn(".container", out)
        self._assert_highlighted(out)

    def test_id_selector(self):
        out = self._lex("#main { margin: 0; }")
        self.assertIn("#main", out)
        self._assert_highlighted(out)

    def test_tag_selector(self):
        out = self._lex("h1 { font-size: 2rem; }")
        self.assertIn("h1", out)
        self._assert_highlighted(out)

    def test_pseudo_class(self):
        out = self._lex("button:hover { background: blue; }")
        self.assertIn(":hover", out)
        self._assert_highlighted(out)

    def test_pseudo_element(self):
        out = self._lex("p::first-line { font-weight: bold; }")
        self.assertIn("first-line", out)
        self._assert_highlighted(out)

    def test_universal_selector(self):
        out = self._lex("* { margin: 0; padding: 0; }")
        self._assert_highlighted(out)

    def test_child_combinator(self):
        out = self._lex(".parent > .child { display: block; }")
        self.assertIn(">", out)
        self._assert_highlighted(out)

    def test_adjacent_combinator(self):
        out = self._lex("h1 + p { margin-top: 0; }")
        self.assertIn("+", out)
        self._assert_highlighted(out)

    def test_sibling_combinator(self):
        out = self._lex("h1 ~ p { color: gray; }")
        self.assertIn("~", out)
        self._assert_highlighted(out)

    def test_descendant_selector(self):
        out = self._lex(".parent .child { color: blue; }")
        self.assertIn(".parent", out)
        self.assertIn(".child", out)
        self._assert_highlighted(out)

    def test_comma_separated(self):
        out = self._lex("h1, h2, h3 { font-weight: bold; }")
        self.assertIn("h1", out)
        self.assertIn("h2", out)
        self.assertIn("h3", out)
        self._assert_highlighted(out)

    def test_complex_selector(self):
        out = self._lex("nav ul li a:hover { color: blue; }")
        self.assertIn("nav", out)
        self.assertIn("ul", out)
        self.assertIn("li", out)
        self.assertIn("a", out)
        self._assert_highlighted(out)

    # ── Properties ───────────────────────────────────────────────────────

    def test_property_color(self):
        out = self._lex("color: red;")
        self.assertIn("color", out)
        self._assert_highlighted(out)

    def test_property_margin(self):
        out = self._lex("margin: 10px;")
        self.assertIn("margin", out)
        self._assert_highlighted(out)

    def test_property_display(self):
        out = self._lex("display: flex;")
        self.assertIn("display", out)
        self._assert_highlighted(out)

    def test_property_font_size(self):
        out = self._lex("font-size: 16px;")
        self.assertIn("font-size", out)
        self._assert_highlighted(out)

    def test_property_background(self):
        out = self._lex("background-color: #ffffff;")
        self.assertIn("background-color", out)
        self._assert_highlighted(out)

    def test_property_border(self):
        out = self._lex("border: 1px solid black;")
        self.assertIn("border", out)
        self._assert_highlighted(out)

    # ── Values ───────────────────────────────────────────────────────────

    def test_value_hex_color(self):
        out = self._lex("color: #ff0000;")
        self.assertIn("#ff0000", out)
        self._assert_highlighted(out)

    def test_value_hex_shorthand(self):
        out = self._lex("color: #f00;")
        self.assertIn("#f00", out)
        self._assert_highlighted(out)

    def test_value_hex_8digit(self):
        out = self._lex("color: #ff000080;")
        self.assertIn("#ff000080", out)
        self._assert_highlighted(out)

    def test_value_px(self):
        out = self._lex("width: 100px;")
        self.assertIn("100px", out)
        self._assert_highlighted(out)

    def test_value_em(self):
        out = self._lex("font-size: 1.5em;")
        self.assertIn("1.5em", out)
        self._assert_highlighted(out)

    def test_value_rem(self):
        out = self._lex("font-size: 2rem;")
        self.assertIn("2rem", out)
        self._assert_highlighted(out)

    def test_value_percent(self):
        out = self._lex("width: 50%;")
        self.assertIn("50%", out)
        self._assert_highlighted(out)

    def test_value_vh(self):
        out = self._lex("height: 100vh;")
        self.assertIn("100vh", out)
        self._assert_highlighted(out)

    def test_value_vw(self):
        out = self._lex("width: 100vw;")
        self.assertIn("100vw", out)
        self._assert_highlighted(out)

    def test_value_deg(self):
        out = self._lex("transform: rotate(45deg);")
        self.assertIn("45deg", out)
        self._assert_highlighted(out)

    def test_value_ms(self):
        out = self._lex("transition: all 300ms;")
        self.assertIn("300ms", out)
        self._assert_highlighted(out)

    def test_value_negative(self):
        out = self._lex("margin: -10px;")
        self.assertIn("-10px", out)
        self._assert_highlighted(out)

    def test_value_positive_sign(self):
        out = self._lex("margin: +10px;")
        self.assertIn("+10px", out)
        self._assert_highlighted(out)

    def test_value_float(self):
        out = self._lex("line-height: 1.6;")
        self.assertIn("1.6", out)
        self._assert_highlighted(out)

    def test_value_zero(self):
        out = self._lex("margin: 0;")
        self.assertIn("0", out)
        self._assert_highlighted(out)

    # ── Strings ──────────────────────────────────────────────────────────

    def test_string_double_quote(self):
        out = self._lex('content: "Hello World";')
        self.assertIn("Hello World", out)
        self._assert_highlighted(out)

    def test_string_single_quote(self):
        out = self._lex("content: 'Hello World';")
        self.assertIn("Hello World", out)
        self._assert_highlighted(out)

    def test_string_empty(self):
        out = self._lex('content: "";')
        self._assert_highlighted(out)

    # ── At-rules ─────────────────────────────────────────────────────────

    def test_at_media(self):
        out = self._lex("@media (max-width: 768px) { }")
        self.assertIn("@media", out)
        self._assert_highlighted(out)

    def test_at_keyframes(self):
        out = self._lex("@keyframes fade { from { opacity: 0; } }")
        self.assertIn("@keyframes", out)
        self._assert_highlighted(out)

    def test_at_import(self):
        out = self._lex('@import url("styles.css");')
        self.assertIn("@import", out)
        self._assert_highlighted(out)

    def test_at_charset(self):
        out = self._lex('@charset "UTF-8";')
        self.assertIn("@charset", out)
        self._assert_highlighted(out)

    def test_at_root(self):
        out = self._lex(":root { --primary: blue; }")
        self.assertIn(":root", out)
        self._assert_highlighted(out)

    def test_at_font_face(self):
        out = self._lex("@font-face { font-family: 'MyFont'; }")
        self.assertIn("@font-face", out)
        self._assert_highlighted(out)

    # ── Comments ─────────────────────────────────────────────────────────

    def test_comment_simple(self):
        out = self._lex("/* comment */")
        self.assertIn("comment", out)
        self._assert_highlighted(out)

    def test_comment_multiline(self):
        out = self._lex("/* line one\nline two */")
        self.assertIn("line one", out)
        self._assert_highlighted(out)

    def test_comment_empty(self):
        out = self._lex("/**/")
        self._assert_highlighted(out)

    def test_comment_with_asterisks(self):
        out = self._lex("/* *** */")
        self._assert_highlighted(out)

    # ── Important ────────────────────────────────────────────────────────

    def test_important(self):
        out = self._lex("color: red !important;")
        self.assertIn("!important", out)
        self._assert_highlighted(out)

    # ── Bracket / punctuation ────────────────────────────────────────────

    def test_braces(self):
        out = self._lex("{ }")
        self.assertIn("{", out)
        self.assertIn("}", out)
        self._assert_highlighted(out)

    def test_parentheses(self):
        out = self._lex("rgb(255, 0, 0)")
        self.assertIn("(", out)
        self.assertIn(")", out)
        self._assert_highlighted(out)

    def test_semicolon(self):
        out = self._lex("color: red;")
        self.assertIn(";", out)
        self._assert_highlighted(out)

    def test_colon(self):
        out = self._lex("color: red")
        self.assertIn(":", out)
        self._assert_highlighted(out)

    def test_comma(self):
        out = self._lex("h1, h2")
        self.assertIn(",", out)
        self._assert_highlighted(out)

    # ── Complex CSS ──────────────────────────────────────────────────────

    def test_full_rule(self):
        out = self._lex('.container { display: flex; justify-content: center; align-items: center; }')
        self.assertIn(".container", out)
        self.assertIn("display", out)
        self.assertIn("justify-content", out)
        self.assertIn("align-items", out)
        self._assert_highlighted(out)

    def test_media_query_full(self):
        out = self._lex('@media (max-width: 768px) { .sidebar { display: none; } }')
        self.assertIn("@media", out)
        self.assertIn("max-width", out)
        self.assertIn("768px", out)
        self.assertIn(".sidebar", out)
        self.assertIn("display", out)
        self.assertIn("none", out)
        self._assert_highlighted(out)

    def test_keyframes_full(self):
        out = self._lex('@keyframes slide { from { transform: translateX(-100%); } to { transform: translateX(0); } }')
        self.assertIn("@keyframes", out)
        self.assertIn("from", out)
        self.assertIn("to", out)
        self.assertIn("transform", out)
        self.assertIn("-100%", out)
        self._assert_highlighted(out)

    def test_grid_layout(self):
        out = self._lex(".grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 1rem; }")
        self.assertIn("grid", out)
        self.assertIn("display", out)
        self.assertIn("grid-template-columns", out)
        self.assertIn("repeat", out)
        self.assertIn("1fr", out)
        self.assertIn("1rem", out)
        self._assert_highlighted(out)

    def test_animation(self):
        out = self._lex("animation: fade 0.3s ease-in-out infinite;")
        self.assertIn("animation", out)
        self.assertIn("fade", out)
        self.assertIn("0.3s", out)
        self.assertIn("ease-in-out", out)
        self.assertIn("infinite", out)
        self._assert_highlighted(out)

    def test_lexer_dispatch_css(self):
        out = self.cs.highlight_lang(".x { color: red; }", "css", self.theme_ptr)
        self.assertIn(".x", out)
        self._assert_highlighted(out)

    def test_empty_line(self):
        out = self._lex("")
        self.assertEqual(out, "")


if __name__ == "__main__":
    unittest.main()
