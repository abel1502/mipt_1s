#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#define CHECKSUM_NOIMPL

#include "general.h"
#include "assembler.h"
#include "constants.h"
#include "textfile.h"
#include "opcodes.h"


const code_size_t CODE_DEFAULT_CAPACITY = 0x20;
const code_size_t CODE_MAX_CAPACITY = 0x7fff0000;


static bool readUntil_(const char **source, char *dest, char until, size_t limit);


code_t *code_init(code_t *self, bool doLog) {
    self->capacity = CODE_DEFAULT_CAPACITY;

    self->buf = (char *)calloc(self->capacity, sizeof(self->buf[0]));

    if (self->buf == NULL) {
        return NULL;
    }

    self->size = 0;

    self->doLog = doLog;

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

    const char *origLine = line;

    #define EOL_ (*line == '\0' || *line == ';')

    #define SKIP_SPACE_() while (!EOL_ && isspace(*line)) line++;

    SKIP_SPACE_();

    if (EOL_) {
        return false;  // Empty lines are ok
    }

    code_log(self, "[ASM] | 0x%04x | ", self->size);

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

    code_log(self, "%02x ", op);

    if (code_writeRaw_(self, (const char *)&op, 1)) {
        ERR("Couldn't write to file");
        return true;
    }

    if (EOL_) {
        if (OPARG_BITMASK[op] != 0) {
            ERR("Expected an argument for op #%02x", op);
            return true;
        }

        code_log(self, "                           |            | \"%s\"\n", origLine);

        return false;
    }

    char argType[4] = "";

    if (readUntil_(&line, argType, ':', 3)) {
        ERR("Bad argument type");
        return true;
    }

    unsigned char addrMode = 0;

    #define ARGTYPE_CASE_(name, value)  if (strcmp(argType, name) == 0) { addrMode |= value << 4; } else

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

    if (EOL_ || isspace(*line) || strncmp(line, "stack", 5) == 0) {
        addrMode |= ARGLOC_STACK << 2;

        code_log(self, "%02x                         ", addrMode);

        code_writeRaw_(self, (const char *)&addrMode, 1);

        while (!EOL_ && !isspace(*line)) ++line;
    } else if (*line == 'r') {
        addrMode |= ARGLOC_REG << 2;

        line++;

        if (!('a' <= *line && *line <= 'a' + GENERAL_REG_CNT)) {
            ERR("Unknown register <r%c>", *line);
            return true;
        }

        addrMode |= (*line - 'a') & 0b11;

        code_log(self, "%02x                         ", addrMode);

        if (code_writeRaw_(self, (const char *)&addrMode, 1)) {
            ERR("Couldn't write to file");
            return true;
        }
        line++;
    } else {
        addrMode |= ARGLOC_IMM << 2;

        code_log(self, "%02x ", addrMode);

        if (code_writeRaw_(self, (const char *)&addrMode, 1)) {
            ERR("Couldn't write to file");
            return true;
        }

        char opArg[sizeof(value_t)] = "";
        code_size_t opArgSize = 0;

        int lineDelta = 0;
        int res = 0;

        #define ARGTYPE_CASE_(type, format)                                  \
                res = sscanf(line, format "%n", (type *)&opArg, &lineDelta); \
                                                                             \
                if (res != 1) {                                              \
                    ERR("Corrupt immediate value: <%s>", line);              \
                    return true;                                             \
                }                                                            \
                                                                             \
                line += lineDelta;                                           \
                opArgSize = sizeof(type);                                    \
                                                                             \
                break;

        #define ARGTYPE_CASE_SIGN_(utype, stype, format)                           \
                if (*line == '+' || *line == '-') {                                \
                    line++;                                                        \
                    res = sscanf(line, format "u%n", (utype *)&opArg, &lineDelta); \
                } else {                                                           \
                    res = sscanf(line, format "d%n", (stype *)&opArg, &lineDelta); \
                }                                                                  \
                                                                                   \
                if (res != 1) {                                                    \
                    ERR("Corrupt immediate value: <%s>", line);                    \
                    return true;                                                   \
                }                                                                  \
                                                                                   \
                line += lineDelta;                                                 \
                opArgSize = sizeof(utype);                                         \
                                                                                   \
                break;

        if (EOL_) {
            ERR("Expected an argument value");
            return true;
        }

        switch ((addrMode >> 4) & 0b1111) {
            // ARGTYPE_DF ARGTYPE_FL ARGTYPE_FH ARGTYPE_QW ARGTYPE_DWL ARGTYPE_DWH ARGTYPE_WL ARGTYPE_WH ARGTYPE_BL ARGTYPE_BH
        case ARGTYPE_DF:
            ARGTYPE_CASE_(double, "%lg")
        case ARGTYPE_FL:
        case ARGTYPE_FH:
            ARGTYPE_CASE_(float, "%g")
        case ARGTYPE_QW:
            ARGTYPE_CASE_SIGN_(uint64_t, int64_t, "%ll")
        case ARGTYPE_DWL:
        case ARGTYPE_DWH:
            ARGTYPE_CASE_SIGN_(uint32_t, int32_t, "%")
        case ARGTYPE_WL:
        case ARGTYPE_WH:
            ARGTYPE_CASE_SIGN_(uint16_t, int16_t, "%h")
        case ARGTYPE_BL:
        case ARGTYPE_BH:
            ARGTYPE_CASE_SIGN_(uint8_t, int8_t, "%hh")
        default:
            ERR("Cannot handle constants of <%s> type", argType);
            return true;
        }

        #undef ARGTYPE_CASE_SIGN_
        #undef ARGTYPE_CASE_

        for (size_t i = 0; i < sizeof(value_t); ++i) {
            if (i < self->size) {
                code_log(self, "%02x ", opArg[i]);
            } else {
                code_log(self, "   ");
            }
        }

        if (code_writeRaw_(self, opArg, opArgSize))  {
            ERR("Couldn't write to file");
            return true;
        }
    }

    code_log(self, "| %u%u%u%u %u%u %u%u | \"%s\"\n", addrMode >> 7 & 1, addrMode >> 6 & 1, addrMode >> 5 & 1, addrMode >> 4 & 1, addrMode >> 3 & 1, addrMode >> 2 & 1, addrMode >> 1 & 1, addrMode >> 0 & 1, origLine);

    if (!EOL_ && !isspace(*line)) {
        ERR("Garbage after argument: <%s>", line);
        return true;
    }

    SKIP_SPACE_();

    if (!EOL_) {
        ERR("Garbage at the end of line: <%s>", line);
        return true;
    }

    return false;

    #undef EOL_
    #undef SKIP_SPACE_
}

