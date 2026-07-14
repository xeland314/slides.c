import unittest
import os
from c_slides_bindings import CSlides


class TestHighlighter(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        lib_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "libslider.so")
        )
        cls.cs = CSlides(lib_path)
        cls.theme = cls.cs.theme_default()

    def test_highlighter_basic(self):
        # Keyword
        out = self.cs.highlight("if", self.theme)
        self.assertIn("if", out)
        self.assertIn("<span foreground=", out)

        # Comment
        out = self.cs.highlight("// comment", self.theme)
        self.assertIn("comment", out)
        self.assertIn("<span foreground=", out)

        # Symbols
        out = self.cs.highlight("{}", self.theme)
        self.assertIn("{", out)
        self.assertIn("}", out)
        self.assertIn("<span foreground=", out)

    def test_highlighter_escaping(self):
        out = self.cs.highlight("a < b && c > d", self.theme)
        self.assertIn("&lt;", out)
        self.assertIn("&gt;", out)
        self.assertIn("&amp;", out)

    def test_highlighter_string(self):
        out = self.cs.highlight('"hello world"', self.theme)
        self.assertIn("hello world", out)
        self.assertIn("<span foreground=", out)

    def test_highlighter_keywords_c(self):
        keywords = ["if", "else", "for", "while", "return", "int", "void",
                     "char", "float", "double", "struct", "enum", "typedef",
                     "switch", "case", "break", "continue", "sizeof"]
        for kw in keywords:
            with self.subTest(keyword=kw):
                out = self.cs.highlight(kw, self.theme)
                self.assertIn("<span foreground=", out)
                self.assertIn(kw, out)

    def test_highlighter_keywords_python(self):
        keywords = ["def", "class", "if", "elif", "else", "for", "while",
                     "return", "import", "from", "True", "False", "None",
                     "and", "or", "not", "in", "is", "lambda", "try",
                     "except", "finally", "with", "as", "yield", "pass"]
        for kw in keywords:
            with self.subTest(keyword=kw):
                out = self.cs.highlight(kw, self.theme)
                self.assertIn(kw, out)

    def test_highlighter_single_line_comment(self):
        out = self.cs.highlight("// This is a single line comment", self.theme)
        self.assertIn("This is a single line comment", out)
        self.assertIn("<span foreground=", out)

    def test_highlighter_python_comment(self):
        out = self.cs.highlight("# Python comment", self.theme)
        self.assertIn("Python comment", out)

    def test_highlighter_strings_double(self):
        out = self.cs.highlight('"double quoted string"', self.theme)
        self.assertIn("double quoted string", out)
        self.assertIn("<span foreground=", out)

    def test_highlighter_strings_single(self):
        out = self.cs.highlight("'single quoted string'", self.theme)
        self.assertIn("single quoted string", out)

    def test_highlighter_numbers_integer(self):
        out = self.cs.highlight("42", self.theme)
        self.assertIn("42", out)
        self.assertIn("<span foreground=", out)

    def test_highlighter_numbers_float(self):
        out = self.cs.highlight("3.14159", self.theme)
        self.assertIn("3.14159", out)

    def test_highlighter_numbers_hex(self):
        out = self.cs.highlight("0xFF", self.theme)
        self.assertIn("0xFF", out)

    def test_highlighter_numbers_binary(self):
        out = self.cs.highlight("0b1010", self.theme)
        self.assertIn("0b1010", out)

    def test_highlighter_numbers_scientific(self):
        out = self.cs.highlight("6.022e23", self.theme)
        self.assertIn("6.022e23", out)

    def test_highlighter_operators(self):
        ops = ["+", "-", "*", "/", "%", "=", "==", "!=", "<", ">", "<=",
               ">=", "&&", "||", "!", "&", "|", "^", "~", "<<", ">>",
               "+=", "-=", "*=", "/=", "%="]
        for op in ops:
            with self.subTest(op=op):
                out = self.cs.highlight(op, self.theme)
                self.assertIn("<span foreground=", out)

    def test_highlighter_brackets(self):
        pairs = [("(", ")"), ("[", "]"), ("{", "}")]
        for open_b, close_b in pairs:
            with self.subTest(pair=f"{open_b}{close_b}"):
                out = self.cs.highlight(f"{open_b}{close_b}", self.theme)
                self.assertIn(open_b, out)
                self.assertIn(close_b, out)
                self.assertIn("<span foreground=", out)

    def test_highlighter_empty_string(self):
        out = self.cs.highlight("", self.theme)
        self.assertEqual(out, "")

    def test_highlighter_pure_text_no_highlight(self):
        out = self.cs.highlight("hello world", self.theme)
        self.assertIn("hello world", out)

    def test_highlighter_mixed_content(self):
        out = self.cs.highlight('if (x == 42) { return "ok"; }', self.theme)
        self.assertIn("if", out)
        self.assertIn("42", out)
        self.assertIn("return", out)
        self.assertIn("<span foreground=", out)

    def test_highlighter_multiline_simulation(self):
        lines = [
            "int main() {",
            "    // comment",
            '    printf("hello");',
            "    return 0;",
            "}",
        ]
        for line in lines:
            out = self.cs.highlight(line, self.theme)
            self.assertIn("<span foreground=", out)

    def test_highlighter_different_themes(self):
        theme_names = ["dark", "light", "dracula", "monokai"]
        for name in theme_names:
            with self.subTest(theme=name):
                theme = self.cs.theme_find(name)
                out = self.cs.highlight("if (true) {}", theme)
                self.assertIn("<span foreground=", out)
                self.assertIn("if", out)

    # ── C family aliases ─────────────────────────────────────────────────

    def _lex_lang(self, code, lang):
        return self.cs.highlight_lang(code, lang, self.cs.theme_default())

    def test_alias_c(self):
        out = self._lex_lang("int x = 1;", "c")
        self.assertIn("int", out)
        self.assertIn("<span foreground=", out)

    def test_alias_cpp(self):
        out = self._lex_lang("int x = 1;", "cpp")
        self.assertIn("int", out)
        self.assertIn("<span foreground=", out)

    def test_alias_cplusplus(self):
        out = self._lex_lang("int x = 1;", "c++")
        self.assertIn("int", out)
        self.assertIn("<span foreground=", out)

    def test_alias_java(self):
        out = self._lex_lang("public class Main {}", "java")
        self.assertIn("class", out)
        self.assertIn("<span foreground=", out)

    def test_alias_cs(self):
        out = self._lex_lang("var x = 1;", "cs")
        self.assertIn("<span foreground=", out)

    def test_alias_csharp(self):
        out = self._lex_lang("var x = 1;", "csharp")
        self.assertIn("<span foreground=", out)

    def test_alias_c_hash(self):
        out = self._lex_lang("var x = 1;", "c#")
        self.assertIn("<span foreground=", out)

    def test_alias_kotlin(self):
        out = self._lex_lang("val x = 1", "kotlin")
        self.assertIn("<span foreground=", out)

    def test_alias_kt(self):
        out = self._lex_lang("val x = 1", "kt")
        self.assertIn("<span foreground=", out)

    def test_alias_swift(self):
        out = self._lex_lang("let x = 1", "swift")
        self.assertIn("<span foreground=", out)

    def test_alias_dart(self):
        out = self._lex_lang("var x = 1;", "dart")
        self.assertIn("<span foreground=", out)

    def test_alias_rust(self):
        out = self._lex_lang("let x = 1;", "rust")
        self.assertIn("<span foreground=", out)

    def test_alias_rs(self):
        out = self._lex_lang("let x = 1;", "rs")
        self.assertIn("<span foreground=", out)

    def test_alias_php(self):
        out = self._lex_lang("$x = 1;", "php")
        self.assertIn("<span foreground=", out)

    def test_alias_scala(self):
        out = self._lex_lang("val x = 1", "scala")
        self.assertIn("<span foreground=", out)

    def test_alias_unknown_falls_back_to_c(self):
        out = self._lex_lang("int x = 1;", "unknown_lang")
        self.assertIn("int", out)
        self.assertIn("<span foreground=", out)


if __name__ == "__main__":
    unittest.main()
