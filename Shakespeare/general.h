#ifndef SS_GENERAL_H_GUARD
#define SS_GENERAL_H_GUARD

#include <assert.h>

#define ERR(msg, ...) do {fprintf(stderr, "[ERROR in %s()] " msg "\n", __func__, ##__VA_ARGS__);} while (0)


/**
 * A type for a Cyrillic-compatible letter
 */
typedef unsigned char letter;

/**
 * An error code
 */
typedef enum {
    SS_OK,        ///< Success
    SS_NOALLOC,   ///< Memory allocation failed
    SS_TOOBIG,    ///< Something overflows some limit
    SS_TOOSMALL,  ///< Something underflows some limit
    SS_RACECOND,  ///< Some sort of race condition happened
    SS_INNER      ///< Some inner function failed
} SS_ERROR;  // SS means ShakeSpeare


#endif // SS_GENERAL_H_GUARD

