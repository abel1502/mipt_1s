#ifndef ASSEMBLER_H_GUARD
#define ASSEMBLER_H_GUARD

#include "opcodes.h"
#include "aef_file.h"


typedef struct code_s code_t;


struct code_s {
    code_size_t size;
    code_size_t capacity;
    char *buf;
    bool doLog;
};


code_t *code_init(code_t *self, bool doLog);

bool code_writeRaw_(code_t *self, const char *data, code_size_t amount);

bool code_assembleLine(code_t *self, const char *line);

bool code_assembleFile(code_t *self, FILE *ifile);  // TODO: copy Onegin's abstractions

bool code_compileToFile(code_t *self, FILE *ofile);

bool code_resize(code_t *self, code_size_t capacity);

void code_free(code_t *self);

void code_log(code_t *self, const char *fmt, ...);


#endif // ASSEMBLER_H_GUARD
