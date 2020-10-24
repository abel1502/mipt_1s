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
#include "opcode.h"


const code_size_t CODE_DEFAULT_CAPACITY = 0x20;
const code_size_t CODE_MAX_CAPACITY = 0x7fff0000;


static bool readUntil_(const char **source, char *dest, char until, size_t limit);

static inline void skipSpace_(const char **line);

static inline bool isEOL_(const char *line);

static bool readConst_(const char **line, void *valueBuf, uint8_t argType);


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

bool code_writeRaw_(code_t *self, const void *data, code_size_t amount) {
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

    skipSpace_(&line);

    if (isEOL_(line)) {
        return false;  // Empty lines are ok
    }

    code_log(self, "[ASM] | 0x%04x | ", self->size);  // TODO?: 0x%08x

    opcode_t op = OP_NOP;
    int curNameLen = 0;

    unsigned curArgCnt = 0;
    uint16_t curArgTypeMask = 0;
    uint8_t curArgLocMask = 0;

    do {
        #define DEF_OP(NUM, NAME_CAP, NAME_LOW, ARG_CNT, ARG_TYPE_MASK, ARG_LOC_MASK, CODE) \
            curNameLen = 0; \
            sscanf(line, #NAME_LOW "%n", &curNameLen); \
            if (curNameLen == strlen(#NAME_LOW) && (isspace(*(line + curNameLen)) || isEOL_(line + curNameLen))) { \
                op = OP_##NAME_CAP; \
                line += curNameLen; \
                curArgCnt = ARG_CNT; \
                curArgTypeMask = ARG_TYPE_MASK; \
                curArgLocMask = ARG_LOC_MASK; \
                break; \
            }

        #include "opcode_defs.h"

        #undef DEF_OP

        ERR("Unknown opcode <%s>", line);
        return true;
    } while (false);

    skipSpace_(&line);

    code_log(self, "%02x ", op);

    if (code_writeRaw_(self, &op, sizeof(op))) {
        ERR("Couldn't write to file");
        return true;
    }

    if (isEOL_(line)) {
        if (curArgCnt > 0) {
            ERR("Expected an argument for op 0x%02x", op);
            return true;
        }

        code_log(self, "                           |          | \"%s\"\n", origLine);

        return false;
    }

    char argType[4] = "";

    if (readUntil_(&line, argType, ':', 4)) {
        ERR("Bad argument type");
        return true;
    }

    addrMode_t addrMode = {};

    #define ARGTYPE_CASE_(name, value)  if (strcmp(argType, name) == 0) { addrMode.type = value; } else

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

    if (!(curArgTypeMask & 1 << addrMode.type)) {
        ERR("Inappropriate argument type 0x%02x for op 0x%02x", addrMode.type, op);
        return true;
    }

    #define WRITE_ADDRMODE_() \
        if (!(curArgLocMask & (1 << addrMode.loc))) { \
            ERR("Inappropriate argument loc 0x%02x for op 0x%02x", addrMode.loc, op); \
            return true; \
        } \
        if (code_writeRaw_(self, &addrMode, sizeof(addrMode))) { \
            ERR("Couldn't write to file"); \
            return true; \
        }

    if (*line == '[') {
        //addrMode.locMem = 1;
        addrMode.loc |= ARGLOC_MEM;
        line++;
    }

    if (isEOL_(line) || isspace(*line) || (strncmp(line, "stack", 5) == 0 && (line += 5 /* Crotchy, but we kind of need this */))) {
        //addrMode.locReg = 0;
        //addrMode.locImm = 0;

        code_log(self, "%02x                         ", addrMode);

        WRITE_ADDRMODE_();

        //while (!isEOL_(line) && !isspace(*line)) ++line;
    } else if (*line == 'r') {  // TODO: labels too
        //addrMode.locReg = 1;
        //addrMode.locImm = 0;
        addrMode.loc |= ARGLOC_REG;

        line++;

        if (!('a' <= *line && *line <= 'a' + GENERAL_REG_CNT)) {
            ERR("Unknown register <r%c>", *line);
            return true;
        }

        uint8_t reg = *line - 'a';

        line++;

        skipSpace_(&line);

        if (*line == '+' || *line == '-') {  // WARNING: This is always signed
            //addrMode.locImm = 1;
            addrMode.loc |= ARGLOC_IMM;

            WRITE_ADDRMODE_();

            if (code_writeRaw_(self, &reg, sizeof(reg))) {
                ERR("Couldn't write to file");
                return true;
            }

            char immArg[sizeof(value_t)] = {};

            if (readConst_(&line, immArg, addrMode.type)) {
                ERR("Couldn't read an argument");
                return true;
            }

            /*for (size_t i = 0; i < sizeof(value_t); ++i) {
                if (i < self->size) {
                    code_log(self, "%02x ", immArg[i]);
                } else {
                    code_log(self, "   ");
                }
            }*/

            if (code_writeRaw_(self, immArg, 1 << addrMode.typeS))  {
                ERR("Couldn't write to file");
                return true;
            }

            code_log(self, "%02x %02x <>                   ", addrMode, reg);  // TODO
        } else {
            WRITE_ADDRMODE_();

            if (code_writeRaw_(self, &reg, sizeof(reg))) {
                ERR("Couldn't write to file");
                return true;
            }

            code_log(self, "%02x %02x                      ", addrMode, reg);
        }
    } else {
        //addrMode.locReg = 0;
        //addrMode.locImm = 1;
        addrMode.loc |= ARGLOC_IMM;

        code_log(self, "%02x ", addrMode);

        WRITE_ADDRMODE_();

        char immArg[sizeof(value_t)] = {};

        if (readConst_(&line, immArg, addrMode.type)) {
            ERR("Couldn't read an argument");
            return true;
        }

        for (unsigned i = 0; i < sizeof(value_t); ++i) {
            if (i < (unsigned)(1 << addrMode.typeS)) {
                code_log(self, "%02x ", (unsigned char)immArg[i]);
            } else {
                code_log(self, "   ");
            }
        }

        if (code_writeRaw_(self, immArg, 1 << addrMode.typeS))  {
            ERR("Couldn't write to file");
            return true;
        }
    }

    if (/*addrMode.locMem*/ addrMode.loc & ARGLOC_MEM) {
        if (*line != ']') {
            ERR("Missing closing square bracket");
            return true;
        }
        line++;
    }

    #undef WRITE_ADDRMODE_

    // TODO: Change
    code_log(self, "| %u%u%u%u%u%u%u%u | \"%s\"\n", addrMode.all >> 7 & 1, addrMode.all >> 6 & 1, addrMode.all >> 5 & 1, addrMode.all >> 4 & 1, addrMode.all >> 3 & 1, addrMode.all >> 2 & 1, addrMode.all >> 1 & 1, addrMode.all >> 0 & 1, origLine);

    skipSpace_(&line);

    if (!isEOL_(line)) {
        ERR("Garbage at the end of line: <%s>", line);
        return true;
    }

    return false;
}

bool code_assembleFile(code_t *self, FILE *ifile) {
    assert(self != NULL);
    assert(ifile != NULL);

    text_t itext = {};

    if (text_read(&itext, ifile)) {
        ERR("Can't read ifile text");
        return true;
    }

    code_log(self, "[ASM] +--------+\n");

    for (unsigned int i = 0; i < itext.length; ++i) {
        if (code_assembleLine(self, (const char *)itext.index[i].val)) {  // TODO: Rework assembleLine to work with line_t
            ERR("Couldn't assemble line #%u", i);  // TODO?: i + 1
            return true;
        }
    }

    code_log(self, "[ASM] +--------+\n");

    text_free(&itext);

    return false;
}

bool code_compileToFile(code_t *self, FILE *ofile) {
    assert(self != NULL);
    assert(ofile != NULL);

    aef_mmap_t mmap = {};
    aef_mmap_init(&mmap, self->size, self->buf);

    code_log(self, "[ASM] Total size: 0x%04x\n\n", self->size);  // TODO?: 0x%08x

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

static inline bool isEOL_(const char *line) {
    return *line == '\0' || *line == ';';
}

static inline void skipSpace_(const char **line) {
    while (!isEOL_(*line) && isspace(**line)) ++*line;
}

static bool readConst_(const char **line, void *valueBuf, uint8_t argType) {
    int lineDelta = 0;
    int res = 0;

    addrMode_t tmpAM = {};  // For convenience
    tmpAM.type = argType;

    #define ARGTYPE_CASE_(TYPE, FORMAT)                                 \
        assert(sizeof(TYPE) == 1 << tmpAM.typeS);                       \
        lineDelta = 0;                                                  \
        res = sscanf(*line, FORMAT "%n", (TYPE *)valueBuf, &lineDelta); \
                                                                        \
        if (res != 1) {                                                 \
            ERR("Corrupt immediate value: <%s>", *line);                \
            return true;                                                \
        }                                                               \
                                                                        \
        *line += lineDelta;                                             \
                                                                        \
        break;

    #define ARGTYPE_CASE_SIGN_(UTYPE, STYPE, FORMAT)                          \
        static_assert(sizeof(STYPE) == sizeof(UTYPE));                        \
        lineDelta = 0;                                                        \
        assert(sizeof(UTYPE) == 1 << tmpAM.typeS);                            \
        if (**line == '+' || **line == '-') {                                 \
            ++*line;                                                          \
            res = sscanf(*line, FORMAT "u%n", (UTYPE *)valueBuf, &lineDelta); \
        } else {                                                              \
            res = sscanf(*line, FORMAT "d%n", (STYPE *)valueBuf, &lineDelta); \
        }                                                                     \
                                                                              \
        if (res != 1) {                                                       \
            ERR("Corrupt immediate value: <%s>", line);                       \
            return true;                                                      \
        }                                                                     \
                                                                              \
        *line += lineDelta;                                                   \
                                                                              \
        break;

    if (isEOL_(*line)) {
        ERR("Expected an argument value");
        return true;
    }

    switch (argType) {
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

    return false;
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

