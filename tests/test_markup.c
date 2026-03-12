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

// --- Test Dinámico (Fuzzing) ---

void test_fuzzing_markup(void) {
    const char *tokens[] = {"*", "**", "***", "_", "__", "___", "&", "<", ">", " ", "abc", "123"};
    int n_tokens = sizeof(tokens) / sizeof(tokens[0]);
    char input[512];
    char output[2048]; // Buffer grande para evitar desbordamiento legítimo del algoritmo

    srand(time(NULL));

    // Ejecutamos 10,000 iteraciones aleatorias
    for (int i = 0; i < 10000; i++) {
        input[0] = '\0';
        int len = rand() % 20; // Longitud de 0 a 20 tokens
        for (int j = 0; j < len; j++) {
            strcat(input, tokens[rand() % n_tokens]);
        }

        // Propiedad 1: No debe crashear
        md_to_markup(input, output, sizeof(output));

        // Propiedad 2: El output debe ser nulo-terminado (implícito en strcat/strcpy)
        // Propiedad 3: Los caracteres < y > deben estar solo en tags conocidos o escapados
        const char *p = output;
        while (*p) {
            if (*p == '<') {
                // Solo permitimos tags conocidos: <b>, <i>, </b>, </i>
                int is_valid_tag = (strncmp(p, "<b>", 3) == 0 || strncmp(p, "<i>", 3) == 0 ||
                                    strncmp(p, "</b>", 4) == 0 || strncmp(p, "</i>", 4) == 0);
                if (!is_valid_tag) {
                    TEST_CHECK_(is_valid_tag, "Tag inválido generado en iteración %d: %s (Input: %s)", i, p, input);
                }
            }
            if (*p == '&') {
                // Debe estar escapado como &amp;, &lt; o &gt;
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
    { "fuzzing_markup", test_fuzzing_markup },
    { NULL, NULL }
};

