import unittest
import os
from c_slides_bindings import CSlides


class TestLexerTS(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        lib_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "libslider.so")
        )
        cls.cs = CSlides(lib_path)

    def setUp(self):
        self.theme_ptr = self.cs.theme_default()

    def _lex(self, line):
        return self.cs.highlight_lang(line, "typescript", self.theme_ptr)

    def _assert_highlighted(self, out):
        self.assertIn("<span foreground=", out)

    # ── JS keywords (inherited) ────────────────────────────────────────────

    def test_keyword_const(self):
        out = self._lex("const x = 5;")
        self.assertIn("const", out)
        self._assert_highlighted(out)

    def test_keyword_function(self):
        out = self._lex("function hello() {}")
        self.assertIn("function", out)
        self._assert_highlighted(out)

    def test_keyword_class(self):
        out = self._lex("class Dog {}")
        self.assertIn("class", out)
        self._assert_highlighted(out)

    def test_keyword_async_await(self):
        out = self._lex("async function go() { await fetch(); }")
        self.assertIn("async", out)
        self.assertIn("await", out)
        self._assert_highlighted(out)

    def test_keyword_import_export(self):
        out = self._lex('import { Component } from "react";')
        self.assertIn("import", out)
        self.assertIn("from", out)
        self._assert_highlighted(out)

    def test_keyword_true_false_null_undefined(self):
        for kw in ["true", "false", "null", "undefined"]:
            out = self._lex(kw)
            self.assertIn(kw, out)
            self._assert_highlighted(out)

    # ── TypeScript-specific keywords ───────────────────────────────────────

    def test_keyword_type(self):
        out = self._lex("type ID = string | number;")
        self.assertIn("type", out)
        self._assert_highlighted(out)

    def test_keyword_interface(self):
        out = self._lex("interface User { name: string; }")
        self.assertIn("interface", out)
        self._assert_highlighted(out)

    def test_keyword_enum(self):
        out = self._lex("enum Color { Red, Green, Blue }")
        self.assertIn("enum", out)
        self._assert_highlighted(out)

    def test_keyword_namespace(self):
        out = self._lex("namespace Utils {}")
        self.assertIn("namespace", out)
        self._assert_highlighted(out)

    def test_keyword_module(self):
        out = self._lex('declare module "lodash" {}')
        self.assertIn("module", out)
        self._assert_highlighted(out)

    def test_keyword_declare(self):
        out = self._lex("declare const api: string;")
        self.assertIn("declare", out)
        self._assert_highlighted(out)

    def test_keyword_abstract(self):
        out = self._lex("abstract class Shape {}")
        self.assertIn("abstract", out)
        self._assert_highlighted(out)

    def test_keyword_implements(self):
        out = self._lex("class Foo implements Bar {}")
        self.assertIn("implements", out)
        self._assert_highlighted(out)

    def test_keyword_readonly(self):
        out = self._lex("readonly name: string;")
        self.assertIn("readonly", out)
        self._assert_highlighted(out)

    def test_keyword_access_modifiers(self):
        for kw in ["private", "protected", "public"]:
            out = self._lex(f"{kw} field: string;")
            self.assertIn(kw, out)
            self._assert_highlighted(out)

    def test_keyword_keyof(self):
        out = self._lex("type Keys = keyof User;")
        self.assertIn("keyof", out)
        self._assert_highlighted(out)

    def test_keyword_infer(self):
        out = self._lex("type T = infer U;")
        self.assertIn("infer", out)
        self._assert_highlighted(out)

    def test_keyword_is(self):
        out = self._lex("function isString(x: any): x is string {}")
        self.assertIn("is", out)
        self._assert_highlighted(out)

    def test_keyword_asserts(self):
        out = self._lex("function assert(x: any): asserts x {}")
        self.assertIn("asserts", out)
        self._assert_highlighted(out)

    def test_keyword_satisfies(self):
        out = self._lex("const x = { a: 1 } satisfies Record<string, number>;")
        self.assertIn("satisfies", out)
        self._assert_highlighted(out)

    def test_keyword_override(self):
        out = self._lex("override toString() {}")
        self.assertIn("override", out)
        self._assert_highlighted(out)

    # ── Comments ───────────────────────────────────────────────────────────

    def test_comment_single_line(self):
        out = self._lex("// TypeScript comment")
        self.assertIn("// TypeScript comment", out)
        self._assert_highlighted(out)

    def test_comment_multi_line(self):
        out = self._lex("/* block */")
        self.assertIn("block", out)
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
        out = self._lex('`template ${name}`')
        self.assertIn("template", out)
        self._assert_highlighted(out)

    # ── Numbers ────────────────────────────────────────────────────────────

    def test_number_integer(self):
        out = self._lex("42")
        self.assertIn("42", out)
        self._assert_highlighted(out)

    def test_number_hex(self):
        out = self._lex("0xFF")
        self.assertIn("0xFF", out)
        self._assert_highlighted(out)

    def test_number_float(self):
        out = self._lex("3.14")
        self.assertIn("3.14", out)
        self._assert_highlighted(out)

    # ── TypeScript patterns ────────────────────────────────────────────────

    def test_type_annotation(self):
        out = self._lex("function greet(name: string): void {}")
        self.assertIn("string", out)
        self.assertIn("void", out)
        self._assert_highlighted(out)

    def test_generic_type(self):
        out = self._lex("function identity<T>(x: T): T { return x; }")
        self.assertIn("function", out)
        self.assertIn("return", out)
        self._assert_highlighted(out)

    def test_union_type(self):
        out = self._lex("type Status = 'active' | 'inactive' | 'pending';")
        self.assertIn("type", out)
        self._assert_highlighted(out)

    def test_interface_with_methods(self):
        out = self._lex("interface Serializable { serialize(): string; deserialize(data: string): void; }")
        self.assertIn("interface", out)
        self.assertIn("serialize", out)
        self._assert_highlighted(out)

    def test_enum_with_values(self):
        out = self._lex("enum Direction { Up = 'UP', Down = 'DOWN' }")
        self.assertIn("enum", out)
        self._assert_highlighted(out)

    def test_class_with_access_modifiers(self):
        out = self._lex("class Person { private age: number; protected name: string; public greet() {} }")
        self.assertIn("class", out)
        self.assertIn("private", out)
        self.assertIn("protected", out)
        self.assertIn("public", out)
        self._assert_highlighted(out)

    def test_abstract_class(self):
        out = self._lex("abstract class Shape { abstract area(): number; }")
        self.assertIn("abstract", out)
        self._assert_highlighted(out)

    def test_readonly_property(self):
        out = self._lex("class Config { readonly apiUrl: string = 'https://api.example.com'; }")
        self.assertIn("readonly", out)
        self._assert_highlighted(out)

    def test_type_assertion(self):
        out = self._lex("const el = document.getElementById('app') as HTMLDivElement;")
        self.assertIn("as", out)
        self._assert_highlighted(out)

    def test_satisfies_example(self):
        out = self._lex("const routes = { home: '/' } satisfies Record<string, string>;")
        self.assertIn("satisfies", out)
        self._assert_highlighted(out)

    # ── Empty / edge ───────────────────────────────────────────────────────

    def test_empty_string(self):
        out = self._lex("")
        self.assertEqual(out, "")

    def test_multiline_simulation(self):
        lines = [
            'import { useState } from "react";',
            "",
            "interface Props {",
            "  title: string;",
            "  onClick: () => void;",
            "}",
            "",
            "export const Button = ({ title, onClick }: Props) => {",
            "  return <button onClick={onClick}>{title}</button>;",
            "};",
        ]
        for line in lines:
            if line:
                out = self._lex(line)
                self._assert_highlighted(out)

    def test_complex_expression(self):
        out = self._lex("const result: number[] = data.filter((x): x is number => typeof x === 'number');")
        self._assert_highlighted(out)

    def test_mapped_type(self):
        out = self._lex("type Optional<T> = { [K in keyof T]?: T[K] };")
        self.assertIn("type", out)
        self.assertIn("keyof", out)
        self._assert_highlighted(out)

    def test_conditional_type(self):
        out = self._lex("type IsString<T> = T extends string ? true : false;")
        self.assertIn("type", out)
        self._assert_highlighted(out)

    def test_via_highlight_lang(self):
        out = self.cs.highlight_lang("const x: number = 5;", "typescript", self.theme_ptr)
        self.assertIn("const", out)
        self._assert_highlighted(out)


if __name__ == "__main__":
    unittest.main()
