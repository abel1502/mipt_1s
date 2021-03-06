/**
 * A fancy logging library
 */


#ifndef LOGGINGLIB_GUARD
#define LOGGINGLIB_GUARD

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
//#include <assert.h>

//================================================================================

#ifndef MACROFUNC  // Same as in tests, but may not be included
#define MACROFUNC(...) do {__VA_ARGS__} while (0)
#endif

#ifndef REQUIRE
// Logger assertions mustn't be disableable, since an error in logging is possibly the most critical error possible
#define REQUIRE(stmt)  MACROFUNC(                                                              \
    if (!(stmt)) {                                                                             \
        fprintf(stderr, "\nRequirement (%s) not met in (%s#%d)\n", #stmt, __FILE__, __LINE__); \
        abort();                                                                               \
    } )
#endif

//================================================================================

typedef struct logger_s logger_t;

typedef struct block_stack_s_ block_stack_t_;

typedef const char *block_class_t;


struct block_stack_s_ {
    size_t size;
    block_class_t *buf;
};

struct logger_s {
    FILE *file;
    block_stack_t_ blockStack;
};

//--------------------------------------------------------------------------------

const size_t LOGGER_MAX_BLOCKSTACK_DEPTH = 64;


const block_class_t LBC_ROOT = "log";
const block_class_t LBC_VARDUMP = "var";
const block_class_t LBC_PTRDUMP = "ptr";
const block_class_t LBC_ARRDUMP = "arr";
const block_class_t LBC_STRUCTDUMP = "struct";
const block_class_t LBC_FIELDDUMP = "fld";
const block_class_t LBC_COMMENT = "cmnt";
const block_class_t LBC_ERROR = "err";
const block_class_t LBC_WARNING = "warn";

//--------------------------------------------------------------------------------

logger_t *logger_open(logger_t *self, const char *fileName);

void logger_flush(logger_t *self);

void logger_close(logger_t *self);

void logger_blockStart(logger_t *self, block_class_t cls, ...);

void logger_blockEnd(logger_t *self);

void logger_write_(logger_t *self, const char *value);

void logger_writef_(logger_t *self, const char *format, ...);

void logger_log(logger_t *self, const char *value);

void logger_logf(logger_t *self, const char *format, ...);

//--------------------------------------------------------------------------------

block_stack_t_ *block_stack_construct_(block_stack_t_ *self);

static void block_stack_push_(block_stack_t_ *self, block_class_t cls);

static block_class_t block_stack_pop_(block_stack_t_ *self);

static void block_stack_free_(block_stack_t_ *self);

//================================================================================

#ifndef NO_LOG

block_stack_t_ *block_stack_construct_(block_stack_t_ *self) {
    REQUIRE(self != NULL);

    self->size = 0;
    self->buf = (block_class_t *)calloc(LOGGER_MAX_BLOCKSTACK_DEPTH, sizeof(block_class_t));

    REQUIRE(self->buf != NULL);

    return self;
}

static void block_stack_push_(block_stack_t_ *self, block_class_t cls) {
    REQUIRE(self != NULL);
    REQUIRE(self->buf != NULL);
    REQUIRE(cls != NULL);

    REQUIRE(self->size < LOGGER_MAX_BLOCKSTACK_DEPTH /* Sorry, LBS depth limit reached :( */ );

    self->buf[self->size++] = cls;
}

static block_class_t block_stack_pop_(block_stack_t_ *self) {
    REQUIRE(self != NULL);
    REQUIRE(self->buf != NULL);

    REQUIRE(self->size > 0 /* Sorry, LBS is empty :( */ );

    return self->buf[--(self->size)];
}

static void block_stack_free_(block_stack_t_ *self) {
    REQUIRE(self != NULL);
    REQUIRE(self->buf != NULL);

    REQUIRE(self->size == 0 /* Sorry, LBS is not empty :( */ );

    free(self->buf);
}

//--------------------------------------------------------------------------------

