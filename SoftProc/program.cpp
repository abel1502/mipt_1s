#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

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

bool program_parseOpcode(program_t *self, opcode_info_t *opcode) {
    assert(self != NULL);
    assert(opcode != NULL);

    if (self->ip >= self->mmap.header.codeSize) {
        ERR("Instruction pointer out of bounds");
        return true;
    }

    opcode->addr = self->ip;

    if (readByte_(self, (uint8_t *)&opcode->op)) {
        ERR("Reached end of code unexpectedly");
        return true;
    }

    opcode->hasArg = (OPARG_BITMASK[opcode->op] != 0);

    uint8_t curAddrMode = 0;

    if (opcode->hasArg) {
        if (readByte_(self, &curAddrMode)) {
            ERR("Reached end of code unexpectedly");
            return true;
        }

        if (!(OPARG_BITMASK[opcode->op] & 1 << (curAddrMode >> 2))) {
            ERR("Illegal address mode 0x%02x for opcode 0x%02x at 0x%08x", curAddrMode, opcode->op, opcode->addr);
            return true;
        }

        opcode->argLoc = (argLoc_t)(curAddrMode >> 2 & 0b11);
        opcode->argType = (argType_t)(curAddrMode >> 4 & 0b1111);

        switch (opcode->argLoc) {
        case ARGLOC_STACK:
            break;
        case ARGLOC_REG:
            opcode->arg.bl = curAddrMode & 0b11;
            break;
        case ARGLOC_IMM:
            #define ARGTYPE_CASE_(typeCap, typeLow) \
                case ARGTYPE_##typeCap: \
                    readBytes_(self, sizeof(opcode->arg.typeLow), &opcode->arg.typeLow); \
                    break;

            switch (opcode->argType) {
                ARGTYPE_CASE_(DF, df)
                ARGTYPE_CASE_(FL, fl)
                ARGTYPE_CASE_(FH, fh)
                ARGTYPE_CASE_(QW, qw)
                ARGTYPE_CASE_(DWL, dwl)
                ARGTYPE_CASE_(DWH, dwh)
                ARGTYPE_CASE_(WL, wl)
                ARGTYPE_CASE_(WH, wh)
                ARGTYPE_CASE_(BL, bl)
                ARGTYPE_CASE_(BH, bh)
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

    return false;
}

bool program_executeOpcode(program_t *self) {
    assert(self != NULL);

    //printf("[PRC] 0x%08x |\n", self->ip);

    opcode_info_t curOp = {};

    program_parseOpcode(self, &curOp);

    /*printf("[PRC] %02x (0x%016llx) ", curOp, OPARG_BITMASK[curOp]);
    if (hasArg) {
        printf("%02x %016llx ", curAddrMode, curArg.qw);
        if (curArgType == ARGTYPE_DF && curArgLoc == ARGLOC_IMM) {
            printf("(%lg) ", curArg.df);
        }
    }
    printf("\n");*/

    #define TMP_ONLYDOUBLE_  if (curOp.argType != ARGTYPE_DF) { goto notimpl_label; }

    #define POP_(dest)  if (stack_pop(&self->stack, (dest))) { ERR("Cannot pop from stack"); return true; }
    #define PUSH_(val)  if (stack_push(&self->stack, (val))) { ERR("Cannot push to stack"); return true; }

    value_t tos0 = {}, tos1 = {}, res = {};

    switch (curOp.op) {
    case OP_NOP:
        break;
    case OP_PUSH:
        if (curOp.argLoc == ARGLOC_REG) {
            curOp.arg = self->registers[curOp.arg.bl];
        }
        PUSH_(curOp.arg);
        break;
    case OP_POP:
        POP_(&self->registers[curOp.arg.bl]);
        break;
    case OP_POPV:
        POP_(NULL);
        break;
    case OP_ADD: TMP_ONLYDOUBLE_
        POP_(&tos0);
        POP_(&tos1);
        res.df = tos1.df + tos0.df;
        PUSH_(res);
        break;
    case OP_SUB: TMP_ONLYDOUBLE_
        POP_(&tos0);
        POP_(&tos1);
        res.df = tos1.df - tos0.df;
        PUSH_(res);
        break;
    case OP_MUL: TMP_ONLYDOUBLE_
        POP_(&tos0);
        POP_(&tos1);
        res.df = tos1.df * tos0.df;
        PUSH_(res);
        break;
    case OP_DIV: TMP_ONLYDOUBLE_
        POP_(&tos0);
        POP_(&tos1);
        res.df = tos1.df / tos0.df;  // Zero division produces inf
        PUSH_(res);
        break;
    case OP_SQR:
        POP_(&tos0);
        res.df = tos0.df * tos0.df;
        PUSH_(res);
        break;
    case OP_SQRT:
        POP_(&tos0);
        res.df = sqrt(tos0.df);
        PUSH_(res);
        break;
    case OP_IN: TMP_ONLYDOUBLE_
        printf("(df) > ");
        switch (curOp.argLoc) {
        case ARGLOC_STACK:
            scanf("%lg", &res.df);
            PUSH_(res);
            break;
        case ARGLOC_REG:
            scanf("%lg", &self->registers[curOp.arg.bl].df);
            break;
        case ARGLOC_IMM:
        default:
            ERR("Shouldn't be reachable");
            abort();
        }

        break;
    case OP_OUT: TMP_ONLYDOUBLE_
        printf("(df) ");
        switch (curOp.argLoc) {
        case ARGLOC_STACK:
            POP_(&tos0);
            printf("%lg\n", tos0.df);
            break;
        case ARGLOC_REG:
            printf("%lg\n", self->registers[curOp.arg.bl].df);
            break;
        case ARGLOC_IMM:
            printf("%lg\n", curOp.arg.df);
            break;
        default:
            ERR("Shouldn't be reachable");
            abort();
        }

        break;
    case OP_END:
        self->flags.flag_exit = true;
        break;
    case OP_DUMP:
        program_dump(self);
        break;
    default:
        ERR("Unknown opcode 0x%02x", curOp.op);  // TODO: opcode names?
        return true;
    notimpl_label:
        ERR("Sorry, opcode 0x%02x (argType 0b%d%d%d%d, argLoc 0b%d%d) not yet implemented", curOp.op, curOp.argType >> 3 & 1, curOp.argType >> 2 & 1, curOp.argType >> 1 & 1, curOp.argType >> 0 & 1, curOp.argLoc >> 1 & 1, curOp.argLoc >> 0 & 1);
        return true;
    }

    #undef POP_
    #undef PUSH_

    #undef TMP_ONLYDOUBLE_

    return false;
}

bool program_execute(program_t *self) {
    assert(self != NULL);

    while (!self->flags.flag_exit) {
        if (program_executeOpcode(self)) {
            ERR("Error during execution of opcode at 0x%08x", self->ip);
            if (self->ip == self->mmap.header.codeSize) {
                ERR("(Most likely your program is missing an <end> opcode in the end)");
            }
            return true;
        }
    }

    return false;
}

bool program_disassembleOpcode(program_t *self) {
    assert(self != NULL);

    printf("[DIS] 0x%08x | ", self->ip);

    opcode_info_t curOp = {};

    program_parseOpcode(self, &curOp);

    if (OPNAMES[curOp.op] == NULL) {
        printf("<corrupt>\n");
        return true;
    }

    printf("%s ", OPNAMES[curOp.op]);

    if (curOp.hasArg) {
        #define ARGTYPE_CASE_(typeCap, typeLow) \
            case ARGTYPE_##typeCap: \
                printf(#typeLow ":"); \
                break;

        switch (curOp.argType) {
            ARGTYPE_CASE_(DF, df)
            ARGTYPE_CASE_(FL, fl)
            ARGTYPE_CASE_(FH, fh)
            ARGTYPE_CASE_(QW, qw)
            ARGTYPE_CASE_(DWL, dwl)
            ARGTYPE_CASE_(DWH, dwh)
            ARGTYPE_CASE_(WL, wl)
            ARGTYPE_CASE_(WH, wh)
            ARGTYPE_CASE_(BL, bl)
            ARGTYPE_CASE_(BH, bh)
        default:
            ERR("Shouldn't be reachable");
            abort();
        }

        #undef ARGTYPE_CASE_

        switch (curOp.argLoc) {
        case ARGLOC_STACK:
            printf("stack");
            break;
        case ARGLOC_REG:
            printf("r%c", 'a' + curOp.arg.bl);
            break;
        case ARGLOC_IMM:
            #define ARGTYPE_CASE_(typeCap, typeLow, fmt) \
                case ARGTYPE_##typeCap:                  \
                    printf(fmt, curOp.arg.typeLow);      \
                    break;

            switch (curOp.argType) {
                ARGTYPE_CASE_(DF, df, "%lg")
                ARGTYPE_CASE_(FL, fl, "%g")
                ARGTYPE_CASE_(FH, fh, "%g")
                ARGTYPE_CASE_(QW, qw, "%llu")
                ARGTYPE_CASE_(DWL, dwl, "%u")
                ARGTYPE_CASE_(DWH, dwh, "%u")
                ARGTYPE_CASE_(WL, wl, "%hu")
                ARGTYPE_CASE_(WH, wh, "%hu")
                ARGTYPE_CASE_(BL, bl, "%hhu")
                ARGTYPE_CASE_(BH, bh, "%hhu")
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

    printf("\n");



    return false;
}

bool program_disassemble(program_t *self) {
    assert(self != NULL);

    while (self->ip < self->mmap.header.codeSize) {
        if (program_disassembleOpcode(self)) {
            ERR("Error during disassembly of opcode at 0x%08x", self->ip);

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

void program_dump(program_t *self) {
    printf("program_t [0x%p] {\n", self);
    if (self != NULL) {
        printf("  ip     = %u (out of %u)\n", self->ip, self->mmap.header.codeSize);
        printf("  flags  = 0b%d%d%d%d%d%d%d%d\n", self->flags.flag_exit, self->flags.f1, self->flags.f2, self->flags.f3, self->flags.f4, self->flags.f5, self->flags.f6, self->flags.f7);

        printf("  registers [0x%p] {\n", self->registers);
        if (self->registers != NULL) {
            for (unsigned char i = 0; i < GENERAL_REG_CNT; ++i) {
                printf("    r%c = 0x%016llX (%lg)\n", 'a' + i, self->registers[i].qw, self->registers[i].df);
            }
        } else {
            printf("    <corrupt>\n");
        }
        printf("  }\n");

        stack_dumpPadded(&self->stack, "  ");
    } else {
        printf("  <corrupt>\n");
    }

    printf("}\n\n");

    fflush(stdout);
    fflush(stderr);
}

