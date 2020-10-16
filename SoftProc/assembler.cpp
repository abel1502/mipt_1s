#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "general.h"
#include "assembler.h"
#include "constants.h"


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


static bool readUntil_(const char **source, char *dest, char until, size_t limit);


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

    unsigned char op = curOpcode;

    line += strlen(OPNAMES[op]);

    SKIP_SPACE_();

    code_writeRaw_(self, (const char *)&op, 1);

    if (*line == '\0') {
        if (OPARG_BITMASK[op] != 0) {
            ERR("Expected an argument for op #%02x", op);
            return true;
        }

        return false;
    }

    char argType[4] = "";

    if (readUntil_(&line, argType, ':', 3)) {
        ERR("Bad argument type");
        return true;
    }

    unsigned char addrMode = 0;

    #define ARGTYPE_CASE_(name, value) if (strcmp(argType, name) == 0) { addrMode |= value << 4; } else

    ARGTYPE_CASE_("df",  ARGTYPE_DF)
    ARGTYPE_CASE_("fl",  ARGTYPE_FL)
    ARGTYPE_CASE_("fh",  ARGTYPE_FH)
    ARGTYPE_CASE_("qw",  ARGTYPE_QW)
    ARGTYPE_CASE_("dwl", ARGTYPE_DWL)
    ARGTYPE_CASE_("dwh", ARGTYPE_DWH)
    ARGTYPE_CASE_("wl",  ARGTYPE_WL)
    ARGTYPE_CASE_("wh",  ARGTYPE_WH)
    ARGTYPE_CASE_("bl",  ARGTYPE_BL)
    ARGTYPE_CASE_("bh",  ARGTYPE_BH)
    {
        ERR("Unknown argument type: <%s>", argType);
        return true;
    }

    #undef ARGTYPE_CASE_

    if (*line == '\0' || *line == ' ' || strncmp(line, "stack", 5) == 0) {
        addrMode |= ARGLOC_STACK << 2;

        code_writeRaw_(self, (const char *)&addrMode, 1);

        while (*line != '\0' && !isspace(*line)) ++line;
    } else if (*line == 'r') {
        addrMode |= ARGLOC_REG << 2;

        code_writeRaw_(self, (const char *)&addrMode, 1);

        line++;

        if (!('a' <= *line && *line <= 'a' + GENERAL_REG_CNT)) {
            ERR("Unknown register <r%c>", *line);
            return true;
        }

        addrMode |= (*line - 'a') & 0b11;

        line++;
    } else {
        addrMode |= ARGLOC_IMM << 2;

        code_writeRaw_(self, (const char *)&addrMode, 1);

        switch ((addrMode >> 4) & 0x1111) {
            // TODO
        default:
            ERR("Cannot handle constants of <%s> type", argType);
            return true;
        }
    }

    if (*line != '\0' && !isspace(*line)) {
        ERR("Garbage after argument: <%s>", line);
        return true;
    }

    SKIP_SPACE_();

    if (*line != '\0') {
        ERR("Garbage at the end of line: <%s>", line);
        return true;
    }

    return false;

    #undef SKIP_SPACE_
}

bool code_assembleFile(code_t *self, FILE *ifile);  // TODO: copy Onegin's abstractions

bool code_compileToFile(code_t *self, FILE *ofile) {
    assert(self != NULL);
    assert(ofile != NULL);

    aef_mmap_t mmap = {};
    aef_mmap_init(&mmap, self->size, self->buf);

    if (aef_mmap_write(&mmap, ofile)) {
        ERR("Couldn't write compiled bytecode to file");
        return true;
    }

    aef_mmap_free(&mmap);

    return false;
}

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

bool readUntil_(const char **source, char *dest, char until, size_t limit) {
    assert(source != NULL);
    assert(dest != NULL);
    assert(until != '\0');

    for (size_t i = 0; i < limit && **source != '\0'; ++i) {
        *dest = **source;

        if (*dest == until) {
            ++*source;
            *dest = '\0';  // TODO?: *++dest = '\0';
            return false;
        }

        ++dest;
        ++*source;
    }

    return true;
}