logger_t *logger_open(logger_t *self, const char *fileName) {
    REQUIRE(self != NULL);

    if (fileName == NULL) {
        self->file = stderr;
    } else {
        self->file = fopen(fileName, "a");
    }

    REQUIRE(self->file != NULL);

    block_stack_construct_(&self->blockStack);

    logger_blockStart(self, LBC_ROOT);

    return self;
}

void logger_flush(logger_t *self) {
    REQUIRE(self != NULL);
    REQUIRE(self->file != NULL);

    fflush(self->file);
}

void logger_close(logger_t *self) {
    REQUIRE(self != NULL);
    REQUIRE(self->file != NULL);

    logger_blockEnd(self);
    logger_write_(self, "\n");

    if (self->file != stdout && self->file != stderr)
        fclose(self->file);

    self->file = NULL;

    block_stack_free_(&self->blockStack);
}

void logger_blockStart(logger_t *self, block_class_t cls, ...) {  // TODO: attributes through va_args
                                                                  // Actually, gotta include their amount as well...
                                                                  // Maybe I'll stick to passing a struct
    REQUIRE(self != NULL);
    REQUIRE(self->file != NULL);

    block_stack_push_(&self->blockStack, cls);

    logger_writef_(self, "<%s", cls);

    logger_write_(self, ">");
}

void logger_blockEnd(logger_t *self) {
    REQUIRE(self != NULL);
    REQUIRE(self->file != NULL);
    REQUIRE(self->blockStack.size > 0);

    logger_writef_(self, "</%s>", block_stack_pop_(&self->blockStack));
}

void logger_write_(logger_t *self, const char *value) {
    REQUIRE(self != NULL);
    REQUIRE(self->file != NULL);
    REQUIRE(value != NULL);

    REQUIRE(fputs(value, self->file) != EOF);

    logger_flush(self);
}

void logger_writef_(logger_t *self, const char *format, ...) {
    REQUIRE(self != NULL);
    REQUIRE(self->file != NULL);
    REQUIRE(format != NULL);

    va_list args = {};
    va_start(args, format);

    int requiredSize = vsnprintf(NULL, 0, format, args);

    va_end(args);

    char *buf = (char *)calloc(requiredSize + 1, sizeof(char));

    va_start(args, format);

    vsprintf(buf, format, args);

    va_end(args);

    logger_write_(self, buf);

    free(buf);
}

void logger_log(logger_t *self, const char *value) {
    REQUIRE(self != NULL);
    REQUIRE(self->file != NULL);
    REQUIRE(value != NULL);

    int res = 0;

    #define ESCAPE_CHAR_(symbol, name) case symbol: res |= fputs("&" #name ";", self->file); break;

    while (*value != '\0') {
        switch (*value) {
            ESCAPE_CHAR_('&',  amp)
            ESCAPE_CHAR_('<',  lt)
            ESCAPE_CHAR_('>',  gt)
            ESCAPE_CHAR_('\'', apos)
            ESCAPE_CHAR_('\"', quot)
        case '\n':
            res |= fputs("<br>", self->file);
            break;
        default:
            res |= fputc(*value, self->file);
            break;
        }

        ++value;
    }

    #undef ESCAPE_CHAR_

    REQUIRE(res >= 0 /* File output failed */ );

    logger_flush(self);
}

void logger_logf(logger_t *self, const char *format, ...) {
    REQUIRE(self != NULL);
    REQUIRE(self->file != NULL);
    REQUIRE(format != NULL);

    va_list args = {};
    va_start(args, format);

    int requiredSize = vsnprintf(NULL, 0, format, args);

    va_end(args);

    char *buf = (char *)calloc(requiredSize + 1, sizeof(char));

    va_start(args, format);

    vsprintf(buf, format, args);

    va_end(args);

    logger_log(self, buf);

    free(buf);
}

#else // NO_LOG

// TODO

#endif // NO_LOG

#endif // LOGGINGLIB_GUARD
