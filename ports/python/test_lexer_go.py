import unittest
import os
import ctypes
from c_slides_bindings import CSlides


class TestLexerGo(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        lib_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "libslider.so")
        )
        cls.cs = CSlides(lib_path)

    def setUp(self):
        self.theme_ptr = self.cs.theme_default()

    def _lex_go(self, line):
        return self.cs.highlight_lang(line, "go", self.theme_ptr)

    def _assert_highlighted(self, out):
        self.assertIn("<span foreground=", out)

    # ── Keywords ───────────────────────────────────────────────────────────

    def test_keyword_func(self):
        out = self._lex_go("func main()")
        self.assertIn("func", out)
        self._assert_highlighted(out)

    def test_keyword_package(self):
        out = self._lex_go("package main")
        self.assertIn("package", out)
        self._assert_highlighted(out)

    def test_keyword_import(self):
        out = self._lex_go('import "fmt"')
        self.assertIn("import", out)
        self._assert_highlighted(out)

    def test_keyword_var(self):
        out = self._lex_go("var x int")
        self.assertIn("var", out)
        self._assert_highlighted(out)

    def test_keyword_const(self):
        out = self._lex_go("const Pi = 3.14")
        self.assertIn("const", out)
        self._assert_highlighted(out)

    def test_keyword_type(self):
        out = self._lex_go("type Dog struct")
        self.assertIn("type", out)
        self._assert_highlighted(out)

    def test_keyword_struct(self):
        out = self._lex_go("type Dog struct {")
        self.assertIn("struct", out)
        self._assert_highlighted(out)

    def test_keyword_interface(self):
        out = self._lex_go("type Writer interface {")
        self.assertIn("interface", out)
        self._assert_highlighted(out)

    def test_keyword_map(self):
        out = self._lex_go("map[string]int{}")
        self.assertIn("map", out)
        self._assert_highlighted(out)

    def test_keyword_chan(self):
        out = self._lex_go("chan int")
        self.assertIn("chan", out)
        self._assert_highlighted(out)

    def test_keyword_go(self):
        out = self._lex_go("go func() {}()")
        self.assertIn("go", out)
        self._assert_highlighted(out)

    def test_keyword_defer(self):
        out = self._lex_go("defer f.Close()")
        self.assertIn("defer", out)
        self._assert_highlighted(out)

    def test_keyword_select(self):
        out = self._lex_go("select {")
        self.assertIn("select", out)
        self._assert_highlighted(out)

    def test_keyword_switch(self):
        out = self._lex_go("switch x {")
        self.assertIn("switch", out)
        self._assert_highlighted(out)

    def test_keyword_case(self):
        out = self._lex_go("case 1:")
        self.assertIn("case", out)
        self._assert_highlighted(out)

    def test_keyword_if_else(self):
        out = self._lex_go("if x > 0 {} else {}")
        self.assertIn("if", out)
        self.assertIn("else", out)
        self._assert_highlighted(out)

    def test_keyword_for_range(self):
        out = self._lex_go("for i, v := range items {")
        self.assertIn("for", out)
        self.assertIn("range", out)
        self._assert_highlighted(out)

    def test_keyword_return(self):
        out = self._lex_go("return nil")
        self.assertIn("return", out)
        self._assert_highlighted(out)

    def test_keyword_break_continue_fallthrough(self):
        out = self._lex_go("break")
        self.assertIn("break", out)
        self._assert_highlighted(out)
        out2 = self._lex_go("continue")
        self.assertIn("continue", out2)
        self._assert_highlighted(out2)
        out3 = self._lex_go("fallthrough")
        self.assertIn("fallthrough", out3)
        self._assert_highlighted(out3)

    def test_keyword_nil_true_false(self):
        for kw in ["nil", "true", "false"]:
            out = self._lex_go(kw)
            self.assertIn(kw, out)
            self._assert_highlighted(out)

    def test_keyword_iota(self):
        out = self._lex_go("iota")
        self.assertIn("iota", out)
        self._assert_highlighted(out)

    def test_keyword_make_new_len_cap(self):
        for kw in ["make", "new", "len", "cap"]:
            out = self._lex_go(kw + "(x)")
            self.assertIn(kw, out)
            self._assert_highlighted(out)

    def test_keyword_append_copy_delete(self):
        for kw in ["append", "copy", "delete"]:
            out = self._lex_go(kw + "(s, x)")
            self.assertIn(kw, out)
            self._assert_highlighted(out)

    def test_keyword_panic_recover(self):
        for kw in ["panic", "recover"]:
            out = self._lex_go(kw + "()")
            self.assertIn(kw, out)
            self._assert_highlighted(out)

    def test_keyword_print_println(self):
        for kw in ["print", "println"]:
            out = self._lex_go(kw + '("hello")')
            self.assertIn(kw, out)
            self._assert_highlighted(out)

    # ── Built-in types ─────────────────────────────────────────────────────

    def test_builtin_types(self):
        for t in ["int", "int8", "int16", "int32", "int64",
                   "uint", "uint8", "uint16", "uint32", "uint64",
                   "float32", "float64", "complex64", "complex128",
                   "bool", "byte", "rune", "string", "error", "any", "comparable"]:
            out = self._lex_go(f"var x {t}")
            self.assertIn(t, out)
            self._assert_highlighted(out)

    # ── Comments ───────────────────────────────────────────────────────────

    def test_comment_single_line(self):
        out = self._lex_go("// this is a comment")
        self.assertIn("// this is a comment", out)
        self._assert_highlighted(out)

    def test_comment_inline(self):
        out = self._lex_go("x := 5 // inline comment")
        self.assertIn("// inline comment", out)
        self._assert_highlighted(out)

    # ── Strings ────────────────────────────────────────────────────────────

    def test_string_double_quotes(self):
        out = self._lex_go('"hello world"')
        self.assertIn("hello world", out)
        self._assert_highlighted(out)

    def test_string_escaped(self):
        out = self._lex_go('"with \\"escape\\""')
        self.assertIn("with", out)
        self._assert_highlighted(out)

    def test_string_rune(self):
        out = self._lex_go("'a'")
        self.assertIn("a", out)
        self._assert_highlighted(out)

    # ── Numbers ────────────────────────────────────────────────────────────

    def test_number_integer(self):
        out = self._lex_go("42")
        self.assertIn("42", out)
        self._assert_highlighted(out)

    def test_number_float(self):
        out = self._lex_go("3.14")
        self.assertIn("3.14", out)
        self._assert_highlighted(out)

    def test_number_hex(self):
        out = self._lex_go("0xFF")
        self.assertIn("0xFF", out)
        self._assert_highlighted(out)

    def test_number_binary(self):
        out = self._lex_go("0b1010")
        self.assertIn("0b1010", out)
        self._assert_highlighted(out)

    def test_number_octal(self):
        out = self._lex_go("0o77")
        self.assertIn("0o77", out)
        self._assert_highlighted(out)

    def test_number_underscore(self):
        out = self._lex_go("1_000_000")
        self.assertIn("1_000_000", out)
        self._assert_highlighted(out)

    def test_number_scientific(self):
        out = self._lex_go("6.022e23")
        self.assertIn("6.022e23", out)
        self._assert_highlighted(out)

    # ── Operators ──────────────────────────────────────────────────────────

    def test_operator_short_declaration(self):
        out = self._lex_go("x := 5")
        self.assertIn(":", out)
        self.assertIn("=", out)
        self._assert_highlighted(out)

    def test_operator_receive(self):
        out = self._lex_go("v := <-ch")
        self.assertIn("<", out)
        self.assertIn("-", out)
        self._assert_highlighted(out)

    def test_operators_arithmetic(self):
        out = self._lex_go("a + b * c / d % e")
        self.assertIn("+", out)
        self.assertIn("*", out)
        self._assert_highlighted(out)

    def test_operators_comparison(self):
        out = self._lex_go("a == b && c != d || e >= f")
        self._assert_highlighted(out)

    # ── Brackets ───────────────────────────────────────────────────────────

    def test_brackets(self):
        out = self._lex_go("m[key]")
        self.assertIn("[", out)
        self.assertIn("]", out)
        self._assert_highlighted(out)

    def test_braces(self):
        out = self._lex_go("func() {}")
        self.assertIn("{", out)
        self.assertIn("}", out)
        self._assert_highlighted(out)

    # ── Identifiers ────────────────────────────────────────────────────────

    def test_identifier(self):
        out = self._lex_go("myVar")
        self.assertIn("myVar", out)
        # Identifier should NOT be highlighted (plain text)
        self.assertNotIn("myVar</span>", out)

    def test_identifier_underscore_start(self):
        out = self._lex_go("_private")
        self.assertIn("_private", out)

    # ── Empty / edge cases ─────────────────────────────────────────────────

    def test_empty_string(self):
        out = self._lex_go("")
        self.assertEqual(out, "")

    def test_multiline_simulation(self):
        lines = [
            "package main",
            "",
            'import "fmt"',
            "",
            "func main() {",
            '    fmt.Println("Hello, Go!")',
            "}",
        ]
        for line in lines:
            out = self._lex_go(line)
            if line:
                self._assert_highlighted(out)

    def test_complex_expression(self):
        out = self._lex_go("result := a + b*2 - c/3")
        self._assert_highlighted(out)
        self.assertIn("result", out)

    def test_switch_statement(self):
        out = self._lex_go("switch day {")
        self.assertIn("switch", out)
        self.assertIn("day", out)
        self._assert_highlighted(out)

    def test_go_routine(self):
        out = self._lex_go("go processRequest(req)")
        self.assertIn("go", out)
        self.assertIn("processRequest", out)
        self._assert_highlighted(out)

    def test_channel_operations(self):
        out = self._lex_go("ch <- data")
        self.assertIn("ch", out)
        self.assertIn("data", out)
        self._assert_highlighted(out)

    def test_type_declaration(self):
        out = self._lex_go("type Server struct {")
        self.assertIn("type", out)
        self.assertIn("struct", out)
        self.assertIn("Server", out)
        self._assert_highlighted(out)

    def test_interface_method(self):
        out = self._lex_go("    Read(p []byte) (n int, err error)")
        self.assertIn("Read", out)
        self.assertIn("int", out)
        self.assertIn("error", out)
        self._assert_highlighted(out)

    def test_map_literal(self):
        out = self._lex_go('m := map[string]int{"a": 1}')
        self.assertIn("map", out)
        self.assertIn("string", out)
        self.assertIn("int", out)
        self._assert_highlighted(out)

    def test_defer_close(self):
        out = self._lex_go("defer f.Close()")
        self.assertIn("defer", out)
        self._assert_highlighted(out)

    def test_error_check_idiom(self):
        out = self._lex_go("if err != nil {")
        self.assertIn("if", out)
        self.assertIn("err", out)
        self.assertIn("nil", out)
        self._assert_highlighted(out)

    def test_via_highlight_lang(self):
        out = self.cs.highlight_lang("func test()", "go", self.theme_ptr)
        self.assertIn("func", out)
        self._assert_highlighted(out)


if __name__ == "__main__":
    unittest.main()
