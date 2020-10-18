#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define CHECKSUM_NOIMPL

#include "program.h"


const size_t STACK_INITIAL_CAPACITY = 8;


static inline bool readByte_(program_t *self, uint8_t *dest);

static inline bool readBytes_(program_t *self, uint32_t size, void *dest);


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
    assert(self != NULL);

    //printf("[PRC] 0x%08x |\n", self->ip);

    if (self->ip >= self->mmap.header.codeSize) {
        ERR("Instruction pointer out of bounds");
        return true;
    }

    opcode_t curOp = (opcode_t)0;

    if (readByte_(self, (uint8_t *)&curOp)) {
        ERR("Reached end of code unexpectedly");
        return true;
    }

    bool hasArg = OPARG_BITMASK[curOp] != 0;

    uint8_t curAddrMode = 0;

    argLoc_t curArgLoc = (argLoc_t)0;
    argType_t curArgType = (argType_t)0;

    value_t curArg = {};

    if (hasArg) {
        if (readByte_(self, &curAddrMode)) {
            ERR("Reached end of code unexpectedly");
            return true;
        }

        if (!(OPARG_BITMASK[curOp] & 1 << (curAddrMode >> 2))) {
            //ERR("0x%016llx 0x%02x", OPARG_BITMASK[curOp], curAddrMode >> 2);
            ERR("Illegal address mode 0x%02x for opcode 0x%02x at 0x%08x", curAddrMode, curOp, self->ip - 2);
            return true;
        }

        curArgLoc = (argLoc_t)(curAddrMode >> 2 & 0b11);
        curArgType = (argType_t)(curAddrMode >> 4 & 0b1111);

        switch (curArgLoc) {
        case ARGLOC_STACK:
            break;
        case ARGLOC_REG:
            curArg.bl = curAddrMode & 0x11;
            break;
        case ARGLOC_IMM:
            #define ARGTYPE_CASE_(typeCap, typeLow) \
                case ARGTYPE_##typeCap: \
                    readBytes_(self, sizeof(curArg.typeLow), &curArg.typeLow); \
                    break;

            switch (curArgType) {
                ARGTYPE_CASE_(DF, df);
                ARGTYPE_CASE_(FL, fl);
                ARGTYPE_CASE_(FH, fh);
                ARGTYPE_CASE_(QW, qw);
                ARGTYPE_CASE_(DWL, dwl);
                ARGTYPE_CASE_(DWH, dwh);
                ARGTYPE_CASE_(WL, wl);
                ARGTYPE_CASE_(WH, wh);
                ARGTYPE_CASE_(BL, bl);
                ARGTYPE_CASE_(BH, bh);
            default:
                ERR("Shouldn't be reachable");
                abort();
            }

            #undef ARGTYPE_CASE_

            break;
        default:
            ERR("Shouldn't be reachable");
            abort();
        }
    }

    /*printf("[PRC] %02x (0x%016llx) ", curOp, OPARG_BITMASK[curOp]);
    if (hasArg) {
        printf("%02x %016llx ", curAddrMode, curArg.qw);
        if (curArgType == ARGTYPE_DF && curArgLoc == ARGLOC_IMM) {
            printf("(%lg) ", curArg.df);
        }
    }
    printf("\n");*/

    return false;

    #undef READ_BYTE_
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

static inline bool readByte_(program_t *self, uint8_t *dest) {
    if (self->ip + 1 > self->mmap.header.codeSize) {
        return true;
    }

    *dest = self->mmap.code[self->ip++];

    return false;
}

static inline bool readBytes_(program_t *self, uint32_t size, void *dest) {
    if (self->ip + size > self->mmap.header.codeSize) {
        return true;
    }

    memcpy(dest, self->mmap.code + self->ip, size);  // TODO: Replace with a switch of default types for speed

    self->ip += size;

    return false;
}

