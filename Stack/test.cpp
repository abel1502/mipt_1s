//#define TEST
#include "../libs/test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include "checksum.h"

// ==== [ Stack inclusion ] ====
typedef double stack_elem_t;
static void printHelper(stack_elem_t elem) {
    printf("%lg", elem);
}
#define STACK_ELEM_PRINT printHelper
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
    stack_construct(&stk, 10);

    stack_push(&stk, NAN);
    stack_push(&stk, 99.9);
    stack_dump(&stk);

    stk.capacity =0x1FFFFFFFFL;

    stack_free(&stk);

    return EXIT_SUCCESS;
}
