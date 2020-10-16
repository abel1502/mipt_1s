#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "general.h"
#include "assembler.h"


const code_size_t CODE_DEFAULT_CAPACITY = 0x20;
const code_size_t CODE_MAX_CAPACITY = 0x7fff0000;

/*const char *ARG_TYPES[] {
    "qw",
    "df",
    "fl",
    "fh",
    "dwl",
    "wl",
    "bl",
    "bh",
    "wh",
    "dwh"
};*/  // TODO: Add during further development


code_t *code_init(code_t *self) {
    self->capacity = CODE_DEFAULT_CAPACITY;

    self->buf = (char *)calloc(self->capacity, sizeof(self->buf[0]));

    if (self->buf == NULL) {
        return NULL;
    }

    self->size = 0;

    return self;
}

bool code_writeRaw_(code_t *self, const char *data, code_size_t amount) {
    assert(self != NULL);
    assert(self->buf != NULL);
    assert(data != NULL);

    assert(self->capacity + amount >= self->capacity);

    while (self->capacity < self->size + amount) {
        if (code_resize(self, self->capacity * 2)) {
            ERR("Resize failed (probably, amount is too big)");

            return true;
        }
    }

    memcpy(self->buf + self->size, data, amount);

    self->size += amount;

    return false;
}

bool code_assembleLine(code_t *self, const char *line) {
    assert(self != NULL);
    assert(self->buf != NULL);
    assert(line != NULL);

    #define SKIP_SPACE_() while (*line != '\0' && isspace(*line)) line++;

    SKIP_SPACE_();

    int curOpcode = -1;

    for (int i = 0; i < 256; ++i) {
        if (OPNAMES[i] == NULL) {
            continue;
        }

        if (strncmp(line, OPNAMES[i], strlen(OPNAMES[i])) == 0) {
            curOpcode = i;
            break;
        }
    }

    if (curOpcode == -1) {
        ERR("Unknown opcode <%s>", line);
        return true;
    }

    line += strlen(OPNAMES[curOpcode]);

    /*for (int i = 0; i < OPARGS[curOpcode]; ++i) {
        SKIP_SPACE_();

        //char argType[] =

        if (strncmp(line, ARG_STACK, strlen(ARG_STACK)) == 0) {

            break;
        }
    }*/

    SKIP_SPACE_();

    assert(*line == '\0');

    #undef SKIP_SPACE_

    return false;
}

bool code_assembleFile(code_t *self, FILE *ifile);  // TODO: copy Onegin's abstractions

bool code_compileToFile(code_t *self, FILE *ofile);

bool code_resize(code_t *self, code_size_t capacity) {
    assert(self != NULL);
    assert(self->buf != NULL);

    if (capacity <= self->size || capacity >= CODE_MAX_CAPACITY) {
        return true;
    }

    char *newBuf = (char *)realloc(self->buf, capacity * sizeof(self->buf[0]));

    if (newBuf == NULL) {
        return true;
    }

    self->buf = newBuf;

    self->capacity = capacity;

    return false;
}

void code_free(code_t *self) {
    assert(self != NULL);

    free(self->buf);

    self->capacity = 0;
    self->size = 0;
}

