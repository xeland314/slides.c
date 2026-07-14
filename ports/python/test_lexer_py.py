import unittest
import os
from c_slides_bindings import CSlides


class TestLexerPy(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        lib_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "libslider.so")
        )
        cls.cs = CSlides(lib_path)
        cls.theme = cls.cs.theme_default().contents

    def _lex(self, line):
        return self.cs.highlight_python(line, ctypes.byref(self.theme) if hasattr(self, '_use_ref') else self.theme)

    def setUp(self):
        import ctypes
        self.theme_ptr = self.cs.theme_default()

    def _lex_py(self, line):
        return self.cs.highlight_python(line, self.theme_ptr)

    def _lex_lang(self, line, lang):
        return self.cs.highlight_lang(line, lang, self.theme_ptr)

    def test_keyword_def(self):
        out = self._lex_py("def foo():")
        self.assertIn("foo", out)

    def test_keyword_class(self):
        out = self._lex_py("class MyClass:")
        self.assertIn("MyClass", out)

    def test_keyword_return(self):
        out = self._lex_py("return 42")
        self.assertIn("42", out)

    def test_keyword_if_else(self):
        out = self._lex_py("if x: else:")
        self.assertIn("x", out)

    def test_keyword_for_while(self):
        out = self._lex_py("for i in range(10): while True:")
        self.assertIn("i", out)

    def test_keyword_import_from(self):
        out = self._lex_py("import os from sys import argv")
        self.assertIn("os", out)

    def test_keyword_try_except(self):
        out = self._lex_py("try: except ValueError:")
        self.assertIn("ValueError", out)

    def test_keyword_none_true_false(self):
        out = self._lex_py("None True False")
        self.assertIn("None", out)

    def test_keyword_pass_break_continue(self):
        out = self._lex_py("pass break continue")
        self.assertIn("pass", out)

    def test_keyword_lambda(self):
        out = self._lex_py("lambda x: x + 1")
        self.assertIn("x", out)

    def test_keyword_with_as(self):
        out = self._lex_py("with open('f') as f:")
        self.assertIn("f", out)

    def test_keyword_raise(self):
        out = self._lex_py("raise Exception('err')")
        self.assertIn("Exception", out)

    def test_comment_single(self):
        out = self._lex_py("# This is a comment")
        self.assertIn("# This is a comment", out)

    def test_comment_inline(self):
        out = self._lex_py("x = 1  # comment")
        self.assertIn("# comment", out)

    def test_string_double_quotes(self):
        out = self._lex_py('"hello world"')
        self.assertIn("hello world", out)

    def test_string_single_quotes(self):
        out = self._lex_py("'hello world'")
        self.assertIn("hello world", out)

    def test_string_escaped(self):
        out = self._lex_py(r'"hello \"world\""')
        self.assertIn("hello", out)

    def test_number_integer(self):
        out = self._lex_py("42")
        self.assertIn("42", out)

    def test_number_hex(self):
        out = self._lex_py("0xFF")
        self.assertIn("0xFF", out)

    def test_number_binary(self):
        out = self._lex_py("0b1010")
        self.assertIn("0b1010", out)

    def test_number_octal(self):
        out = self._lex_py("0o17")
        self.assertIn("0o17", out)

    def test_number_float(self):
        out = self._lex_py("3.14")
        self.assertIn("3.14", out)

    def test_number_scientific(self):
        out = self._lex_py("1e10")
        self.assertIn("1e10", out)

    def test_number_complex(self):
        out = self._lex_py("3j")
        self.assertIn("3", out)
        self.assertIn("j", out)

    def test_number_underscore(self):
        out = self._lex_py("1_000_000")
        self.assertIn("1_000_000", out)

    def test_brackets(self):
        out = self._lex_py("([{}])")
        self.assertIn("(", out)
        self.assertIn(")", out)
        self.assertIn("[", out)
        self.assertIn("]", out)
        self.assertIn("{", out)
        self.assertIn("}", out)

    def test_operators(self):
        out = self._lex_py("a + b - c * d / e % f")
        self.assertIn("+", out)
        self.assertIn("-", out)

    def test_comparison_operators(self):
        out = self._lex_py("a < b > c <= d >= e == f != g")
        self.assertIn("<", out)

    def test_identifier(self):
        out = self._lex_py("my_variable_name")
        self.assertIn("my_variable_name", out)

    def test_identifier_underscore_start(self):
        out = self._lex_py("_private")
        self.assertIn("_private", out)

    def test_empty_string(self):
        out = self._lex_py("")
        self.assertEqual(out, "")

    def test_multiline_simulation(self):
        lines = ["def hello():", "    print('hi')", "    return True"]
        result = ""
        for line in lines:
            result += self._lex_py(line) + "\n"
        self.assertIn("def", result)
        self.assertIn("print", result)

    def test_via_highlighter_python(self):
        out = self._lex_lang("def foo(): pass", "python")
        self.assertIn("def", out)
        self.assertIn("foo", out)

    def test_via_highlighter_py_alias(self):
        out = self._lex_lang("def foo(): pass", "py")
        self.assertIn("def", out)
        self.assertIn("foo", out)

    def test_keyword_not_and_or(self):
        out = self._lex_py("not True and False or True")
        self.assertIn("not", out)
        self.assertIn("and", out)
        self.assertIn("or", out)

    def test_keyword_in_is(self):
        out = self._lex_py("x in y is None")
        self.assertIn("in", out)
        self.assertIn("is", out)

    def test_string_triple_double(self):
        out = self._lex_py('"""docstring"""')
        self.assertIn("docstring", out)

    def test_string_triple_single(self):
        out = self._lex_py("'''docstring'''")
        self.assertIn("docstring", out)

    def test_attribute_access(self):
        out = self._lex_py("obj.method")
        self.assertIn("obj", out)
        self.assertIn("method", out)

    def test_decorator(self):
        out = self._lex_py("@staticmethod")
        self.assertIn("@", out)
        self.assertIn("staticmethod", out)

    def test_fstring(self):
        out = self._lex_py('f"value={x}"')
        self.assertIn("f", out)
        self.assertIn("value", out)

    def test_ellipsis(self):
        out = self._lex_py("...")
        self.assertIn(".", out)

    def test_complex_expression(self):
        out = self._lex_py("result = [x**2 for x in range(10) if x % 2 == 0]")
        self.assertIn("result", out)

    def test_walrus_operator(self):
        out = self._lex_py("if (n := len(a)) > 10:")
        self.assertIn("n", out)

    def test_star_import(self):
        out = self._lex_py("from module import *")
        self.assertIn("*", out)

    def test_negative_number(self):
        out = self._lex_py("-42")
        self.assertIn("-", out)
        self.assertIn("42", out)

    def test_power_operator(self):
        out = self._lex_py("2 ** 10")
        self.assertIn("*", out)
        self.assertIn("2", out)
        self.assertIn("10", out)


import ctypes

if __name__ == "__main__":
    unittest.main()
