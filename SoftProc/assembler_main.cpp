#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "constants.h"
#include "aef_file.h"
#include "assembler.h"


int main(int argc, char **argv) {
    printf("%zu\n", sizeof(OPARGS));

    code_t testCode = {};
    code_init(&testCode);

    //

    code_free(&testCode);

    /*aef_mmap_t mmap = {};
    assert(aef_mmap_init(&mmap, 8, (char *)"\x12\x34\x56\x78\x9A\xBC\xDE\xF0") != NULL);

    FILE *ofile = fopen("test.aef", "wb");

    assert(!aef_mmap_write(&mmap, ofile));

    fclose(ofile);

    aef_mmap_free(&mmap);*/

    return 0;
}

