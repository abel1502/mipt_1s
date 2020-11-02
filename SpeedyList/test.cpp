#include "../libs/test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>


// ==== [ Stack inclusion ] ====
typedef double list_elem_t;
static void printHelper(list_elem_t elem) {
    printf("%lg", elem);
}
#define LIST_ELEM_PRINT printHelper
#define LIST_VALIDATION_LEVEL 3
#include "list.h"
// =============================

//--------------------------------------------------------------------------------

int main() {
    TEST_MAIN(
        ,
        test_list(1.23, 4.56, 7.89);
        $g; TEST_MSG("Passed All."); $d;
        ,
    )

    return EXIT_SUCCESS;
}

