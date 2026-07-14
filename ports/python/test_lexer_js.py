import unittest
import os
from c_slides_bindings import CSlides


class TestLexerJS(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        lib_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "libslider.so")
        )
        cls.cs = CSlides(lib_path)

    def setUp(self):
        self.theme_ptr = self.cs.theme_default()

    def _lex(self, line):
        return self.cs.highlight_lang(line, "javascript", self.theme_ptr)

    def _assert_highlighted(self, out):
        self.assertIn("<span foreground=", out)

    # ── Keywords ───────────────────────────────────────────────────────────

    def test_keyword_var(self):
        out = self._lex("var x = 5;")
        self.assertIn("var", out)
        self._assert_highlighted(out)

    def test_keyword_let(self):
        out = self._lex("let x = 5;")
        self.assertIn("let", out)
        self._assert_highlighted(out)

    def test_keyword_const(self):
        out = self._lex("const x = 5;")
        self.assertIn("const", out)
        self._assert_highlighted(out)

    def test_keyword_function(self):
        out = self._lex("function hello() {}")
        self.assertIn("function", out)
        self._assert_highlighted(out)

    def test_keyword_return(self):
        out = self._lex("return 42;")
        self.assertIn("return", out)
        self._assert_highlighted(out)

    def test_keyword_if_else(self):
        out = self._lex("if (x > 0) {} else {}")
        self.assertIn("if", out)
        self.assertIn("else", out)
        self._assert_highlighted(out)

    def test_keyword_for_while_do(self):
        for kw in ["for", "while", "do"]:
            out = self._lex(f"{kw} (true) {{}}")
            self.assertIn(kw, out)
            self._assert_highlighted(out)

    def test_keyword_switch_case_default(self):
        for kw in ["switch", "case", "default"]:
            out = self._lex(f"{kw}: break;")
            self.assertIn(kw, out)
            self._assert_highlighted(out)

    def test_keyword_break_continue(self):
        out = self._lex("break;")
        self.assertIn("break", out)
        self._assert_highlighted(out)
        out2 = self._lex("continue;")
        self.assertIn("continue", out2)
        self._assert_highlighted(out2)

    def test_keyword_new_this(self):
        for kw in ["new", "this"]:
            out = self._lex(f"{kw}.prop")
            self.assertIn(kw, out)
            self._assert_highlighted(out)

    def test_keyword_class_extends_super(self):
        for kw in ["class", "extends", "super"]:
            out = self._lex(f"{kw} Foo")
            self.assertIn(kw, out)
            self._assert_highlighted(out)

    def test_keyword_import_export_from(self):
        for kw in ["import", "export", "from"]:
            out = self._lex(f'{kw} "module"')
            self.assertIn(kw, out)
            self._assert_highlighted(out)

    def test_keyword_try_catch_finally(self):
        for kw in ["try", "catch", "finally"]:
            out = self._lex(f"{kw} {{}}")
            self.assertIn(kw, out)
            self._assert_highlighted(out)

    def test_keyword_throw(self):
        out = self._lex('throw new Error("fail");')
        self.assertIn("throw", out)
        self._assert_highlighted(out)

    def test_keyword_async_await(self):
        for kw in ["async", "await"]:
            out = self._lex(f"{kw} function() {{}}")
            self.assertIn(kw, out)
            self._assert_highlighted(out)

    def test_keyword_yield(self):
        out = self._lex("yield result;")
        self.assertIn("yield", out)
        self._assert_highlighted(out)

    def test_keyword_instanceof_typeof_void_delete(self):
        for kw in ["instanceof", "typeof", "void", "delete"]:
            out = self._lex(f"{kw} x")
            self.assertIn(kw, out)
            self._assert_highlighted(out)

    def test_keyword_true_false_null_undefined(self):
        for kw in ["true", "false", "null", "undefined"]:
            out = self._lex(kw)
            self.assertIn(kw, out)
            self._assert_highlighted(out)

    def test_keyword_nan_infinity(self):
        for kw in ["NaN", "Infinity"]:
            out = self._lex(kw)
            self.assertIn(kw, out)
            self._assert_highlighted(out)

    def test_keyword_static_get_set(self):
        for kw in ["static", "get", "set"]:
            out = self._lex(f"{kw} prop() {{}}")
            self.assertIn(kw, out)
            self._assert_highlighted(out)

    def test_keyword_with_debugger(self):
        for kw in ["with", "debugger"]:
            out = self._lex(f"{kw};")
            self.assertIn(kw, out)
            self._assert_highlighted(out)

    def test_keyword_of_in(self):
        for kw in ["of", "in"]:
            out = self._lex(f"for (let x {kw} arr) {{}}")
            self.assertIn(kw, out)
            self._assert_highlighted(out)

    # ── Comments ───────────────────────────────────────────────────────────

    def test_comment_single_line(self):
        out = self._lex("// comment")
        self.assertIn("// comment", out)
        self._assert_highlighted(out)

    def test_comment_inline(self):
        out = self._lex("x = 5; // inline")
        self.assertIn("// inline", out)
        self._assert_highlighted(out)

    def test_comment_multi_line(self):
        out = self._lex("/* block comment */")
        self.assertIn("block comment", out)
        self._assert_highlighted(out)

    # ── Strings ────────────────────────────────────────────────────────────

    def test_string_double_quotes(self):
        out = self._lex('"hello"')
        self.assertIn("hello", out)
        self._assert_highlighted(out)

    def test_string_single_quotes(self):
        out = self._lex("'hello'")
        self.assertIn("hello", out)
        self._assert_highlighted(out)

    def test_string_template_literal(self):
        out = self._lex('`template ${x}`')
        self.assertIn("template", out)
        self._assert_highlighted(out)

    def test_string_escaped(self):
        out = self._lex('"with \\"escape\\""')
        self.assertIn("with", out)
        self._assert_highlighted(out)

    # ── Numbers ────────────────────────────────────────────────────────────

    def test_number_integer(self):
        out = self._lex("42")
        self.assertIn("42", out)
        self._assert_highlighted(out)

    def test_number_float(self):
        out = self._lex("3.14")
        self.assertIn("3.14", out)
        self._assert_highlighted(out)

    def test_number_hex(self):
        out = self._lex("0xFF")
        self.assertIn("0xFF", out)
        self._assert_highlighted(out)

    def test_number_binary(self):
        out = self._lex("0b1010")
        self.assertIn("0b1010", out)
        self._assert_highlighted(out)

    def test_number_octal(self):
        out = self._lex("0o77")
        self.assertIn("0o77", out)
        self._assert_highlighted(out)

    def test_number_underscore(self):
        out = self._lex("1_000_000")
        self.assertIn("1_000_000", out)
        self._assert_highlighted(out)

    def test_number_scientific(self):
        out = self._lex("6.022e23")
        self.assertIn("6.022e23", out)
        self._assert_highlighted(out)

    # ── Operators ──────────────────────────────────────────────────────────

    def test_operators(self):
        out = self._lex("a + b * c / d % e")
        self._assert_highlighted(out)

    def test_operators_comparison(self):
        out = self._lex("a == b && c != d || e >= f")
        self._assert_highlighted(out)

    def test_operator_ternary(self):
        out = self._lex("x ? y : z")
        self._assert_highlighted(out)

    # ── Identifiers ────────────────────────────────────────────────────────

    def test_identifier(self):
        out = self._lex("myVar")
        self.assertIn("myVar", out)
        self.assertNotIn("myVar</span>", out)

    def test_identifier_dollar(self):
        out = self._lex("$element")
        self.assertIn("$element", out)

    def test_identifier_underscore_start(self):
        out = self._lex("_private")
        self.assertIn("_private", out)

    # ── Empty / edge ───────────────────────────────────────────────────────

    def test_empty_string(self):
        out = self._lex("")
        self.assertEqual(out, "")

    def test_multiline_simulation(self):
        lines = [
            'import React from "react";',
            "",
            "export default function App() {",
            "  return <div>Hello</div>;",
            "}",
        ]
        for line in lines:
            if line:
                out = self._lex(line)
                self._assert_highlighted(out)

    def test_complex_expression(self):
        out = self._lex("const result = arr.map(x => x * 2).filter(x => x > 5);")
        self._assert_highlighted(out)

    def test_destructuring(self):
        out = self._lex("const { a, b } = obj;")
        self._assert_highlighted(out)

    def test_arrow_function(self):
        out = self._lex("const add = (a, b) => a + b;")
        self._assert_highlighted(out)

    def test_optional_chaining(self):
        out = self._lex("obj?.prop?.method()")
        self._assert_highlighted(out)

    def test_nullish_coalescing(self):
        out = self._lex("val ?? 'default'")
        self._assert_highlighted(out)

    def test_spread_operator(self):
        out = self._lex("const arr = [...other];")
        self._assert_highlighted(out)

    def test_async_await_full(self):
        out = self._lex("async function fetchData() { const res = await fetch(url); }")
        self.assertIn("async", out)
        self.assertIn("await", out)
        self._assert_highlighted(out)

    def test_class_with_constructor(self):
        out = self._lex("class Animal { constructor(name) { this.name = name; } }")
        self.assertIn("class", out)
        self.assertIn("constructor", out)
        self.assertIn("this", out)
        self._assert_highlighted(out)

    def test_promise_chaining(self):
        out = self._lex("fetch(url).then(res => res.json()).catch(err => console.log(err));")
        self._assert_highlighted(out)

    def test_via_highlight_lang(self):
        out = self.cs.highlight_lang("const x = 5;", "javascript", self.theme_ptr)
        self.assertIn("const", out)
        self._assert_highlighted(out)


if __name__ == "__main__":
    unittest.main()
