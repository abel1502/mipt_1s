#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define CHECKSUM_NOIMPL

#include "program.h"


const size_t STACK_INITIAL_CAPACITY = 8;


bool program_read(program_t *self, FILE *ifile) {
    assert(self != NULL);
    assert(ifile != NULL);

    if (aef_mmap_read(&self->mmap, ifile)) {
        ERR("Couldn't read the program file");
        return true;
    }

    self->ip = 0;

    // self->registers = {} didn't work
    memset(self->registers, 0, sizeof(self->registers[0]) * GENERAL_REG_CNT);

    self->flags = {};

    if (stack_construct(&self->stack, STACK_INITIAL_CAPACITY) == NULL) {
        ERR("Couldn't initialize program stack");
        return true;
    }

    return false;
}

bool program_executeOpcode(program_t *self) {
    return false;
}

bool program_execute(program_t *self) {
    assert(self != NULL);

    while (!self->flags.flag_exit) {
        if (program_executeOpcode(self)) {
            ERR("Error during execution of opcode at 0x%x", self->ip);
            if (self->ip == self->mmap.header.codeSize) {
                ERR("(Most likely your program is missing an <end> opcode in the end)");
            }
            return true;
        }
    }

    return false;
}

void program_free(program_t *self) {
    assert(self != NULL);

    stack_free(&self->stack);
}
