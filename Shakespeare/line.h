#ifndef SS_LINE_H_GUARD
#define SS_LINE_H_GUARD

#include "general.h"


const unsigned char MAX_LINE = 80;

/**
 * A line, possibly with metadata
 */
typedef struct line {
    letter *val;        ///< The string value
    unsigned char len;  ///< The line's length
} line_t;


/**
 * Reads a single line from a file
 * @param [in]     text    The raw text of the poem
 * @param [in,out] offset  The offset from which to start (Changes to the next line's beginning
 * @param [out]    line    The output line
 *
 * @return An error code:
 *  - `SS_OK`
 *  - `SS_TOOBIG`  The line was too long
 */
SS_ERROR readLine(letter *text, size_t *offset, line_t *line);

/**
 * A comparator function for lines that compares two lines,
 *  ignoring the non-alphabetical characters
 *
 * @param [in]  a, b  The first lines to be compared. (Actual type: `line *`)
 *
 * @return The rough equivalent of `a` - `b`, as specified in `sortLines`
 */
int cmpLines(const void *a, const void *b);  // Attention: compares two strings, not lines_t!

/**
 * The same as cmpLines, but from the back
 *
 * @param [in]  a, b  The first lines to be compared. (Actual type: `line *`)
 *
 * @return The rough equivalent of `a` - `b`, as specified in `sortLines`
 */
int cmpLinesReverse(const void *a, const void *b);

/**
 * Checks if a letter shouldn't be ignored during the comparison
 *
 * @param [in]  c  The letter to be checked
 *
 * @return Non-zero if `c` is an alphabetic character in Latin or Cyrillic or a decimal digit
 */
int isRelevant(letter c);

line_t * assignLiteralLine(line_t *line, char *value);

#endif // SS_LINE_H_GUARD
