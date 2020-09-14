/*
 Global TODO:

 # Add reverse sort and restoration
 - Wrap all error codes in enums
 - Move tests into files
 # Open files in binary mode
 # Add digits to whitelist in the comparator
 # Store line's length in line_t
 / Remove struct names (and possibly remove _t from typedef ones)
 # Add ё and Ё
 # Add error handling to readLine
 ? Maybe exclude ёЁ from the whitelist, since they're not in alphabetical order inside the encoding
*/

//#define TEST
#include "../libs/test.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <locale.h>
#include <ctype.h>
#include <stdbool.h>


#define ERR(msg, ...) do {fprintf(stderr, "[ERROR in %s()] " msg "\n", __func__, ##__VA_ARGS__);} while (0)

//--------------------------------------------------------------------------------

const unsigned char MAX_LINE = 80;
// It's a letter[] constant, but even when I moved the typedef to the top, it still didn't work
// (error: structured binding declaration cannot have type 'const letter' {aka 'const unsigned char'}|)
#define SEP "\n--------------------------------------------------------------------------------\n\n"

//--------------------------------------------------------------------------------

/**
 * A type for a Cyrillic-compatible letter
 */
typedef unsigned char letter;

/**
 * A line, possibly with metadata
 */
typedef struct line {
    letter *val;  /**< The string value */
    unsigned char len;  /**< The line's length */
} line_t;

/**
 * An array of lines of a poem
 */
typedef struct lines {
    int len;  /**< The number of lines */
    line_t *vals;  /**< The actual lines */
    size_t textLen;  /**< `text`'s length */
    letter *text;  /**< Where all lines reside */
} lines_t;

/**
 * Shows a constant banner at the beginning of execution
 */
void showBanner(void);

/**
 * Shows help on how to use the program
 */
void showUsage(const char *binname);

/**
 * Reads a single line from a file
 * @param [in]     text    The raw text of the poem
 * @param [in,out] offset  The offset from which to start (Changes to the next line's beginning
 * @param [out]    line    The output line
 *
 * @return An error code:
 *  - 0
 *    Success
 *  - 1
 *    Error, line too long
 */
int readLine(letter *text, size_t *offset, line_t *line);

/**
 * Reads all lines from a file
 *
 * @param [in]  ifile     The input file
 * @param [out] lines     The destination
 *
 * @return An error code:
 *  - 0
 *    Success
 *  - 1
 *    Error, space couldn't have been allocated for the lines
 *  - 2
 *    Error, couldn't read the entire file
 *  - 3
 *    Error, number of lines changed (somehow)
 *  - 4
 *    Error, the file is empty
 *  - 5
 *    Error, `readLine` failed
 */
int readLines(FILE *ifile, lines_t *lines);

/**
 * Writes all lines to a file
 *
 * @param [in]  ofile  The output file
 * @param [in]  lines  The lines to write
 *
 * @return An error code:
 *  - 0
 *    Success
 *  - 1
 *    Error, `fputs` failed
 */
int writeLines(FILE *ofile, lines_t *lines);

/**
 * Writes all lines to a file in their original order
 *
 * @param [in]  ofile  The output file
 * @param [in]  lines  The lines to write
 *
 * @return An error code:
 *  - 0
 *    Success
 *  - 1
 *    Error, `fputc` failed
 */
int writeOriginalLines(FILE *ofile, lines_t *lines);

/**
 * A constructor for lines
 *
 * @param [out] lines     The lines to be initialized
 * @param [in]  maxLines  The limit to how many lines there may be
 * @param [in]  maxLen    The limit to text's length
 *
 * @return An error code:
 *  - 0
 *    Success
 *  - 1
 *    Error, space couldn't have been allocated for the lines
 *  - 2
 *    Error, space couldn't have been allocated for the text
 */
