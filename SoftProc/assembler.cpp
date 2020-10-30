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
const code_size_t CODE_LABEL_CAPACITY = 0x10000;
const code_size_t CODE_LOG_BYTESPERLINE = 12;
const code_size_t CODE_DEFAULT_RAM_SIZE = 0x64000;
const char CODE_EP_NAME[] = "main";


static bool readUntil_(const char **source, char *dest, char until, size_t limit);

static inline void skipSpace_(const char **line);

static inline bool isEOL_(const char *line);

static bool code_assembleLabelLine_(code_t *self, const char **line);

static bool parseStrArgtype_(const char *argType, addrMode_t *addrMode);

static bool code_assembleOpArg_(code_t *self, addrMode_t addrMode, uint8_t argLocMask, const char **line);


code_t *code_init(code_t *self, bool doLog) {
    self->size = 0;
    self->capacity = CODE_DEFAULT_CAPACITY;

    self->buf = (char *)calloc(self->capacity, sizeof(self->buf[0]));

    if (self->buf == NULL) {
        return NULL;
    }

    self->ramSize = CODE_DEFAULT_RAM_SIZE;

    self->doLog = doLog;
    self->lineStart = 0;

    self->labelCnt = 0;
    self->labelsInited = false;

    self->labels = (label_t *)calloc(CODE_LABEL_CAPACITY, sizeof(self->labels[0]));

    if (self->labels == NULL) {
        return NULL;
    }

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

    if (self->labelsInited) {  // We may actually not write anything during label initialization.
        memcpy(self->buf + self->size, data, amount);
    }

    self->size += amount;

    return false;
}

bool code_assembleLine(code_t *self, const char *line) {
    assert(self != NULL);
    assert(self->buf != NULL);
    assert(line != NULL);

    const char *origLine = line;

    self->lineStart = self->size;

    skipSpace_(&line);

    if (isEOL_(line)) {
        return false;  // Empty lines are ok
    }

    if (*line == '$') {  // A label
        return code_assembleLabelLine_(self, &line);
    }

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

    if (code_writeRaw_(self, &op, sizeof(op))) {
        ERR("Couldn't write to file");
        return true;
    }

    if (isEOL_(line)) {
        if (curArgCnt > 0) {
            ERR("Expected an argument for op 0x%02x", op);
            return true;
        }

        if (self->labelsInited) {
            code_logLine(self, origLine);
        }

        return false;
    }

    char argType[4] = "";

    if (readUntil_(&line, argType, ':', 4)) {
        ERR("Bad argument type");
        return true;
    }

    addrMode_t addrMode = {};

    if (parseStrArgtype_(argType, &addrMode)) {
        ERR("Couldn't parse argument type");
        return true;
    }

    if (!(curArgTypeMask & 1 << addrMode.type)) {
        ERR("Inappropriate argument type 0x%02x for op 0x%02x", addrMode.type, op);
        return true;
    }

    skipSpace_(&line);

    if (code_assembleOpArg_(self, addrMode, curArgLocMask, &line)) {
        ERR("Bad argument for op 0x%02x", op);
        return true;
    }

    skipSpace_(&line);

    if (!isEOL_(line)) {
        ERR("Garbage at the end of line: <%s>", line);
        return true;
    }

    if (self->labelsInited) {
        code_logLine(self, origLine);
    }

    return false;
}

static bool code_assembleLabelLine_(code_t *self, const char **line) {
    assert(self != NULL);
    assert(line != NULL);

    assert(**line == '$');
    ++*line;

    if (code_readLabel(self, line)) {
        ERR("Failed to register label");
        return true;
    }

    skipSpace_(line);

    if (!isEOL_(*line)) {
        ERR("Garbage after label definition");
        return true;
    }

    if (!self->labelsInited) {
        label_t *curLabel = &self->labels[self->labelCnt - 1];
        char *tmpBuf = (char *)calloc(curLabel->len + 1, sizeof(char));
        assert(tmpBuf != NULL);

        memcpy(tmpBuf, curLabel->name, curLabel->len);

        code_log(self, "[ASM] $%s = 0x%08x\n", tmpBuf, curLabel->value);
        free(tmpBuf);
    }

    /*if (self->labelsInited) {
        code_log(self, "[ASM] |   ----->   |\n");
    }*/

    return false;
}

