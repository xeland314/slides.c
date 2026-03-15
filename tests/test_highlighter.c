#include "acutest.h"
#include "../src/core/highlighter.h"
#include "../src/core/theme.h"
#include <string.h>

void test_highlighter_basic(void) {
    const Theme *theme = theme_default();
    char out[1024];
    
    // Keyword
    highlighter_highlight("if", theme, out, sizeof(out));
    TEST_CHECK(strstr(out, "if") != NULL);
    TEST_CHECK(strstr(out, "<span foreground=") != NULL);

    // Comment
    highlighter_highlight("// comment", theme, out, sizeof(out));
    TEST_CHECK(strstr(out, "comment") != NULL);
    TEST_CHECK(strstr(out, "<span foreground=") != NULL);

    // Symbols
    highlighter_highlight("{}", theme, out, sizeof(out));
    TEST_CHECK(strstr(out, "{") != NULL);
    TEST_CHECK(strstr(out, "}") != NULL);
    TEST_CHECK(strstr(out, "<span foreground=") != NULL);
}

void test_highlighter_escaping(void) {
    const Theme *theme = theme_default();
    char out[1024];
    
    // Escaping
    highlighter_highlight("a < b && c > d", theme, out, sizeof(out));
    TEST_CHECK(strstr(out, "&lt;") != NULL);
    TEST_CHECK(strstr(out, "&gt;") != NULL);
    TEST_CHECK(strstr(out, "&amp;") != NULL);
}

void test_highlighter_string(void) {
    const Theme *theme = theme_default();
    char out[1024];
    
    // String
    highlighter_highlight("\"hello world\"", theme, out, sizeof(out));
    TEST_CHECK(strstr(out, "hello world") != NULL);
    TEST_CHECK(strstr(out, "<span foreground=") != NULL);
}

TEST_LIST = {
    { "highlighter_basic", test_highlighter_basic },
    { "highlighter_escaping", test_highlighter_escaping },
    { "highlighter_string", test_highlighter_string },
    { NULL, NULL }
};
