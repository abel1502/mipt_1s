//#define TEST
#include "../libs/test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>


// ==== [ Stack inclusion ] ====
typedef double stack_elem_t;
static void printHelper(stack_elem_t elem) {
    printf("%lg", elem);
}
#define STACK_ELEM_PRINT printHelper
#define STACK_VALIDATION_LEVEL 2
#include "stack.h"
// =============================

//--------------------------------------------------------------------------------

int main() {
    TEST_MAIN(
        ,
        test_crc32();
        test_stack(1.23, 4.56, 7.89);
        $g; TEST_MSG("Passed All."); $d;
        ,
    )

    stack_t stk = {};
    assert(stack_construct(&stk, 8));

    assert(!stack_push(&stk, NAN));
    assert(!stack_push(&stk, 99.9));
    stack_dump(&stk);

    for (size_t i = 0; i < 17; ++i) {
        assert(!stack_push(&stk, i + 0.99));
        //printf("%zu\n", stk.capacity);
    }

    stack_dump(&stk);

    for (size_t i = 0; i < 17; ++i) {
        assert(!stack_pop(&stk, NULL));
        //printf("%zu\n", stk.capacity);
    }

    stack_dump(&stk);

    stk.capacity = 0xFFFFFFFFFFFFL;

    stack_free(&stk);

    return EXIT_SUCCESS;
}