static bool code_assembleOpArg_(code_t *self, addrMode_t addrMode, uint8_t argLocMask, const char **line) {
    #define WRITE_ADDRMODE_() \
        addrMode.type = backupArgType; \
        if (!(argLocMask & (1 << addrMode.loc))) { \
            ERR("Inappropriate argument loc 0x%02x", addrMode.loc); \
            return true; \
        } \
        if (code_writeRaw_(self, &addrMode, sizeof(addrMode))) { \
            ERR("Couldn't write to file"); \
            return true; \
        } \
        if (addrMode.locMem) { \
            addrMode.type = ARGTYPE_DWL; \
        }

    uint8_t backupArgType = addrMode.type;  // Because memory addressing forces arg type to be dwl, and the actual type may be valuable later

    if (**line == '[') {
        addrMode.locMem = 1;

        addrMode.type = ARGTYPE_DWL;
        ++*line;
    }

    skipSpace_(line);

    if (isEOL_(*line) || isspace(**line) || (strncmp(*line, "stack", 5) == 0 && (*line += 5 /* Crotchy, but we kind of need this */))) {
        addrMode.locReg = 0;
        addrMode.locImm = 0;

        WRITE_ADDRMODE_();
    } else {
        char immArg[sizeof(value_t)] = {};
        uint8_t reg = 0xff;

        if (**line == 'r') {
            addrMode.locReg = 1;

            if (code_readArgReg_(self, line, &reg)) {
                ERR("Couldn't read argument");
                return true;
            }

            skipSpace_(line);
        }

        if (!addrMode.locReg || **line == '+' || **line == '-') {
            addrMode.locImm = 1;

            if (code_readArgConst_(self, line, immArg, addrMode.type)) {
                ERR("Couldn't read argument");
                return true;
            }
        }

        WRITE_ADDRMODE_();

        if (addrMode.locReg && code_writeRaw_(self, &reg, sizeof(reg))) {
            ERR("Couldn't write to file");
            return true;
        }

        if (addrMode.locImm && code_writeRaw_(self, immArg, 1 << addrMode.typeS))  {
            ERR("Couldn't write to file");
            return true;
        }
    }

    skipSpace_(line);

    if (addrMode.locMem) {
        if (**line != ']') {
            ERR("Missing closing square bracket");
            return true;
        }
        ++*line;
    }

    #undef WRITE_ADDRMODE_

    return false;
}

static bool parseStrArgtype_(const char *argType, addrMode_t *addrMode) {
    #define ARGTYPE_CASE_(NAME, VALUE)  if (strcmp(argType, NAME) == 0) { addrMode->type = VALUE; } else

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

    self->labelsInited = false;

    for (unsigned int i = 0; i < itext.length; ++i) {
        if (code_assembleLine(self, (const char *)itext.index[i].val)) {
            ERR("Couldn't assemble line #%u", i + 1);
            return true;
        }
    }

    self->labelsInited = true;
    self->size = 0;

    code_log(self, "[ASM] +------------+\n");

    for (unsigned int i = 0; i < itext.length; ++i) {
        if (code_assembleLine(self, (const char *)itext.index[i].val)) {  // TODO: Rework assembleLine to work with line_t
            ERR("Couldn't assemble line #%u", i + 1);
            return true;
        }
    }

    code_log(self, "[ASM] +------------+\n");

    text_free(&itext);

    return false;
}

