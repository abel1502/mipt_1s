#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#define CHECKSUM_NOIMPL

#include "program.h"


const size_t STACK_INITIAL_CAPACITY = 8;


static inline bool readByte_(program_t *self, uint8_t *dest);

static inline bool readBytes_(program_t *self, uint32_t size, void *dest);

static bool readImm_(program_t *self, opcode_info_t *opcode);

static bool readArg_(program_t *self, opcode_info_t *opcode);


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

    if (stack_construct(&self->frameStack, STACK_INITIAL_CAPACITY) == NULL) {
        ERR("Couldn't initialize frame stack");
        return true;
    }

    return false;
}

bool program_executeOpcode(program_t *self) {
    assert(self != NULL);

    opcode_info_t curOp = {};

    if (self->ip >= self->mmap.header.codeSize) {
        ERR("Instruction pointer out of bounds");
        return true;
    }

    curOp.addr = self->ip;

    if (readByte_(self, (uint8_t *)&curOp.op)) {
        ERR("Reached end of code unexpectedly");
        return true;
    }

    if (self->flags.flag_trace) {
        printf("[PRC] OP 0x%02x\n", curOp.op);
    }

    #define NOTIMPL_  goto notimpl_label;
    #define TMP_ONLYDOUBLE_  if (curOp.addrMode.type != ARGTYPE_DF) { NOTIMPL_ }\

    #define POP_(dest)  if (stack_pop(&self->stack, (dest))) { ERR("Cannot pop from stack"); return true; }
    #define PUSH_(val)  if (stack_push(&self->stack, (val))) { ERR("Cannot push to stack"); return true; }
    #define POP_FRAME_(dest)  if (stack_pop(&self->frameStack, (dest))) { ERR("Cannot pop from frame stack"); return true; }
    #define PUSH_FRAME_(val)  if (stack_push(&self->frameStack, (val))) { ERR("Cannot push to frame stack"); return true; }

    #define AM_   (curOp.addrMode)
    #define ARG_  (curOp.arg)

    value_t tos0 = {}, tos1 = {}, tos2 = {}, tos3 = {}, res = {};

    #define DEF_OP(NUM, NAME_CAP, NAME_LOW, ARG_CNT, ARG_TYPE_MASK, ARG_LOC_MASK, CODE) \
        case OP_##NAME_CAP: \
            if (ARG_CNT != 0 && readArg_(self, &curOp)) { \
                ERR("Coudn't read opcode argument"); \
                return true; \
            } \
            if (ARG_CNT != 0 && (!(ARG_TYPE_MASK & 1 << curOp.addrMode.type) || !(ARG_LOC_MASK & 1 << curOp.addrMode.loc))) { \
                ERR("Inappropriate argument for opcode 0x%02hhx", curOp.op); \
                return true; \
            } \
            CODE \
            break;

    switch (curOp.op) {
        #include "opcode_defs.h"
    default:
        ERR("Unknown opcode 0x%02x", curOp.op);  // TODO: opcode names?
        return true;
    notimpl_label:
        ERR("Sorry, opcode 0x%02x (addrMode 0x%02x) not yet implemented", curOp.op, curOp.addrMode.all);
        return true;
    }

    #undef DEF_OP

    #undef AM_
    #undef ARG_

    #undef POP_
    #undef PUSH_
    #undef POP_FRAME_
    #undef PUSH_FRAME_

    #undef TMP_ONLYDOUBLE_
    #undef NOTIMPL_

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

/*bool program_disassembleOpcode(program_t *self) {
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
}*/

void program_free(program_t *self) {
    assert(self != NULL);

    stack_free(&self->stack);

    stack_free(&self->frameStack);
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

static bool readImm_(program_t *self, opcode_info_t *opcode) {
    #define ARGTYPE_CASE_(NAME_CAP, NAME_LOW, TYPE, FMT_U, FMT_S) \
        case ARGTYPE_##NAME_CAP: \
            readBytes_(self, sizeof(opcode->arg.NAME_LOW), &opcode->arg.NAME_LOW); \
            break;

    ARGTYPE_SWITCH_(
        ERR("Inexistent argType: 0x%01x", opcode->addrMode.type);
        return true;
    )

    return false;

    #undef ARGTYPE_CASE_
}

static bool readArg_(program_t *self, opcode_info_t *opcode) {
    assert(self != NULL);
    assert(opcode != NULL);

    if (readByte_(self, (uint8_t *)&opcode->addrMode)) {
        ERR("Reached end of code unexpectedly");
        return true;
    }

    opcode->arg = {};

    opcode->reg = 0xff;

    if (opcode->addrMode.locReg && readByte_(self, &opcode->reg)) {
        ERR("Couldn't read register number for the opcode argument");
        return true;
    }

    if (opcode->reg != 0xff && opcode->reg > GENERAL_REG_CNT) {
        ERR("Bad register number");
        return true;
    }

    if (opcode->addrMode.locImm && readImm_(self, opcode)) {
        ERR("Couldn't read immediate value for the opcode argument");
        return true;
    }

    if (opcode->reg != 0xff) {
        #define ARGTYPE_CASE_(NAME_CAP, NAME_LOW, TYPE, FMT_U, FMT_S) \
            case ARGTYPE_##NAME_CAP: \
                opcode->arg.NAME_LOW += self->registers[opcode->reg].NAME_LOW; \
                break;

        ARGTYPE_SWITCH_(
            ERR("Inexistent argType: 0x%01x", opcode->addrMode.type);
            return true;
        )

        #undef ARGTYPE_CASE_
    }

    return false;
}

void program_dump(program_t *self) {
    printf("program_t [0x%p] {\n", self);
    if (self != NULL) {
        printf("  ip     = %u (out of %u)\n", self->ip, self->mmap.header.codeSize);
        printf("  flags  = 0b%d%d%d%d%d%d%d%d\n", self->flags.flag_exit, self->flags.flag_monday, self->flags.flag_trace, self->flags.f3, self->flags.f4, self->flags.f5, self->flags.f6, self->flags.f7);

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

