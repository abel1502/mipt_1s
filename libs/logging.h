/**
 * A fancy logging library
 */


#ifndef LOGGINGLIB_GUARD
#define LOGGINGLIB_GUARD

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
//#include <assert.h>

#ifndef MACROFUNC  // Same as in tests, but may not be included
#define MACROFUNC(...) do {__VA_ARGS__} while (0)
#endif

#ifndef REQUIRE
#define REQUIRE(stmt)  MACROFUNC(                                                              \
    if (!(stmt)) {                                                                             \
        fprintf(stderr, "\nRequirement (%s) not met in (%s#%d)\n", #stmt, __FILE__, __LINE__); \
        abort();                                                                               \
    } )
#endif

//--------------------------------------------------------------------------------

const size_t LOGGER_MAX_BLOCKSTACK_DEPTH = 32;


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


logger_t *logger_open(logger_t *self, const char *fileName);

void logger_flush(logger_t *self);

void logger_close(logger_t *self);

void logger_blockStart(logger_t *self, block_class_t cls, ...);

void logger_blockEnd(logger_t *self);

void logger_log(logger_t *self, const char *value);

void logger_logf(logger_t *self, const char *format, ...);


static void block_stack_construct_(block_stack_t_ *self);

static void block_stack_push_(block_stack_t_ *self, block_class_t cls);

static block_class_t block_stack_pop_(block_stack_t_ *self);

static void block_stack_free_(block_stack_t_ *self);


//--------------------------------------------------------------------------------


#if 0
static size_t blockDepth = 0;


void log_tabulate() {
    for (size_t i = 0; i < blockDepth; ++i) {
        fprintf(stderr, "  ");
    }
}

void log_token(const char *token) {
    fprintf(stderr, "%s", token);
}

void log_space() {
    log_token(" ");
}

void log_newline() {
    log_token("\n");
}

void log_format(const char *format, ...) {
    va_list args;
    va_start(args, format);

    vfprintf(stderr, format, args);

    va_end(args);
}

void log_println(const char *format, ...) {
    va_list args;
    va_start(args, format);

    log_tabulate();
    vfprintf(stderr, format, args);
    log_newline();

    va_end(args);
}

void log_block_start(const char *name) {
    log_tabulate();

    if (name != NULL) {
        log_token(name);
        log_space();
    }

    log_token("{");

    log_newline();

    blockDepth++;
}

void log_block_end() {
    assert(blockDepth > 0);
    blockDepth--;

    log_token("}");

    log_newline();
}

void log_addr(const void *addr) {
    log_token("[");

    /*if (addr != NULL) {
        log_token("0x");
        log_format("%p", addr);
    } else {
        log_token("NULL");
    }*/

    log_token("0x");
    log_format("%p", addr);

    log_token("]");
}

//void log_var(const char *name, )


#define log_expr(expr, exprtype, format)                      \
    do {                                                      \
        log_println("%s = " format, #expr, (exprtype)(expr)); \
    } while (0)

#define log_expr_ptr(exprptr, exprtype, format)               \
    do {                                                      \
        exprtype *expr = (exprptr);                           \
        log_tabulate();                                       \
        log_addr(expr);                                       \
        log_space();                                          \
        if (expr == NULL) {                                   \
            log_format("%s = <BAD!>\n", #exprptr);            \
        } else {                                              \
            log_format("%s = " format "\n", #exprptr, *expr); \
        }                                                     \
    } while (0)

#define log_expr_str(exprptr)                                 \
    do {                                                      \
        char *expr = (exprptr);                               \
        log_tabulate();                                       \
        log_addr(expr);                                       \
        log_space();                                          \
        if (expr == NULL) {                                   \
            log_format("%s = <BAD!>\n", #exprptr);            \
        } else {                                              \
            log_format("%s = \"%s\"\n", #exprptr, expr);      \
        }                                                     \
    } while (0)
#endif

#endif // LOGGINGLIB_GUARD
