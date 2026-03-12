#include "acutest.h"
#include <string.h>
#include <stddef.h>

// Declaración de la función a testear
void md_to_markup(const char *in, char *out, size_t out_size);

void test_simple_bold(void) {
    char out[256];
    md_to_markup("**bold**", out, sizeof(out));
    TEST_CHECK(strcmp(out, "<b>bold</b>") == 0);
    TEST_MSG("Expected: <b>bold</b>, Got: %s", out);
}

void test_simple_italic(void) {
    char out[256];
    md_to_markup("*italic*", out, sizeof(out));
    TEST_CHECK(strcmp(out, "<i>italic</i>") == 0);
    TEST_MSG("Expected: <i>italic</i>, Got: %s", out);
}

void test_combined_bold_italic(void) {
    char out[256];
    md_to_markup("***ambos***", out, sizeof(out));
    // Con la lógica nueva: *** -> <b><i>
    TEST_CHECK(strcmp(out, "<b><i>ambos</i></b>") == 0);
    TEST_MSG("Expected: <b><i>ambos</i></b>, Got: %s", out);
}

void test_nested_complex(void) {
    char out[256];
    md_to_markup("Texto **con bold y *italic* mixto**", out, sizeof(out));
    // El parser actual es secuencial simple, veamos qué produce
    // **con -> <b>con
    // bold y -> bold y
    // *italic* -> <i>italic</i>
    // mixto** -> mixto</b>
    TEST_CHECK(strcmp(out, "Texto <b>con bold y <i>italic</i> mixto</b>") == 0);
    TEST_MSG("Expected: Texto <b>con bold y <i>italic</i> mixto</b>, Got: %s", out);
}

void test_escaping(void) {
    char out[256];
    md_to_markup("A & B < C", out, sizeof(out));
    TEST_CHECK(strcmp(out, "A &amp; B &lt; C") == 0);
}

TEST_LIST = {
    { "simple_bold", test_simple_bold },
    { "simple_italic", test_simple_italic },
    { "combined_bold_italic", test_combined_bold_italic },
    { "nested_complex", test_nested_complex },
    { "escaping", test_escaping },
    { NULL, NULL }
};