bool code_assembleFile(code_t *self, FILE *ifile) {
    assert(self != NULL);
    assert(ifile != NULL);

    text_t itext = {};

    if (text_read(&itext, ifile)) {
        ERR("Can't read ifile text");
        return true;
    }

    for (unsigned int i = 0; i < itext.length; ++i) {
        if (code_assembleLine(self, (const char *)itext.index[i].val)) {  // TODO: Rework assembleLine to work with line_t
            ERR("Couldn't assemble line #%u", i);  // TODO?: i + 1
            return true;
        }
    }


    text_free(&itext);

    return false;
}

bool code_compileToFile(code_t *self, FILE *ofile) {
    assert(self != NULL);
    assert(ofile != NULL);

    aef_mmap_t mmap = {};
    aef_mmap_init(&mmap, self->size, self->buf);

    code_log(self, "[ASM] Total size: 0x%04x\n\n", self->size);

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
    // Warning: if until is an EOL character ('\0' or ';'), this will return true since EOL will be reached

    for (size_t i = 0; i < limit && **source != '\0' && **source != ';'; ++i) {
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


void code_log(code_t *self, const char *fmt, ...) {
    assert(self != NULL);
    assert(fmt != NULL);

    if (!self->doLog) {
        return;
    }

    va_list args;

    va_start(args, fmt);

    vprintf(fmt, args);

    va_end(args);
}

