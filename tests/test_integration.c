#include "acutest.h"
#include "../slider.h"
#include <stddef.h>

void test_slider_load_basic(void) {
    Slider *s = slider_load("examples/basic.md");
    TEST_ASSERT(s != NULL);
    
    int count = slider_get_count(s);
    // basic.md tiene 2 slides separados por ---
    TEST_CHECK(count == 2);
    
    slider_free(s);
}

void test_slider_load_nonexistent(void) {
    Slider *s = slider_load("nonexistent.md");
    TEST_CHECK(s == NULL);
}

TEST_LIST = {
    { "slider_load_basic", test_slider_load_basic },
    { "slider_load_nonexistent", test_slider_load_nonexistent },
    { NULL, NULL }
};
