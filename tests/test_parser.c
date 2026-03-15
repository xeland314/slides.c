#include "acutest.h"
#include "../src/core/internal.h"
#include <string.h>

// Declaración de la función a testear
void parse_line(const char *raw, SlideLine *out);

void test_parse_title(void) {
    SlideLine sl;
    parse_line("# Mi Titulo", &sl);
    TEST_CHECK(sl.type == LINE_TITLE);
    TEST_CHECK(strcmp(sl.text, "Mi Titulo") == 0);
}

void test_parse_subtitle(void) {
    SlideLine sl;
    parse_line("## Mi Subtitulo", &sl);
    TEST_CHECK(sl.type == LINE_SUBTITLE);
    TEST_CHECK(strcmp(sl.text, "Mi Subtitulo") == 0);
}

void test_parse_blockquote(void) {
    SlideLine sl;
    parse_line("> Nota importante", &sl);
    TEST_CHECK(sl.type == LINE_BLOCKQUOTE);
    TEST_CHECK(strcmp(sl.text, "Nota importante") == 0);
}

void test_parse_bullets(void) {
    SlideLine sl;
    // Nivel 1
    parse_line("- Item 1", &sl);
    TEST_CHECK(sl.type == LINE_BULLET1);
    TEST_CHECK(strcmp(sl.text, "Item 1") == 0);

    // Nivel 2
    parse_line("  - Item 2", &sl);
    TEST_CHECK(sl.type == LINE_BULLET2);
    TEST_CHECK(strcmp(sl.text, "Item 2") == 0);
}

void test_parse_image(void) {
    SlideLine sl;
    // Formato corto
    parse_line("!logo.png", &sl);
    TEST_CHECK(sl.type == LINE_IMAGE);
    TEST_CHECK(strcmp(sl.text, "logo.png") == 0);

    // Formato Markdown completo
    parse_line("![alt text](path/to/img.png)", &sl);
    TEST_CHECK(sl.type == LINE_IMAGE);
    TEST_CHECK(strcmp(sl.text, "path/to/img.png") == 0);
}

void test_parse_table(void) {
    SlideLine sl;
    // Fila normal
    parse_line("| A | B |", &sl);
    TEST_CHECK(sl.type == LINE_TABLE_ROW);
    TEST_CHECK(sl.ncols == 2);
    TEST_CHECK(strcmp(sl.cols[0], "A") == 0);
    TEST_CHECK(strcmp(sl.cols[1], "B") == 0);

    // Separador
    parse_line("|---|---|", &sl);
    TEST_CHECK(sl.type == LINE_TABLE_SEP);
}

void test_parse_task_list(void) {
    SlideLine sl;
    // Unchecked
    parse_line("- [ ] Tarea pendiente", &sl);
    TEST_CHECK(sl.type == LINE_TASK_UNCHECKED);
    TEST_CHECK(strcmp(sl.text, "Tarea pendiente") == 0);

    // Checked
    parse_line("- [x] Tarea completada", &sl);
    TEST_CHECK(sl.type == LINE_TASK_CHECKED);
    TEST_CHECK(strcmp(sl.text, "Tarea completada") == 0);

    // Checked uppercase
    parse_line("- [X] Tarea completada 2", &sl);
    TEST_CHECK(sl.type == LINE_TASK_CHECKED);
    TEST_CHECK(strcmp(sl.text, "Tarea completada 2") == 0);
}

TEST_LIST = {
    { "parse_title", test_parse_title },
    { "parse_subtitle", test_parse_subtitle },
    { "parse_blockquote", test_parse_blockquote },
    { "parse_bullets", test_parse_bullets },
    { "parse_image", test_parse_image },
    { "parse_table", test_parse_table },
    { "parse_task_list", test_parse_task_list },
    { NULL, NULL }
};
