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
} line_t;

/**
 * An array of lines of a poem
 */
typedef struct lines {
    int len;  /**< The number of lines */
    line_t *vals;  /**< The actual lines */
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
 * @param [in]  a, b  The first lines to be compared. (Actual type: `line *`)
 *
 * @return The rough equivalent of `a` - `b`, as specified in `sortLines`
 */
int cmpLines(const void *a, const void *b);  // Attention: compares two strings, not lines_t!

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
    setlocale(LC_ALL, "Russian");

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

    FILE * ifile = fopen(argv[1], "r");
    if (ifile == NULL) {
        ERR("Couldn't open \'%s\' to read", argv[1]);
        return EXIT_FAILURE;
    }
    lines_t lines;
    //initLines(&lines);
    int result = readLines(ifile, &lines);
    fclose(ifile);
    if (result) {
        ERR("Error while reading lines");
        freeLines(&lines);
        return EXIT_FAILURE;
    }

    printf("Read %d lines, sorting...\n", lines.len);
    sortLines(&lines, cmpLines);

    char newName[32] = "sorted_";
    if (strlen(newName) + strlen(argv[1]) > sizeof(newName)) {
        ERR("Binary\'s name is too long");
        freeLines(&lines);
        return EXIT_FAILURE;
    }
    strcat(newName, argv[1]);

    printf("Done sorting, writing to %s\n", newName);
    FILE * ofile = fopen(newName, "w");
    if (ofile == NULL) {
        ERR("Couldn't open \'%s\' to write", newName);
        freeLines(&lines);
        return EXIT_FAILURE;
    }

    result = writeLines(ofile, &lines);
    fclose(ofile);
    if (result) {
        ERR("Error while writing lines");
        freeLines(&lines);
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

    initLines(lines, lineCnt, textLen);
    if (lines->text == NULL) {
        ERR("Can\'t allocate space for the lines");
        return 1;
    }

    if (fread(lines->text, sizeof(letter), textLen - 1, ifile) != textLen - 1) {
        ERR("Insufficient data read from file. Race condition, huh?");
        return 2;
    }
    lines->text[textLen - 1] = '\0';
    if (lines->text[textLen - 2] == '\n') {
        lines->text[textLen - 2] = '\0';
        lineCnt--;
    }
    //printf("%d\n%s", textLen,  text);
    //printf("%02x%02x%02x%02x", text[textLen - 4], text[textLen - 3], text[textLen - 2], text[textLen - 1]);

    lines->vals[0].val = lines->text;
    lines->len++;
    size_t pos = 0;
    while (pos < textLen) {
        if (lines->text[pos] == '\n') {
            lines->text[pos] = '\0';
            lines->vals[lines->len].val = lines->text + pos + 1;
            lines->len++;
        }
        pos++;
    }

    if (lines->len != lineCnt) {
        ERR("Wrong number of lines. Why would one even perform a race condition here?");
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
        fputc('\n', ofile);
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
    return 0;
}

void sortLines(lines_t *lines, int (*cmp)(const void *, const void *)) {
    assert(lines != NULL);
    assert(cmp != NULL);
    qsort(lines->vals, lines->len, sizeof(lines->vals[0]), cmp);
}

bool isLetter(letter c) {
    return isalpha(c) || ((letter)'а' <= c && c <= (letter)'я') || ((letter)'А' <= c && c <= (letter)'Я');
}

int cmpLines(const void *a, const void *b) {
    assert(a != NULL);
    assert(b != NULL);
    const letter * a_str = (*(const line *)a).val;
    const letter * b_str = (*(const line *)b).val;

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
    if (lines == NULL) return;
    if (lines->vals != NULL) free(lines->vals);
    if (lines->text != NULL) free(lines->text);
    lines->vals = NULL;
    lines->text = NULL;
    lines->len = 0;
}

void analyzeFile(FILE * ifile, int *lineCnt, size_t *length) {
    assert(ifile != NULL);
    fseek(ifile, 0L, SEEK_SET);
    (*lineCnt) = 0;
    (*length) = 0;
    int cur = 0;
    do {
        cur = fgetc(ifile);
        (*length)++;
        if (cur == '\n' || cur == EOF) (*lineCnt)++; // We'll assume there's another line at the end terminated by EOF
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
