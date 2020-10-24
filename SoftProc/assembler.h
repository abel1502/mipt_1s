#ifndef ASSEMBLER_H_GUARD
#define ASSEMBLER_H_GUARD

#include "aef_file.h"


typedef struct code_s code_t;

typedef struct label_s label_t;


struct code_s {
    code_size_t size;
    code_size_t capacity;
    char *buf;

    code_size_t lineStart;

    bool doLog;

    code_size_t labelCnt;
    label_t *labels;

    bool labelsInited;
};

struct label_s {
    const char *val;
    unsigned len;
    code_size_t offset;
};


code_t *code_init(code_t *self, bool doLog);

bool code_writeRaw_(code_t *self, const void *data, code_size_t amount);

bool code_assembleLine(code_t *self, const char *line);

bool code_assembleFile(code_t *self, FILE *ifile);

bool code_readConst_(code_t *self, const char **line, void *valueBuf, uint8_t argType);

bool code_compileToFile(code_t *self, FILE *ofile);

bool code_resize(code_t *self, code_size_t capacity);

void code_free(code_t *self);

void code_log(code_t *self, const char *fmt, ...);

void code_logLine(code_t *self, const char *line);

bool code_readLabel(code_t *self, const char **line);

bool code_lookupLabel(code_t *self, const char **line, code_size_t *offset);


#endif // ASSEMBLER_H_GUARD
