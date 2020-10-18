#ifndef PROCESSOR_H_GUARD
#define PROCESSOR_H_GUARD

#include "general.h"
#include "constants.h"
#include "opcodes.h"
#include "aef_file.h"


typedef struct program_s program_t;

/*typedef struct flag_regs_s flag_regs_t;


struct flag_regs_s {
    char f0 : 1;
    char f1 : 1;
    char f2 : 1;
    char f3 : 1;
    char f4 : 1;
    char f5 : 1;
    char f6 : 1;
    char f7 : 1;
};*/


// ==== [ Stack inclusion ] ====
typedef value_t stack_elem_t;
/*
static void printHelper(stack_elem_t elem) {
    printf("%lg", elem);
}
#define STACK_ELEM_PRINT printHelper
#define STACK_VALIDATION_LEVEL 3
*/
#include "../Stack/stack.h"
// =============================


struct program_s {
    aef_mmap_t mmap;
    stack_t stack;
    value_t registers[GENERAL_REG_CNT];
    //flag_regs_t flags;
    uint32_t ip;
};


bool program_read(program_t *self, FILE *ifile);

bool program_executeOpcode(program_t *self);

bool program_execute(program_t *self);

void program_free(program_t *self);


#endif // PROCESSOR_H_GUARD
