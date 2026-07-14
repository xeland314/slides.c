import unittest
import os
from c_slides_bindings import CSlides


class TestLexerHTML(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        lib_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "libslider.so")
        )
        cls.cs = CSlides(lib_path)

    def setUp(self):
        self.theme_ptr = self.cs.theme_default()

    def _lex(self, line):
        return self.cs.highlight_lang(line, "html", self.theme_ptr)

    def _assert_highlighted(self, out):
        self.assertIn("<span foreground=", out)

    # ── Opening tags ──────────────────────────────────────────────────────

    def test_tag_div(self):
        out = self._lex('<div class="main">')
        self.assertIn("div", out)
        self._assert_highlighted(out)

    def test_tag_h1(self):
        out = self._lex("<h1>Title</h1>")
        self.assertIn("h1", out)
        self._assert_highlighted(out)

    def test_tag_p(self):
        out = self._lex("<p>Text</p>")
        self.assertIn("p", out)
        self._assert_highlighted(out)

    def test_tag_span(self):
        out = self._lex('<span id="foo">bar</span>')
        self.assertIn("span", out)
        self._assert_highlighted(out)

    def test_tag_a(self):
        out = self._lex('<a href="url">Link</a>')
        self.assertIn("a", out)
        self._assert_highlighted(out)

    def test_tag_img_self_closing(self):
        out = self._lex('<img src="photo.jpg" alt="Photo" />')
        self.assertIn("img", out)
        self._assert_highlighted(out)

    def test_tag_br(self):
        out = self._lex("<br />")
        self.assertIn("br", out)
        self._assert_highlighted(out)

    def test_tag_meta(self):
        out = self._lex('<meta charset="UTF-8" />')
        self.assertIn("meta", out)
        self._assert_highlighted(out)

    def test_tag_link(self):
        out = self._lex('<link rel="stylesheet" href="style.css" />')
        self.assertIn("link", out)
        self._assert_highlighted(out)

    def test_tag_script(self):
        out = self._lex("<script>")
        self.assertIn("script", out)
        self._assert_highlighted(out)

    def test_tag_style(self):
        out = self._lex("<style>")
        self.assertIn("style", out)
        self._assert_highlighted(out)

    # ── Closing tags ─────────────────────────────────────────────────────

    def test_close_div(self):
        out = self._lex("</div>")
        self.assertIn("div", out)
        self._assert_highlighted(out)

    def test_close_p(self):
        out = self._lex("</p>")
        self.assertIn("p", out)
        self._assert_highlighted(out)

    def test_close_span(self):
        out = self._lex("</span>")
        self.assertIn("span", out)
        self._assert_highlighted(out)

    def test_close_script(self):
        out = self._lex("</script>")
        self.assertIn("script", out)
        self._assert_highlighted(out)

    # ── Attributes ───────────────────────────────────────────────────────

    def test_attribute_class(self):
        out = self._lex('<div class="container">')
        self.assertIn("class", out)
        self._assert_highlighted(out)

    def test_attribute_id(self):
        out = self._lex('<div id="main">')
        self.assertIn("id", out)
        self._assert_highlighted(out)

    def test_attribute_href(self):
        out = self._lex('<a href="https://example.com">')
        self.assertIn("href", out)
        self._assert_highlighted(out)

    def test_attribute_src(self):
        out = self._lex('<img src="image.png" />')
        self.assertIn("src", out)
        self._assert_highlighted(out)

    def test_attribute_alt(self):
        out = self._lex('<img alt="description" />')
        self.assertIn("alt", out)
        self._assert_highlighted(out)

    def test_attribute_style(self):
        out = self._lex('<div style="color: red;">')
        self.assertIn("style", out)
        self._assert_highlighted(out)

    def test_attribute_value(self):
        out = self._lex('<input type="text" value="hello" />')
        self.assertIn("value", out)
        self._assert_highlighted(out)

    def test_attribute_data_attr(self):
        out = self._lex('<div data-id="123">')
        self.assertIn("data-id", out)
        self._assert_highlighted(out)

    def test_attribute_aria_attr(self):
        out = self._lex('<button aria-label="Close">X</button>')
        self.assertIn("aria-label", out)
        self._assert_highlighted(out)

    def test_attribute_disabled(self):
        out = self._lex('<input disabled />')
        self.assertIn("disabled", out)
        self._assert_highlighted(out)

    def test_attribute_checked(self):
        out = self._lex('<input checked />')
        self.assertIn("checked", out)
        self._assert_highlighted(out)

    # ── Attribute values ─────────────────────────────────────────────────

    def test_attribute_value_double_quote(self):
        out = self._lex('<div class="my-class">')
        self.assertIn("my-class", out)
        self._assert_highlighted(out)

    def test_attribute_value_single_quote(self):
        out = self._lex("<div class='my-class'>")
        self.assertIn("my-class", out)
        self._assert_highlighted(out)

    def test_attribute_value_with_dashes(self):
        out = self._lex('<div data-component="my-widget">')
        self.assertIn("my-widget", out)
        self._assert_highlighted(out)

    def test_attribute_multiple(self):
        out = self._lex('<input type="email" required />')
        self.assertIn("type", out)
        self.assertIn("required", out)
        self._assert_highlighted(out)

    # ── Comments ─────────────────────────────────────────────────────────

    def test_comment_simple(self):
        out = self._lex("<!-- comment -->")
        self.assertIn("comment", out)
        self._assert_highlighted(out)

    def test_comment_multiline(self):
        out = self._lex("<!-- line one\nline two -->")
        self.assertIn("line one", out)
        self._assert_highlighted(out)

    def test_comment_with_dashes(self):
        out = self._lex("<!-- a--b -->")
        self.assertIn("a--b", out)
        self._assert_highlighted(out)

    def test_comment_empty(self):
        out = self._lex("<!---->")
        self._assert_highlighted(out)

    # ── DOCTYPE ──────────────────────────────────────────────────────────

    def test_doctype_html(self):
        out = self._lex("<!DOCTYPE html>")
        self.assertIn("DOCTYPE", out)
        self.assertIn("html", out)
        self._assert_highlighted(out)

    def test_doctype_lowercase(self):
        out = self._lex("<!doctype html>")
        self.assertIn("doctype", out)
        self._assert_highlighted(out)

    # ── Entities ─────────────────────────────────────────────────────────

    def test_entity_amp(self):
        out = self._lex("&amp;")
        self.assertIn("amp", out)
        self._assert_highlighted(out)

    def test_entity_lt(self):
        out = self._lex("&lt;")
        self.assertIn("lt", out)
        self._assert_highlighted(out)

    def test_entity_gt(self):
        out = self._lex("&gt;")
        self.assertIn("gt", out)
        self._assert_highlighted(out)

    def test_entity_nbsp(self):
        out = self._lex("&nbsp;")
        self.assertIn("nbsp", out)
        self._assert_highlighted(out)

    # ── Text content ─────────────────────────────────────────────────────

    def test_text_plain(self):
        out = self._lex("Hello World")
        self.assertIn("Hello World", out)

    def test_text_with_special_chars(self):
        out = self._lex("<p>Hello & goodbye</p>")
        self.assertIn("p", out)
        self.assertIn("goodbye", out)

    # ── Complex HTML ─────────────────────────────────────────────────────

    def test_nested_tags(self):
        out = self._lex('<div><p>Hello</p></div>')
        self.assertIn("div", out)
        self.assertIn("p", out)
        self._assert_highlighted(out)

    def test_full_element(self):
        out = self._lex('<a href="https://example.com" target="_blank" rel="noopener">Click</a>')
        self.assertIn("a", out)
        self.assertIn("href", out)
        self.assertIn("target", out)
        self.assertIn("https://example.com", out)
        self._assert_highlighted(out)

    def test_form_element(self):
        out = self._lex('<input type="text" placeholder="Enter name" required />')
        self.assertIn("input", out)
        self.assertIn("type", out)
        self.assertIn("placeholder", out)
        self.assertIn("Enter name", out)
        self._assert_highlighted(out)

    def test_table_element(self):
        out = self._lex('<td colspan="2" rowspan="1">Cell</td>')
        self.assertIn("td", out)
        self.assertIn("colspan", out)
        self.assertIn("rowspan", out)
        self._assert_highlighted(out)

    def test_unknown_tag(self):
        out = self._lex('<my-component>content</my-component>')
        self.assertIn("my-component", out)
        self._assert_highlighted(out)

    def test_lexer_dispatch_html(self):
        out = self.cs.highlight_lang('<div class="x">', "html", self.theme_ptr)
        self.assertIn("div", out)
        self._assert_highlighted(out)

    def test_lexer_dispatch_html_from_bindings(self):
        out = self.cs.highlight_lang("<br />", "html", self.theme_ptr)
        self.assertIn("br", out)
        self._assert_highlighted(out)

    def test_empty_line(self):
        out = self._lex("")
        self.assertEqual(out, "")

    def test_angle_brackets_alone(self):
        out = self._lex("< >")
        self._assert_highlighted(out)


if __name__ == "__main__":
    unittest.main()