int initLines(lines_t *lines, int maxLines, size_t maxLen);

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
 * Checks if a letter shouldn't be ignored during the sorting
 *
 * @param [in]  c  The letter to be checked
 *
 * @return True if `c` is an alphabetic character in Latin or Cyrillic or a decimal digit
 */
bool isRelevant(letter c);

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
 * Frees the memory allocated by `initLines`
 *
 * @param [in,out]  lines  The same lines as what you passed to `initLines`
 */
void freeLines(lines_t * lines);

/**
 * Goes over the file and identifies its length and line count
 *
 * @param [in]  ifile    The input file
 * @param [out] lineCnt  Line counter
 * @param [out] length   Length counter
 */
void analyzeFile(FILE * ifile, int *lineCnt, size_t *length);

#ifdef TEST
void test_cmpLines(lines_t * lines);
#endif // TEST

//================================================================================

int main(const int argc, const char **argv) {
    setlocale(LC_CTYPE, "Russian");

    TEST_MAIN(
        lines_t test_lines;
        initLines(&test_lines, 40, 1000);
        ,
        test_cmpLines(&test_lines);
        $g; TEST_MSG("Passed All."); $d;
        ,
        freeLines(&test_lines);
    )

    showBanner();

    if (argc != 2) {
        showUsage(argv[0]);
        ERR("Bad arg count");
        return EXIT_FAILURE;
    }

    FILE * ifile = fopen(argv[1], "rb");
    if (ifile == NULL) {
        ERR("Couldn't open \'%s\' to read", argv[1]);
        return EXIT_FAILURE;
    }
    lines_t lines;  // Lines intentionally not init-ed, because readLines does it itself
    int result = readLines(ifile, &lines);
    fclose(ifile);
    if (result) {
        ERR("Error while reading lines");
        freeLines(&lines);
        return EXIT_FAILURE;
    }

    char newName[32] = "sorted_";
    if (strlen(newName) + strlen(argv[1]) > sizeof(newName)) {
        ERR("Binary\'s name is too long");
        freeLines(&lines);
        return EXIT_FAILURE;
    }
    strcat(newName, argv[1]);


    FILE * ofile = fopen(newName, "wb");
    if (ofile == NULL) {
        ERR("Couldn't open \'%s\' to write", newName);
        freeLines(&lines);
        return EXIT_FAILURE;
    }

    printf("Read %d lines, sorting...\n", lines.len);
    sortLines(&lines, cmpLines);

    printf("Done sorting, writing to %s\n", newName);
    if (writeLines(ofile, &lines)) {
        ERR("Error while writing lines");
        fclose(ofile);
        freeLines(&lines);
        return EXIT_FAILURE;
    }

    printf("Sorting again, this time in reverse...\n");
    sortLines(&lines, cmpLinesReverse);

    printf("Done sorting for the second time, writing to the same file\n");
    if (fprintf(ofile, SEP) < 0 || writeLines(ofile, &lines)) {
        ERR("Error while writing lines");
        fclose(ofile);
        freeLines(&lines);
        return EXIT_FAILURE;
    }

    printf("Okay, returning the original text now...\n");
    if (fprintf(ofile, SEP) < 0 || writeOriginalLines(ofile, &lines)) {
        ERR("Error while writing lines");
        fclose(ofile);
        freeLines(&lines);
        return EXIT_FAILURE;
    }

    printf("Done.\n");
    fclose(ofile);
    freeLines(&lines);
    return EXIT_SUCCESS;
}

//================================================================================

void showBanner(void) {
    printf("################################\n"
           "#                              #\n"
           "#         Shakespeare          #\n"
           "#        (c) Abel, 2020        #\n"
           "#                              #\n"
           "################################\n");
    printf("This program sorts a Shakespearean poem in several funky ways.\n"
           "I\'m too lazy to explain. Just give it a try).\n"
           "\nEDIT: Okay, apparently Hamlet isn\'t a real poem, so I hereby\n"
           "proclaim that this also works on Eugene Onegin\n\n");
}