bool code_compileToFile(code_t *self, FILE *ofile) {
    assert(self != NULL);
    assert(ofile != NULL);

    assert(self->labelsInited);

    code_size_t entrypoint = 0;
    const char *epname = CODE_EP_NAME;
    if (code_lookupLabel(self, &epname, &entrypoint)) {
        ERR("Warning: implicit entrypoint");
    }

    aef_mmap_t mmap = {};
    aef_mmap_init(&mmap, self->size, self->buf, entrypoint, self->ramSize);

    code_log(self, "[ASM] Total size: 0x%08x\n\n", self->size);

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

    free(self->labels);

    self->capacity = 0;
    self->size = 0;

    self->labelCnt = 0;
    self->labelsInited = false;
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

bool code_readArgConst_(code_t *self, const char **line, void *valueBuf, uint8_t argType) {
    assert(self != NULL);
    assert(line != NULL);
    assert(valueBuf != NULL);

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
            res = sscanf(*line, FORMAT "d%n", (STYPE *)valueBuf, &lineDelta); \
        } else {                                                              \
            res = sscanf(*line, FORMAT "u%n", (UTYPE *)valueBuf, &lineDelta); \
        }                                                                     \
                                                                              \
        if (res != 1) {                                                       \
            ERR("Corrupt immediate value: <%s>", *line);                      \
            return true;                                                      \
        }                                                                     \
                                                                              \
        *line += lineDelta;                                                   \
                                                                              \
        break;

    #define ALLOW_LABEL_(TYPE)                              \
        if (**line == '+' && *(*line + 1) == '$') ++*line;  \
        if (**line == '$') {                                \
            ++*line;                                        \
            code_size_t buf = 0;                            \
                                                            \
            if (code_lookupLabel(self, line, &buf)) {       \
                ERR("Couldn't look up label $%s", *line);   \
                return true;                                \
            }                                               \
                                                            \
            *(TYPE *)valueBuf = (TYPE)buf;                  \
                                                            \
            break;                                          \
        }

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
        ALLOW_LABEL_(uint32_t);
        ARGTYPE_CASE_SIGN_(uint32_t, int32_t, "%")
    case ARGTYPE_WL:
    case ARGTYPE_WH:
        ALLOW_LABEL_(uint16_t);
        ARGTYPE_CASE_SIGN_(uint16_t, int16_t, "%h")
    case ARGTYPE_BL:
    case ARGTYPE_BH:
        ALLOW_LABEL_(uint8_t);
        ARGTYPE_CASE_SIGN_(uint8_t, int8_t, "%hh")
    default:
        ERR("Cannot handle constants of <%s> type", argType);
        return true;
    }

    #undef ALLOW_LABEL_
    #undef ARGTYPE_CASE_SIGN_
    #undef ARGTYPE_CASE_

    return false;
}

bool code_readArgReg_(code_t *self, const char **line, uint8_t *reg) {
    assert(self != NULL);
    assert(line != NULL);
    assert(reg != NULL);

    assert(**line == 'r');

    ++*line;

    if (!('a' <= **line && **line <= 'a' + GENERAL_REG_CNT)) {
        ERR("Unknown register <r%c>", **line);
        return true;
    }

    *reg = **line - 'a';

    ++*line;

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

void code_logLine(code_t *self, const char *line) {
    assert(self != NULL);
    assert(line != NULL);

    code_log(self, "[ASM] | 0x%08x | ", self->lineStart);

    code_size_t ind = 0;

    while (self->lineStart + ind < self->size) {
        code_log(self, "%02x ", (unsigned char)self->buf[self->lineStart + ind++]);
    }

    while (ind < CODE_LOG_BYTESPERLINE) {
        code_log(self, "   ");
        ind++;
    }

    char buf[MAX_LINE + 1] = "";

    skipSpace_(&line);

    readUntil_(&line, buf, ';', MAX_LINE);

    code_log(self, "| %s\n", buf);
}

bool code_readLabel(code_t *self, const char **line) {
    assert(self != NULL);

    label_t label = {};
    label.name = *line;

    while (!isEOL_(*line) && **line != ':' && **line != ']' && !isspace(**line)) {
        ++*line;
        label.len++;
    }

    skipSpace_(line);

    switch (*((*line)++)) {
    case ':':
        label.value = self->size;
        break;
    case '=':
        skipSpace_(line);

        if (code_readArgConst_(self, line, &label.value, ARGTYPE_DWL)) {
            ERR("Label assignment must be followed by a valid dwl constant");
            return true;
        }

        break;
    default:
        ERR("Expected a semicolon or an assignment after label definition");
        return true;
    }

    if (self->labelsInited) {
        return false;
    }

    if (self->labelCnt >= CODE_LABEL_CAPACITY) {
        ERR("Too many labels");
        return true;
    }

    self->labels[self->labelCnt++] = label;

    return false;
}

bool code_lookupLabel(code_t *self, const char **line, code_size_t *value) {
    assert(self != NULL);
    assert(line != NULL);
    assert(*line != NULL);
    assert(value != NULL);

    if (!self->labelsInited) {
        *value = 0;
        while (!isEOL_(*line) && !isspace(**line) && **line != ']') {
            ++*line;
        }
        return false;
    }

    for (code_size_t i = 0; i < self->labelCnt; ++i) {
        if (strncmp(self->labels[i].name, *line, self->labels[i].len) == 0) {
            const char nextChar = (*line)[self->labels[i].len];
            if (!isEOL_(&nextChar) && !isspace(nextChar) && nextChar != ']') {
                continue;
            }

            *value = self->labels[i].value;
            *line += self->labels[i].len;
            return false;
        }
    }

    return true;
}

