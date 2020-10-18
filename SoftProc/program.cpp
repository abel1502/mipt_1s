#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define CHECKSUM_NOIMPL

#include "program.h"


const STACK_INITIAL_CAPACITY = 8;


bool program_read(program_t *self, FILE *ifile) {
    assert(self != NULL);
    assert(ifile != NULL);

    if (aef_mmap_read(&self->mmap, ifile)) {
        ERR("Couldn't read the program file");
        return true;
    }

    self->ip = 0;
    self->registers = {};

    if (stack_construct(&self->stack, STACK_INITIAL_CAPACITY) == NULL) {
        ERR("Couldn't initialize program stack");
        return true;
    }

    return false;
}

bool program_executeOpcode(program_t *self);

bool program_execute(program_t *self);

void program_free(program_t *self);
