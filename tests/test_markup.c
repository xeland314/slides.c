#include "acutest.h"
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>

// Declaración de la función a testear
void md_to_markup(const char *in, char *out, size_t out_size);

void test_simple_bold(void) {
    char out[256];
    md_to_markup("**bold**", out, sizeof(out));
    TEST_CHECK(strcmp(out, "<b>bold</b>") == 0);
}

void test_simple_italic(void) {
    char out[256];
    md_to_markup("*italic*", out, sizeof(out));
    TEST_CHECK(strcmp(out, "<i>italic</i>") == 0);
}

void test_combined_bold_italic(void) {
    char out[256];
    md_to_markup("***ambos***", out, sizeof(out));
    TEST_CHECK(strcmp(out, "<b><i>ambos</i></b>") == 0);
}

void test_nested_complex(void) {
    char out[256];
    md_to_markup("Texto **con bold y *italic* mixto**", out, sizeof(out));
    TEST_CHECK(strcmp(out, "Texto <b>con bold y <i>italic</i> mixto</b>") == 0);
}

void test_escaping(void) {
    char out[256];
    md_to_markup("A & B < C", out, sizeof(out));
    TEST_CHECK(strcmp(out, "A &amp; B &lt; C") == 0);
}

void test_inline_code(void) {
    char out[256];
    md_to_markup("Usa `printf()`", out, sizeof(out));
    TEST_CHECK(strcmp(out, "Usa <tt>printf()</tt>") == 0);

    md_to_markup("`**no bold**`", out, sizeof(out));
    TEST_CHECK(strcmp(out, "<tt>**no bold**</tt>") == 0);
}

// --- Test Dinámico (Fuzzing) ---

void test_fuzzing_markup(void) {
    const char *tokens[] = {"*", "**", "***", "_", "__", "___", "&", "<", ">", "`", " ", "abc", "123"};
    int n_tokens = sizeof(tokens) / sizeof(tokens[0]);
    char input[512];
    char output[2048];

    srand(time(NULL));

    for (int i = 0; i < 10000; i++) {
        input[0] = '\0';
        int len = rand() % 20;
        for (int j = 0; j < len; j++) {
            strcat(input, tokens[rand() % n_tokens]);
        }

        md_to_markup(input, output, sizeof(output));

        const char *p = output;
        while (*p) {
            if (*p == '<') {
                int is_valid_tag = (strncmp(p, "<b>", 3) == 0 || strncmp(p, "<i>", 3) == 0 ||
                                    strncmp(p, "<tt>", 4) == 0 ||
                                    strncmp(p, "</b>", 4) == 0 || strncmp(p, "</i>", 4) == 0 ||
                                    strncmp(p, "</tt>", 5) == 0);
                if (!is_valid_tag) {
                    TEST_CHECK_(is_valid_tag, "Tag inválido generado en iteración %d: %s (Input: %s)", i, p, input);
                }
            }
            if (*p == '&') {
                int is_escaped = (strncmp(p, "&amp;", 5) == 0 || strncmp(p, "&lt;", 4) == 0 || strncmp(p, "&gt;", 4) == 0);
                if (!is_escaped) {
                    TEST_CHECK_(is_escaped, "Ampersand no escapado en iteración %d: %s (Input: %s)", i, p, input);
                }
            }
            p++;
        }
    }
}

TEST_LIST = {
    { "simple_bold", test_simple_bold },
    { "simple_italic", test_simple_italic },
    { "combined_bold_italic", test_combined_bold_italic },
    { "nested_complex", test_nested_complex },
    { "escaping", test_escaping },
    { "inline_code", test_inline_code },
    { "fuzzing_markup", test_fuzzing_markup },
    { NULL, NULL }
};
