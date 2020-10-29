#ifndef PROCESSOR_H_GUARD
#define PROCESSOR_H_GUARD

#include "general.h"
#include "constants.h"
#include "opcode.h"
#include "aef_file.h"


typedef struct program_s program_t;

typedef struct opcode_info_s opcode_info_t;

typedef struct flag_regs_s flag_regs_t;


struct flag_regs_s {
    bool flag_exit : 1;
    bool flag_monday : 1;
    bool flag_trace : 1;
    bool f3 : 1;
    bool f4 : 1;
    bool f5 : 1;
    bool f6 : 1;
    bool f7 : 1;
};


// ==== [ Stack inclusion ] ====
typedef value_t stack_elem_t;
#ifndef STACK_NOIMPL
static void printHelper(stack_elem_t elem) {
    printf("%lg", elem.df);
}
#define STACK_ELEM_PRINT printHelper
#endif
#include "../Stack/stack.h"
// =============================


struct program_s {
    aef_mmap_t mmap;
    stack_t stack;
    stack_t frameStack;
    value_t registers[GENERAL_REG_CNT];
    flag_regs_t flags;
    code_size_t ip;
    void *ram;
};

struct opcode_info_s {
    code_size_t addr;
    opcode_t op;
    addrMode_t addrMode;
    uint8_t reg;
    value_t arg;
    code_size_t memAddr;
};


bool program_read(program_t *self, FILE *ifile);

bool program_executeOpcode(program_t *self);

void program_checkMonday(program_t *self);

bool program_drawScreen(program_t *self, code_size_t addr);

bool program_execute(program_t *self);

char program_ramReadByte(program_t *self, code_size_t addr);

void program_ramWriteByte(program_t *self, code_size_t addr, char byte);

char *program_ramReadBytes(program_t *self, code_size_t addr, code_size_t size, void *dest);

void program_ramWriteBytes(program_t *self, code_size_t addr, code_size_t size, const void *source);

void program_free(program_t *self);

void program_dump(program_t *self);


#endif // PROCESSOR_H_GUARD
