//#define TEST
#include "../libs/test.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <locale.h>
#include <ctype.h>


#if !NDEBUG  // Okay, not gonna bother this time
#define ERR(msg, ...) do {fprintf(stderr, "[ERROR in %s()] " msg "\n", __func__, ##__VA_ARGS__);} while (0)
#else
#definne ERR(msg, ...) do {} while (0)
#endif

//--------------------------------------------------------------------------------

const int MAX_LINE = 80;
const int MAX_LINES = 0x4000;

//--------------------------------------------------------------------------------

// TODO: Document exceptional return values

/**
 * A type for a Cyrillic-compatible letter
 */
typedef unsigned char letter;

/**
 * An array of lines of a poem
 */
typedef struct lines {
    int len;  /**< The number of lines */
    letter **vals;  /**< The actual lines */
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
 * Reads a line from a file
 *
 * @param [in]  ifile   The input file
 * @param [out] line    The destination
 * @param [in]  maxLen  A limit to how long the line may be
 *
 * @return An error code:
 *  - 0
 *    Success
 *  - 1
 *    Success (?), but EOF was reached instead of '\\n'
 *  - 2
 *    Error, line's length exceeds `maxLen`
 */
int readLine(FILE *ifile, letter *line, int maxLen);

/**
 * Reads all lines from a file
 *
 * @param [in]  ifile     The input file
 * @param [out] lines     The destination
 * @param [in]  maxLines  A limit to how many lines there may be
 *
 * @return An error code:
 *  - 0
 *    Success
 *  - 1
 *    Error, space couldn't have been allocated for one of the lines
 *  - 2
 *    Error, one of the lines was too long
 *
 *  Attention: if `maxLines` lines were read, 0 is returned,
 *  but EOF may not have been reached yet
 *
 */
int readLines(FILE *ifile, lines_t *lines, int maxLines);

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
 *    Error, fputs failed
 */
int writeLines(FILE *ofile, lines_t *lines);

/**
 * A constructor for lines
 *
 * @param [out] lines  The lines to be initialized
 *
 * @return An error code:
 *  - 0
 *    Success
 *  - 1
 *    Error, space couldn't have been allocated for the lines array
 */
int initLines(lines_t *lines);

/**
 * Quick sorts lines based on a comparator
 *
 * @param [in,out] lines  The lines to be sorted
 * @param [in]     cmp    The comparator function.
 *   Real signature: `int cmp(const letter ** a, const letter ** b)`.
 *   Should return the equivalent of `a` - `b`
 */
void sortLines(lines_t *lines, int (*cmp)(const void *, const void *));

/**
 * Checks if a letter is an alphabetic character in Latin or Cyrillic
 *
 * @param [in]  c  The letter to be checked
 *
 * @return True if `c` is an alphabetic character
 */
bool isLetter(letter c);

/**
 * A comparator function for lines that compares two lines,
 *  ignoring the non-alphabetical characters
 *
 * @param [in]  a, b  The first lines to be compared. (Actual type: `letter**`)
 *
 * @return The rugh equivalent of `a` - `b`, as specified in `sortLines`
 */
int cmpLines(const void *a, const void *b);  // Attention: compares two strings, not lines_t!

/**
 * Frees the memory allocated by `initLines`
 *
 * @param [in,out]  lines  The same lines as what you passed to `initLines`
 */
void freeLines(lines_t * lines);

#ifdef TEST
void test_cmpLines(lines_t * lines);
#endif // TEST

//================================================================================

int main(const int argc, const char **argv) {
    setlocale(LC_ALL, "Russian");

    TEST_MAIN(
        lines_t test_lines;
        initLines(&test_lines);
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

    FILE * ifile = fopen(argv[1], "r");
    lines_t lines;
    initLines(&lines);
    int result = readLines(ifile, &lines, MAX_LINES);
    fclose(ifile);
    if (result) {
        ERR("Error while reading lines");
        return EXIT_FAILURE;
    }

    printf("Read %d lines, sorting...\n", lines.len);
    sortLines(&lines, cmpLines);

    char newName[32] = "sorted_";
    if (strlen(newName) + strlen(argv[1]) > sizeof(newName)) {
        ERR("Binary\'s name is too long");
        return EXIT_FAILURE;
    }
    strcat(newName, argv[1]);

    printf("Done sorting, writing to %s\n", newName);
    FILE * ofile = fopen(newName, "w");
    result = writeLines(ofile, &lines);
    fclose(ofile);
    if (result) {
        ERR("Error while writing lines");
        return EXIT_FAILURE;
    }

    printf("Done.\n");
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

int readLine(FILE *ifile, letter *line, int maxLen) {
    int cur = fgetc(ifile);
    int pos = 0;
    for (; cur != EOF && cur != '\n' && pos < maxLen; ++pos) {
        line[pos] = cur;
        cur = fgetc(ifile);
    }
    line[pos] = '\0';
    if (cur == '\n') {
        return 0;
    } else if (cur == EOF) {
        return 1;
    } else {
        ERR("Line too long");
        return 2;
    }
}

int readLines(FILE *ifile, lines_t *lines, int maxLines) {
    int state = 0;
    int i = 0;
    for (; state == 0 && i < maxLines; ++i) {
        lines->vals[i] = (letter *) malloc(sizeof(letter) * MAX_LINE);
        if (lines->vals[i] == NULL) {
            ERR("Can\'t allocate space for a line");
            return 1;
        }
        state = readLine(ifile, lines->vals[i], MAX_LINE - 1);
        lines->len++;
    }
    if (state == 1 && lines->vals[lines->len - 1][0] == '\0') { // Empty last line
        free(lines->vals[lines->len - 1]);
        lines->len--;
    }
    if (state == 2) {
        ERR("Line #%d too long", i);
        return 2;
    }
    return 0;
}

int writeLines(FILE *ofile, lines_t *lines) {
    for (int i = 0; i < lines->len; ++i) {
        if (fputs((const char *)lines->vals[i], ofile) == EOF) {
            ERR("Can\'t write line #%d", i);
            return 1;
        }
        fputc('\n', ofile);
    }
    return 0;
}

int initLines(lines_t *lines) {
    lines->vals = (letter **) calloc(MAX_LINES, sizeof(letter *));
    if (lines->vals == NULL) {
        ERR("Can't allocate space for lines");
        return 1;
    }
    lines->len = 0;
    return 0;
}

void sortLines(lines_t *lines, int (*cmp)(const void *, const void *)) {
    qsort(lines->vals, lines->len, sizeof(lines->vals[0]), cmp);
}

bool isLetter(letter c) {
    return isalpha(c) || ((letter)'а' <= c && c <= (letter)'я') || ((letter)'А' <= c && c <= (letter)'Я');
}

int cmpLines(const void *a, const void *b) {
    const letter * a_str = *(const letter **)a;
    const letter * b_str = *(const letter **)b;

    int i = 0, j = 0;
    while (i < MAX_LINE && j < MAX_LINE && a_str[i] && b_str[j]) {
        if (isLetter(a_str[i]) && isLetter(b_str[j])) {
            int res = (int)a_str[i] - (int)b_str[j];
            if (res) return res;
            i++; j++;
            continue;
        }
        if (!isLetter(a_str[i])) i++;
        if (!isLetter(b_str[j])) j++;
    }
    return 0;
}

void freeLines(lines_t *lines) {
    for (int i = 0; i < lines->len; ++i) {
        free(lines->vals[i]);
    }
    free(lines->vals);
    lines->len = 0;
}

//================================================================================

#ifdef TEST
void test_cmpLines(lines_t * lines) {
    const int cnt = 5;
    const letter values[cnt][20] = {"Привет", "При, вот", "!При, ват", "Bonjour", "[123]"};
    const letter result[cnt][20] = {"[123]", "Bonjour", "!При, ват", "Привет", "При, вот"};

    for (int i = 0; i < cnt; ++i) {
        lines->vals[i] = (letter *)calloc(20, sizeof(letter));
        TEST_ASSERT_M(lines->vals[i] != NULL, "Not enough RAM for the test");
        strcpy((char *)lines->vals[lines->len++], (const char *)values[i]);
    }

    sortLines(lines, cmpLines);

    TEST_ASSERT(lines->len == cnt);
    for (int i = 0; i < cnt; ++i) {
        TEST_ASSERT_M(strcmp((const char *)lines->vals[i], (const char *)result[i]) == 0, "Expected %s on %ith place, got %s instead", result[i], i, lines->vals[i]);
    }
}
#endif // TEST
