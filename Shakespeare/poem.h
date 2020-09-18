#ifndef SS_POEM_H_GUARD
#define SS_POEM_H_GUARD

#include "general.h"
#include "line.h"


/**
 * An array of lines of a poem
 */
typedef struct {
    int len;         ///< The number of lines
    line_t *vals;    ///< The actual lines
    size_t textLen;  ///< `text`'s length
    letter *text;    ///< Where all lines reside
} lines_t;



/**
 * Reads all lines from a file
 *
 * @param [in]  ifile     The input file
 * @param [out] lines     The destination
 *
 * @return An error code:
 *  - `SS_OK`
 *  - `SS_NOALLOC`   `initLines`
 *  - `SS_RACECOND`
 *  - `SS_TOOSMALL`  The file is (almost) empty
 *  - `SS_INNER`     `readLine`
 */
SS_ERROR readLines(FILE *ifile, lines_t *lines);

/**
 * Writes all lines to a file
 *
 * @param [in]  ofile  The output file
 * @param [in]  lines  The lines to write
 *
 * @return An error code:
 *  - `SS_OK`
 *  - `SS_INNER`  `fputs`
 */
SS_ERROR writeLines(FILE *ofile, lines_t *lines);

/**
 * Writes all lines to a file in their original order
 *
 * @param [in]  ofile  The output file
 * @param [in]  lines  The lines to write
 *
 * @return An error code:
 *  - `SS_OK`
 *  - `SS_INNER`  `fputc`
 */
SS_ERROR writeOriginalLines(FILE *ofile, lines_t *lines);

/**
 * A constructor for lines
 *
 * @param [out] lines     The lines to be initialized
 * @param [in]  maxLines  The limit to how many lines there may be
 * @param [in]  maxLen    The limit to text's length
 *
 * @return An error code:
 *  - `SS_OK`
 *  - `SS_NOALLOC`
 */
SS_ERROR initLines(lines_t *lines, int maxLines, size_t maxLen);

/**
 * Quick sorts lines based on a comparator
 *
 * @param [in,out] lines  The lines to be sorted
 * @param [in]     cmp    The comparator function.
 *   Real signature: `int cmp(const line_t * a, const line_t * b)`.
 *   Should return the equivalent of `a` - `b`
 */
void sortLines(lines_t *lines, int (*cmp)(const void *, const void *));

/**
 * Frees the memory allocated by `initLines`
 * Whatever states the lines are in, cleanup will happen
 *
 * @param [in,out]  lines  The same lines as what you passed to `initLines`
 */
void freeLines(lines_t *lines);

/**
 * Goes over the file and identifies its length and line count
 *
 * @param [in]  ifile    The input file
 * @param [out] lineCnt  Line counter
 * @param [out] length   Length counter
 */
void analyzeFile(FILE *ifile, int *lineCnt, size_t *length);

#endif // SS_POEM_H_GUARD