void showUsage(const char *binname) {
    printf("Usage: %s ifile\n\n"
           "ifile - The file containing the source poem\n"
           "(The results are placed in files with the same names with extra prefixes)\n\n", binname);
}

int readLine(letter *text, size_t *offset, line_t *line) {
    line->val = text + (*offset);
    for (line->len = 0; text[*offset] != '\n' && line->len < MAX_LINE; (*offset)++, line->len++) {}
    if (text[*offset] != '\n') {
        ERR("Line too long");
        return 1;
    }
    text[*offset] = '\0';
    line->len++;
    (*offset)++;
    return 0;
}

int readLines(FILE *ifile, lines_t *lines) {
    assert(ifile != NULL);
    assert(lines != NULL);
    size_t textLen = 0;
    int lineCnt = 0;
    analyzeFile(ifile, &lineCnt, &textLen);
    if (textLen <= 2) {
        ERR("Empty file");
        return 4;
    }

    if (initLines(lines, lineCnt, textLen) != 0) {
        ERR("Trouble initializing lines");
        return 1;
    }

    if (fread(lines->text, sizeof(letter), textLen - 1, ifile) != textLen - 1) {
        ERR("Insufficient data read from file. Race condition, huh?");
        return 2;
    }
    lines->textLen = textLen;
    lines->text[textLen - 1] = '\0';
    if (lines->text[textLen - 2] != '\n') {
        lines->text[textLen - 1] = '\n';
        lineCnt++;
    } else {
        lines->textLen--;
        textLen--;
    }

    size_t offset = 0;
    for (lines->len = 0; lines->len < lineCnt; lines->len++) {
        if (readLine(lines->text, &offset, &lines->vals[lines->len])) {
            ERR("Can't parse line #%d", lines->len);
            return 5;
        }
    }
    //printf("[DBG] %llu out of %llu\n", offset, textLen);
    //assert(offset == textLen);
    /*

    lines->vals[0].val = lines->text;
    lines->len++;
    size_t pos = 0;
    while (pos < textLen) {
        if (lines->text[pos] == '\n') {
            lines->text[pos] = '\0';
            lines->vals[lines->len].val = lines->text + pos + 1;
            lines->vals[lines->len - 1].len = lines->vals[lines->len].val - lines->vals[lines->len - 1].val;
            lines->len++;
        }
        pos++;
    }
    if (lines->text[textLen - 2] != '\n') {
        //lines->len--;
        lines->vals[lines->len - 1].len = lines->vals[lines->len - 1].val - lines->vals[lines->len - 1].val;
    }*/

    if (lines->len != lineCnt) {
        ERR("Wrong number of lines (%d instead of %d)", lines->len, lineCnt);
        return 3;
    }
    return 0;
}

int writeLines(FILE *ofile, lines_t *lines) {
    assert(ofile != NULL);
    assert(lines != NULL);
    for (int i = 0; i < lines->len; ++i) {
        if (fputs((const char *)lines->vals[i].val, ofile) == EOF) {
            ERR("Can\'t write line #%d", i);
            return 1;
        }
        //fprintf(ofile, " (%d)", lines->vals[i].len);
        fputc('\n', ofile);
    }
    return 0;
}

int writeOriginalLines(FILE *ofile, lines_t *lines) {
    for (size_t i = 0; i < lines->textLen; ++i) {
        letter cur = lines->text[i];
        if (cur == '\0') cur = '\n';
        if (fputc(cur, ofile) == EOF) {
            ERR("Can\'t write letter #%zu", i);
            return 1;
        }
    }
    return 0;
}

int initLines(lines_t *lines, int maxLines, size_t maxLen) {
    assert(lines != NULL);
    lines->vals = (line *) calloc(maxLines, sizeof(line));
    if (lines->vals == NULL) {
        ERR("Can't allocate space for lines");
        return 1;
    }
    lines->text = (letter *)malloc(maxLen * sizeof(letter));
    if (lines->text == NULL) {
        ERR("Can't allocate space for text");
        return 2;
    }
    lines->len = 0;
    lines->textLen = 0;
    return 0;
}

