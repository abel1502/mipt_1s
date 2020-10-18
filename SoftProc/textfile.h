#ifndef TEXTFILE_H_GUARD
#define TEXTFILE_H_GUARD

#include <stdio.h>

#include "general.h"


const unsigned char MAX_LINE = 255;

/**
 * A type for a Cyrillic-compatible letter
 */
typedef unsigned char letter_t;

/**
 * A line class
 */
typedef struct {
    letter_t *val;       ///< Actual string value
    unsigned char len;   ///< Line's length
} line_t;

/**
 * Memory-mapped text file class
 */
typedef struct {
    unsigned int length;  ///< Line count
    line_t *index;        ///< Array of lines
    size_t size;          ///< Text buffer size
    letter_t *buf;        ///< Text buffer
} text_t;


bool text_read(text_t *self, FILE *ifile);

bool text_write(text_t *self, FILE *ofile);

bool text_init(text_t *self, unsigned int maxLength, size_t maxSize);

void text_free(text_t *self);

bool getFSize(FILE *ifile, size_t *length);

bool line_read(line_t *self, letter_t *text, size_t *offset);


#endif // TEXTFILE_H_GUARD
