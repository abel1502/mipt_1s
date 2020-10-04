#define TEST
#include "../libs/test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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
        test_stack();
        $g; TEST_MSG("Passed All."); $d;
        ,
    )

    stack_t stk = {};
    stack_construct(&stk, 10);

    stack_dump(NULL);

    stack_push(&stk, 1.23);
    stack_push(&stk, 4.56);
    stack_dump(&stk);

    stk.data = NULL;

    stack_free(&stk);
}
