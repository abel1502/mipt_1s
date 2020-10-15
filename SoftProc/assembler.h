#ifndef ASSEMBLER_H_GUARD
#define ASSEMBLER_H_GUARD

#include "opcodes.h"
#include "aef_file.h"


typedef struct code_s code_t;


struct code_s {
    code_size_t size;
    code_size_t capacity;
    char *buf;
};


code_t *code_init(code_t *self);

code_size_t code_assembleLine(code_t *self, const char *line);

bool code_assembleFile(code_t *self, FILE *ifile);  // TODO: copy Onegin's abstractions

bool code_write(code_t *self, FILE *ofile);

void code_free(code_t *self);


#endif // ASSEMBLER_H_GUARD
