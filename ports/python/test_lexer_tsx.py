import unittest
import os
from c_slides_bindings import CSlides


class TestLexerTSX(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        lib_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "libslider.so")
        )
        cls.cs = CSlides(lib_path)

    def setUp(self):
        self.theme_ptr = self.cs.theme_default()

    def _lex(self, line):
        return self.cs.highlight_lang(line, "tsx", self.theme_ptr)

    def _assert_highlighted(self, out):
        self.assertIn("<span foreground=", out)

    # ── TypeScript keywords (inherited) ────────────────────────────────────

    def test_keyword_const(self):
        out = self._lex("const x = 5;")
        self.assertIn("const", out)
        self._assert_highlighted(out)

    def test_keyword_function(self):
        out = self._lex("function hello() {}")
        self.assertIn("function", out)
        self._assert_highlighted(out)

    def test_keyword_type(self):
        out = self._lex("type Props = { name: string };")
        self.assertIn("type", out)
        self._assert_highlighted(out)

    def test_keyword_interface(self):
        out = self._lex("interface Props { name: string; }")
        self.assertIn("interface", out)
        self._assert_highlighted(out)

    def test_keyword_class(self):
        out = self._lex("class Component {}")
        self.assertIn("class", out)
        self._assert_highlighted(out)

    def test_keyword_return(self):
        out = self._lex("return <div />;")
        self.assertIn("return", out)
        self._assert_highlighted(out)

    def test_keyword_async_await(self):
        out = self._lex("async function load() { await fetchData(); }")
        self.assertIn("async", out)
        self.assertIn("await", out)
        self._assert_highlighted(out)

    # ── JSX: opening tags ──────────────────────────────────────────────────

    def test_jsx_opening_tag(self):
        out = self._lex('<div className="box">')
        self.assertIn("div", out)
        self.assertIn("className", out)
        self.assertIn("box", out)
        self._assert_highlighted(out)

    def test_jsx_opening_tag_single_attr(self):
        out = self._lex("<span>text</span>")
        self.assertIn("span", out)
        self._assert_highlighted(out)

    def test_jsx_component_tag(self):
        out = self._lex("<Button onClick={handler}>")
        self.assertIn("Button", out)
        self.assertIn("onClick", out)
        self._assert_highlighted(out)

    def test_jsx_nested_components(self):
        out = self._lex("<App><Header /><Footer /></App>")
        self.assertIn("App", out)
        self.assertIn("Header", out)
        self.assertIn("Footer", out)
        self._assert_highlighted(out)

    # ── JSX: self-closing tags ─────────────────────────────────────────────

    def test_jsx_self_closing(self):
        out = self._lex("<img src={url} />")
        self.assertIn("img", out)
        self.assertIn("src", out)
        self._assert_highlighted(out)

    def test_jsx_self_closing_no_space(self):
        out = self._lex("<br/>")
        self.assertIn("br", out)
        self._assert_highlighted(out)

    def test_jsx_self_closing_with_props(self):
        out = self._lex('<Input type="text" placeholder="Name" />')
        self.assertIn("Input", out)
        self.assertIn("type", out)
        self.assertIn("placeholder", out)
        self._assert_highlighted(out)

    # ── JSX: closing tags ──────────────────────────────────────────────────

    def test_jsx_closing_tag(self):
        out = self._lex("</div>")
        self.assertIn("div", out)
        self._assert_highlighted(out)

    def test_jsx_closing_component(self):
        out = self._lex("</Button>")
        self.assertIn("Button", out)
        self._assert_highlighted(out)

    # ── JSX: content between tags ──────────────────────────────────────────

    def test_jsx_text_content(self):
        out = self._lex("<p>Hello World</p>")
        self.assertIn("p", out)
        self.assertIn("Hello World", out)
        self._assert_highlighted(out)

    def test_jsx_expression_in_content(self):
        out = self._lex("<span>{name}</span>")
        self.assertIn("span", out)
        self.assertIn("name", out)
        self._assert_highlighted(out)

    def test_jsx_mixed_content(self):
        out = self._lex("<div>Hi {user.name}!</div>")
        self.assertIn("div", out)
        self.assertIn("Hi", out)
        self._assert_highlighted(out)

    # ── JSX: attribute values ──────────────────────────────────────────────

    def test_jsx_string_attribute(self):
        out = self._lex('<div id="main">')
        self.assertIn("id", out)
        self.assertIn("main", out)
        self._assert_highlighted(out)

    def test_jsx_single_quote_attribute(self):
        out = self._lex("<div className='active'>")
        self.assertIn("className", out)
        self.assertIn("active", out)
        self._assert_highlighted(out)

    def test_jsx_expression_attribute(self):
        out = self._lex("<div onClick={handleClick}>")
        self.assertIn("onClick", out)
        self.assertIn("handleClick", out)
        self._assert_highlighted(out)

    # ── JSX: complex patterns ──────────────────────────────────────────────

    def test_jsx_full_component(self):
        out = self._lex('const App = () => <div className="app"><h1>Title</h1><p>Content</p></div>;')
        self.assertIn("const", out)
        self.assertIn("div", out)
        self.assertIn("app", out)
        self.assertIn("h1", out)
        self.assertIn("p", out)
        self._assert_highlighted(out)

    def test_jsx_conditional_rendering(self):
        out = self._lex("{isLoggedIn ? <Dashboard /> : <Login />}")
        self.assertIn("isLoggedIn", out)
        self.assertIn("Dashboard", out)
        self.assertIn("Login", out)
        self._assert_highlighted(out)

    def test_jsx_list_rendering(self):
        out = self._lex("{items.map(item => <li key={item.id}>{item.name}</li>)}")
        self.assertIn("items", out)
        self.assertIn("map", out)
        self.assertIn("li", out)
        self.assertIn("key", out)
        self._assert_highlighted(out)

    def test_jsx_fragment(self):
        out = self._lex("<><Header /><Content /></>")
        self.assertIn("Header", out)
        self.assertIn("Content", out)
        self._assert_highlighted(out)

    def test_jsx_deep_nesting(self):
        out = self._lex("<div><section><article><p>Deep</p></article></section></div>")
        self.assertIn("div", out)
        self.assertIn("section", out)
        self.assertIn("article", out)
        self.assertIn("p", out)
        self.assertIn("Deep", out)
        self._assert_highlighted(out)

    # ── JSX: edge cases ────────────────────────────────────────────────────

    def test_jsx_html_entity(self):
        out = self._lex('<span>&lt; &gt; &amp;</span>')
        self.assertIn("span", out)
        self._assert_highlighted(out)

    def test_jsx_multiple_attributes(self):
        out = self._lex('<input type="text" name="email" required disabled />')
        self.assertIn("input", out)
        self.assertIn("type", out)
        self.assertIn("name", out)
        self.assertIn("required", out)
        self.assertIn("disabled", out)
        self._assert_highlighted(out)

    def test_jsx_closing_tag_in_expression(self):
        out = self._lex("<div>{show && <Modal />}</div>")
        self.assertIn("div", out)
        self.assertIn("show", out)
        self.assertIn("Modal", out)
        self._assert_highlighted(out)

    # ── Comments and strings in TSX ────────────────────────────────────────

    def test_comment_outside_jsx(self):
        out = self._lex("// component render")
        self.assertIn("// component render", out)
        self._assert_highlighted(out)

    def test_string_outside_jsx(self):
        out = self._lex('const name = "React";')
        self.assertIn("name", out)
        self.assertIn("React", out)
        self._assert_highlighted(out)

    def test_template_literal_outside_jsx(self):
        out = self._lex('const msg = `Hello ${name}`;')
        self.assertIn("msg", out)
        self.assertIn("Hello", out)
        self._assert_highlighted(out)

    # ── Empty / edge ───────────────────────────────────────────────────────

    def test_empty_string(self):
        out = self._lex("")
        self.assertEqual(out, "")

    def test_multiline_simulation(self):
        lines = [
            'import React from "react";',
            "",
            "interface Props {",
            "  title: string;",
            "}",
            "",
            "export const Card = ({ title }: Props) => (",
            '  <div className="card">',
            "    <h2>{title}</h2>",
            "    <p>Card content</p>",
            "  </div>",
            ");",
        ]
        for line in lines:
            if line:
                out = self._lex(line)
                self.assertTrue(len(out) > 0, f"empty output for: {line!r}")

    def test_complex_component(self):
        out = self._lex(
            'function App() {'
            ' const [count, setCount] = useState(0);'
            ' return <button onClick={() => setCount(count + 1)}>Count: {count}</button>;'
            '}'
        )
        self.assertIn("function", out)
        self.assertIn("return", out)
        self.assertIn("button", out)
        self.assertIn("onClick", out)
        self._assert_highlighted(out)

    def test_via_highlight_lang(self):
        out = self.cs.highlight_lang("<div>hello</div>", "tsx", self.theme_ptr)
        self.assertIn("div", out)
        self._assert_highlighted(out)


if __name__ == "__main__":
    unittest.main()