void sortLines(lines_t *lines, int (*cmp)(const void *, const void *)) {
    assert(lines != NULL);
    assert(cmp != NULL);
    qsort(lines->vals, lines->len, sizeof(lines->vals[0]), cmp);
}

bool isRelevant(letter c) {
    return isalnum(c);
}

int cmpLines(const void *a, const void *b) {
    assert(a != NULL);
    assert(b != NULL);
    const letter * a_str = (*(const line *)a).val;
    const letter * b_str = (*(const line *)b).val;
    const unsigned char a_len = (*(const line *)a).len;
    const unsigned char b_len = (*(const line *)b).len;

    int i = 0, j = 0;
    while (i < a_len && j < b_len) {
        if (isRelevant(a_str[i]) && isRelevant(b_str[j])) {
            int res = (int)a_str[i] - (int)b_str[j];
            if (res) return res;
            i++; j++;
            continue;
        }
        if (!isRelevant(a_str[i])) i++;
        if (!isRelevant(b_str[j])) j++;
    }
    return 0;
}

int cmpLinesReverse(const void *a, const void *b) {
    assert(a != NULL);
    assert(b != NULL);
    const letter * a_str = (*(const line *)a).val;
    const letter * b_str = (*(const line *)b).val;
    const unsigned char a_len = (*(const line *)a).len;
    const unsigned char b_len = (*(const line *)b).len;

    int i = a_len - 1, j = b_len - 1;
    while (i >= 0 && j >= 0) {
        if (isRelevant(a_str[i]) && isRelevant(b_str[j])) {
            int res = (int)a_str[i] - (int)b_str[j];
            if (res) return res;
            i--; j--;
            continue;
        }
        if (!isRelevant(a_str[i])) i--;
        if (!isRelevant(b_str[j])) j--;
    }
    if (i < 0 && j >= 0) return -1;
    if (i >= 0 && j < 0) return 1;
    return 0;
}

void freeLines(lines_t *lines) {
    if (lines == NULL) return;
    if (lines->vals != NULL) free(lines->vals);
    if (lines->text != NULL) free(lines->text);
    lines->vals = NULL;
    lines->text = NULL;
    lines->len = 0;
}

void analyzeFile(FILE * ifile, int *lineCnt, size_t *length) {
    assert(ifile != NULL);
    assert(lineCnt != NULL);
    assert(length != NULL);
    fseek(ifile, 0L, SEEK_SET);
    (*lineCnt) = 0;
    (*length) = 0;
    int cur = 0;
    do {
        cur = fgetc(ifile);
        (*length)++;
        if (cur == '\n') (*lineCnt)++;
    } while (cur != EOF);
    fseek(ifile, 0L, SEEK_SET);
}

//================================================================================

#ifdef TEST
void test_cmpLines(lines_t * lines) {
    const int cnt = 5;
    const letter values[cnt][20] = {"Привет", "При, вот", "!При, ват", "Bonjour", "[123]"};
    const letter result[cnt][20] = {"[123]", "Bonjour", "!При, ват", "Привет", "При, вот"};

    for (int i = 0; i < cnt; ++i) {
        lines->vals[i].val = (letter *)calloc(20, sizeof(letter));
        TEST_ASSERT_M(lines->vals[i].val != NULL, "Not enough RAM for the test");
        strcpy((char *)lines->vals[lines->len++].val, (const char *)values[i]);
    }

    sortLines(lines, cmpLines);

    TEST_ASSERT(lines->len == cnt);
    for (int i = 0; i < cnt; ++i) {
        TEST_ASSERT_M(strcmp((const char *)lines->vals[i].val, (const char *)result[i]) == 0, "Expected %s on %ith place, got %s instead", result[i], i, lines->vals[i].val);
    }
}
#endif // TEST
