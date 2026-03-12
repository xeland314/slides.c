#include "acutest.h"
#include "../slider.h"
#include "../src/core/theme.h"
#include "../src/core/internal.h"
#include <stddef.h>
#include <string.h>

void test_slider_load_basic(void) {
    Slider *s = slider_load("examples/basic.md");
    TEST_ASSERT(s != NULL);
    
    int count = slider_get_count(s);
    TEST_CHECK(count == 2);
    
    // Verificar que tiene un tema por defecto
    TEST_ASSERT(s->theme != NULL);
    TEST_CHECK(strcmp(s->theme->name, "dark") == 0);
    
    slider_free(s);
}

void test_slider_change_palette(void) {
    Slider *s = slider_load("examples/basic.md");
    TEST_ASSERT(s != NULL);
    
    s->theme = theme_find("rose");
    TEST_ASSERT(s->theme != NULL);
    TEST_CHECK(strcmp(s->theme->name, "rose") == 0);
    
    slider_free(s);
}

TEST_LIST = {
    { "slider_load_basic", test_slider_load_basic },
    { "slider_change_palette", test_slider_change_palette },
    { NULL, NULL }
};
